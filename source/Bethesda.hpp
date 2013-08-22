#ifndef BETHESDA_H
#define BETHESDA_H

#include "vaultmp.hpp"
#include "RakNet.hpp"

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

#include <string>
#include <vector>

/**
 * \brief Starting point to run a vaultmp game
 */

class Bethesda
{
		friend class NetworkClient;
		friend class Game;

	private:
		typedef HINSTANCE(__stdcall* fLoadLibrary)(char*);
		typedef std::vector<std::pair<std::string, unsigned int>> ModList;

		static std::string password;
		static unsigned int inittime;
		static bool multiinst;
		static ModList modfiles;
		static DWORD process;
		static bool initialized;

		/**
		 * \brief Starts up the game
		 */
		static void Initialize();

		/**
		 * \brief Ends this session, cleanup
		 */
		static void Terminate(RakNet::RakPeerInterface* peer);

		/**
		 * \brief Lookup the ID of a given process name
		 */
		static DWORD lookupProgramID(const char process[]);

#ifdef VAULTMP_DEBUG
		static DebugInput<Bethesda> debug;
#endif

		Bethesda() = delete;

	public:
		/**
		 * \brief Initializes Vault-Tec Multiplayer Mod
		 */
		static void InitializeVaultMP(RakNet::RakPeerInterface* peer, RakNet::SystemAddress server, const std::string& name, const std::string& pwd, bool multiinst, unsigned int inittime);
};

#endif
