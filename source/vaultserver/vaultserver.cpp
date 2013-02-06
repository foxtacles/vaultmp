#ifdef __WIN32__
#include <winsock2.h>
#endif
#include <cstdio>
#include <thread>

#include "vaultserver.h"
#include "Dedicated.h"
#include "Script.h"
#include "../Utils.h"
#include "../iniparser/src/dictionary.h"
#include "../iniparser/src/iniparser.h"

using namespace std;
using namespace RakNet;

void InputThread()
{
	char input[256];
	string cmd;

	do
	{
		fgets(input, sizeof(input), stdin);

		if (*input && input[strlen(input) - 1] == '\n')
			input[strlen(input) - 1] = '\0';

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
					auto player = GameFactory::GetObject<Player>(client->GetPlayer());

					if (player)
						printf("client ID: %d, player name: %s\n", id, player->GetName().c_str());
				}
			}
		}
		else if (!strcmp(cmd.c_str(), "uimsg"))
		{
			char* _id = strtok(nullptr, " ");

			if (_id)
			{
				unsigned int id = atoi(_id);
				Client* client = Client::GetClientFromID(id);

				if (client)
				{
					char* msg = _id + strlen(_id) + 1;

					if (*msg)
						Script::UIMessage(client->GetPlayer(), msg, 0);
				}
			}
		}

	}
	while (strcmp(cmd.c_str(), "exit") != 0);

	Dedicated::TerminateThread();
}

int main(int argc, char* argv[])
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
	printf("Vault-Tec dedicated server %s (Unix)\n----------------------------------------------------------\n", DEDICATED_VERSION);
#endif

	unsigned char game;
	unsigned int port;
	unsigned int players;
	unsigned int fileslots;
	bool query;
	bool files;
	const char* announce;
	const char* scripts;
	const char* mods;
	unsigned int cell;

	dictionary* config = iniparser_load(argc > 1 ? argv[1] : "vaultserver.ini");

	const char* game_str = iniparser_getstring(config, "general:game", "fallout3");

	if (stricmp(game_str, "newvegas") == 0)
		game = NEWVEGAS;
	else
		game = FALLOUT3;

	port = iniparser_getint(config, "general:port", RAKNET_STANDARD_PORT);
	players = iniparser_getint(config, "general:players", RAKNET_STANDARD_CONNECTIONS);
	query = static_cast<bool>(iniparser_getboolean(config, "general:query", 1));
	files = static_cast<bool>(iniparser_getboolean(config, "general:fileserve", 0));
	fileslots = iniparser_getint(config, "general:fileslots", 8);
	announce = iniparser_getstring(config, "general:master", "vaultmp.com");
	cell = iniparser_getint(config, "general:spawn", game == FALLOUT3 ? 0x000010C1 : 0x000DAEBB); // Vault101Exterior and Goodsprings
	scripts = iniparser_getstring(config, "scripts:scripts", "");
	mods = iniparser_getstring(config, "mods:mods", "");

	ServerEntry self(game);
	self.SetServerRule("version", DEDICATED_VERSION);
	Dedicated::SetServerEntry(&self);

	char base[MAX_PATH];
	_getcwd(base, sizeof(base));

	try
	{
		putenv(PWNFILES_PATH);
		vector<char> _scripts(scripts, scripts + strlen(scripts) + 1);
		Script::LoadScripts(&_scripts[0], base);
	}
	catch (std::exception& e)
	{
		try
		{
			VaultException& vaulterror = dynamic_cast<VaultException&>(e);
			vaulterror.Console();
		}

		catch (std::bad_cast& no_vaulterror)
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

		thread hDedicatedThread = Dedicated::InitializeServer(port, players, announce, query, files, fileslots);
		thread hInputThread = thread(InputThread);

		hDedicatedThread.join();

		if (hInputThread.joinable())
			hInputThread.join();
	}
	catch (std::exception& e)
	{
		try
		{
			VaultException& vaulterror = dynamic_cast<VaultException&>(e);
			vaulterror.Console();
		}
		catch (std::bad_cast& no_vaulterror)
		{
			VaultException vaulterror(e.what());
			vaulterror.Console();
		}
	}

	iniparser_freedict(config);

#ifdef __WIN32__
	system("PAUSE");
#endif

	return 0;
}
