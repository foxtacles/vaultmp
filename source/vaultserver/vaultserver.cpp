#include "vaultserver.hpp"
#include "Dedicated.hpp"
#include "Script.hpp"
#include "Utils.hpp"
#include "Client.hpp"
#include "ServerEntry.hpp"
#include "iniparser/src/dictionary.h"
#include "iniparser/src/iniparser.h"

#ifdef __WIN32__
#include <winsock2.h>
#endif
#include <cstdio>
#include <thread>
#include <unordered_map>
#include <csignal>

using namespace std;
using namespace RakNet;

bool cmd_exit = false;

void InputThread()
{
	cmd_exit = false;
	char input[256];
	string cmd;

	do
	{
		fgets(input, sizeof(input), stdin);

		unsigned int len;
		if (*input && input[(len = strlen(input)) - 1] == '\n')
			input[len - 1] = '\0';

		char* tok = strtok(input, " ");

		if (!tok)
			continue;

		cmd = string(tok);

		if (!strcmp(cmd.c_str(), "ls"))
		{
			vector<RakNetGUID> clients = Client::GetNetworkList(nullptr);

			for (RakNetGUID& guid : clients)
			{
				Client* client = Client::GetClientFromGUID(guid);

				if (client)
				{
					unsigned int id = client->GetID();
					auto player = GameFactory::Get<Player>(client->GetPlayer());

					if (player)
						printf("client ID: %d, player name: %s\n", id, player->GetName().c_str());
				}
			}
		}
		else if (!strcmp(cmd.c_str(), "uimsg"))
		{
			const char* _id = strtok(nullptr, " ");

			if (_id)
			{
				unsigned int id = atoi(_id);
				Client* client = Client::GetClientFromID(id);

				if (client)
				{
					const char* msg = _id + strlen(_id) + 1;

					if (*msg)
						Script::UIMessage(client->GetPlayer(), msg, 0);
				}
			}
		}
	}
	while (!cmd_exit && !(cmd_exit = !strcmp(cmd.c_str(), "exit")));

	Dedicated::TerminateThread();
}

void SignalHandler(int)
{
	printf("SIGTERM/SIGINT received. Shutting down...press enter to continue\n");
	cmd_exit = true;
	Dedicated::TerminateThread();
}

int main(int, char* argv[])
{
#ifdef VAULTMP_DEBUG
#ifdef __WIN32__

	if (LoadLibrary("exchndl.dll") == nullptr)
		return 0;

#else
	system("ulimit -c unlimited");
#endif
#endif

#ifdef __WIN32__
	printf("Vault-Tec dedicated server %s (Windows)\n----------------------------------------------------------\n", DEDICATED_VERSION);
#else
	printf("Vault-Tec dedicated server %s (Linux)\n----------------------------------------------------------\n", DEDICATED_VERSION);
#endif

	signal(SIGINT, SignalHandler);
	signal(SIGTERM, SignalHandler);

	unordered_map<string, const char*> args;
	unsigned int arg_ = 1;

	while (argv[arg_])
	{
		if (*argv[arg_] == '-' && *(argv[arg_] + 1) && argv[arg_ + 1])
		{
			args.emplace(argv[arg_] + 1, argv[arg_ + 1]);
			++arg_;
		}

		++arg_;
	}

	unsigned int port;
	const char* host;
	unsigned int players;
	unsigned int fileslots;
	bool query;
	bool files;
	const char* announce;
	const char* scripts;
	const char* mods;
	unsigned int cell;
	bool keep;

	dictionary* config = iniparser_load(args.count("ini") ? args["ini"] : "vaultserver.ini");

	auto iniparser_getstring_ex = [&args, config](const char* param, const char* alt) -> const char* { return args.count(param) ? args[param] : iniparser_getstring(config, param, alt); };
	auto iniparser_getint_ex = [&args, config](const char* param, int alt) -> int { return args.count(param) ? strtol(args[param], nullptr, 0) : iniparser_getint(config, param, alt); };
	auto iniparser_getboolean_ex = [&args, config](const char* param, bool alt) -> bool { return args.count(param) ? strtol(args[param], nullptr, 0) : iniparser_getboolean(config, param, alt); };

	port = iniparser_getint_ex("general:port", RAKNET_STANDARD_PORT);
	host = iniparser_getstring_ex("general:host", nullptr);
	players = iniparser_getint_ex("general:players", RAKNET_STANDARD_CONNECTIONS);
	query = iniparser_getboolean_ex("general:query", true);
	files = iniparser_getboolean_ex("general:fileserve", false);
	fileslots = iniparser_getint_ex("general:fileslots", 8);
	announce = iniparser_getstring_ex("general:master", "vaultmp.com");
	cell = iniparser_getint_ex("general:spawn", 0x000010C1); // Vault101Exterior
	keep = iniparser_getboolean_ex("general:keepalive", false);
	scripts = iniparser_getstring_ex("scripts:scripts", "");
	mods = iniparser_getstring_ex("mods:mods", "");

	thread hInputThread = thread(InputThread);

	do
	{
		ServerEntry self;
		self.SetServerRule("version", DEDICATED_VERSION);
		Dedicated::SetServerEntry(&self);

		char base[MAX_PATH];
		_getcwd(base, sizeof(base));

		try
		{
#ifdef __WIN32__
			putenv(PWNFILES_PATH);
#else
			setenv(PWNFILES_KEY, PWNFILES_VAL, 1);
#endif
			vector<char> _scripts(scripts, scripts + strlen(scripts) + 1);
			Script::LoadScripts(&_scripts[0], base);
		}
		catch (exception& e)
		{
			try
			{
				VaultException& vaulterror = dynamic_cast<VaultException&>(e);
				vaulterror.Console();
			}
			catch (bad_cast& no_vaulterror)
			{
				VaultException vaulterror(e.what());
				vaulterror.Console();
			}
		}

		try
		{
			Dedicated::SetSpawnCell(cell);

			vector<char> buf(mods, mods + strlen(mods) + 1);
			char* token = strtok(&buf[0], ",");
			ModList modfiles;
			char file[MAX_PATH];
			unsigned int crc;

			while (token != nullptr)
			{
				snprintf(file, sizeof(file), "%s/%s/%s", base, MODFILES_PATH, token);

				if (!Utils::crc32file(file, &crc))
					throw VaultException("Could not find modfile %s in folder %s", token, MODFILES_PATH).stacktrace();

				modfiles.emplace_back(token, crc);

				token = strtok(nullptr, ",");
			}

			Dedicated::SetModfiles(modfiles);

			thread hDedicatedThread = Dedicated::InitializeServer(port, host, players, announce, query, files, fileslots);
			hDedicatedThread.join();
		}
		catch (exception& e)
		{
			try
			{
				VaultException& vaulterror = dynamic_cast<VaultException&>(e);
				vaulterror.Console();
			}
			catch (bad_cast& no_vaulterror)
			{
				VaultException vaulterror(e.what());
				vaulterror.Console();
			}

			keep = false;
		}
	} while (keep && !cmd_exit);

	if (hInputThread.joinable())
		hInputThread.join();

	iniparser_freedict(config);

#ifndef __WIN32__
	unsetenv(PWNFILES_KEY);
#endif

	return 0;
}
