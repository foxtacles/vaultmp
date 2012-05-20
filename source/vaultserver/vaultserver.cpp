#ifdef __WIN32__
#include <winsock2.h>
#endif
#include <cstdio>
#include <thread>

#include "vaultserver.h"
#include "Dedicated.h"
#include "Script.h"
#include "../Utils.h"
#include "../iniparser/dictionary.c"
#include "../iniparser/iniparser.c"

void InputThread()
{
	char input[256];
	string cmd;

	do
	{
		fgets(input, sizeof(input), stdin);

		if (strlen(input) > 0 && input[strlen(input) - 1] == '\n')
			input[strlen(input) - 1] = '\0';

		char* tok = strtok(input, " ");

		if (!tok)
			continue;

		cmd = string(tok);

		if (!strcmp(cmd.c_str(), "ls"))
		{
			vector<RakNetGUID> clients = Client::GetNetworkList(NULL);

			for (RakNetGUID & guid : clients)
			{
				Client* client = Client::GetClientFromGUID(guid);

				if (client)
				{
					try
					{
						unsigned int id = client->GetID();
						printf("client ID: %d, player name: %s\n", id, vaultcast<Player>(*GameFactory::GetObject(client->GetPlayer()))->GetName().c_str());
					}
					catch (...) {}
				}
			}
		}
		else if (!strcmp(cmd.c_str(), "uimsg"))
		{
			char* _id = strtok(NULL, " ");

			if (_id)
			{
				unsigned int id = atoi(_id);
				Client* client = Client::GetClientFromID(id);

				if (client)
				{
					char* msg = _id + strlen(_id) + 1;

					if (*msg)
						Script::UIMessage(client->GetPlayer(), msg);
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

	if (LoadLibrary("exchndl.dll") == NULL)
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
	int port;
	int players;
	int fileslots;
	bool query;
	bool files;
	char* announce;
	char* scripts;
	char* mods;
	char* savegame;

	dictionary* config = iniparser_load(argc > 1 ? argv[1] : (char*) "vaultserver.ini");

	char* game_str = iniparser_getstring(config, (char*) "general:game", (char*) "fallout3");

	if (stricmp(game_str, "newvegas") == 0)
		game = NEWVEGAS;
	else
		game = FALLOUT3;

	port = iniparser_getint(config, (char*) "general:port", RAKNET_STANDARD_PORT);
	players = iniparser_getint(config, (char*) "general:players", RAKNET_STANDARD_CONNECTIONS);
	query = (bool) iniparser_getboolean(config, (char*) "general:query", 1);
	files = (bool) iniparser_getboolean(config, (char*) "general:fileserve", 0);
	fileslots = iniparser_getint(config, (char*) "general:fileslots", 8);
	announce = iniparser_getstring(config, (char*) "general:master", (char*) "vaultmp.com");
	savegame = iniparser_getstring(config, (char*) "general:save", (char*) "default.fos");
	scripts = iniparser_getstring(config, (char*) "scripts:scripts", (char*) "standard.amx");
	mods = iniparser_getstring(config, (char*) "mods:mods", (char*) "");

	ServerEntry* self = new ServerEntry(game);
	self->SetServerRule("version", DEDICATED_VERSION);
	Dedicated::SetServerEntry(self);

	char base[MAX_PATH];
	getcwd(base, sizeof(base));

	try
	{
		putenv(PWNFILES_PATH);
		char _scripts[strlen(scripts) + 1];
		snprintf(_scripts, sizeof(_scripts), "%s", scripts);
		Script::LoadScripts(_scripts, base);
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
		char file[MAX_PATH];
		snprintf(file, sizeof(file), "%s/%s/%s", base, SAVEGAME_PATH, savegame);

		unsigned int crc;

		if (!Utils::crc32file(file, &crc))
			throw VaultException("Could not find savegame %s in folder %s", savegame, SAVEGAME_PATH);

		Dedicated::SetSavegame(Savegame(string(savegame), crc));

		char* token = strtok(mods, ",");
		ModList modfiles;

		while (token != NULL)
		{
			snprintf(file, sizeof(file), "%s/%s/%s", base, MODFILES_PATH, token);

			if (!Utils::crc32file(file, &crc))
				throw VaultException("Could not find modfile %s in folder %s", token, MODFILES_PATH);

			modfiles.push_back(pair<string, unsigned int>(string(token), crc));

			token = strtok(NULL, ",");
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

	Script::UnloadScripts();
	iniparser_freedict(config);
	delete self;

#ifdef __WIN32__
	system("PAUSE");
#endif

	return 0;
}
