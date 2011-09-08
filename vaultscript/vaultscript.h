/*
 *  vaultscript.h
 *  Don't change anything here
 */

#include <string>

#ifndef __WIN32__
  #define VAULTSCRIPT __attribute__ ((__visibility__("default")))
#else
    #define VAULTSCRIPT __declspec(dllexport)
#endif

namespace vaultmp {

typedef unsigned char Index; // 1 byte
typedef unsigned char Reason; // 1 byte
typedef bool State; // 1 byte
typedef unsigned int Cell; // 4 byte
typedef unsigned int Base; // 4 byte
typedef unsigned int Interval; // 4 byte
typedef double ID; // 8 byte
typedef double Value; // 8 byte
typedef double Timer; // 8 byte

typedef void (*TimerFunc)();

static const Index FALLOUT3             =   0x01;
static const Index NEWVEGAS             =   FALLOUT3 << 1;
static const Index OBLIVION             =   NEWVEGAS << 1;
static const Index FALLOUT_GAMES        =   FALLOUT3 | NEWVEGAS;
static const Index ALL_GAMES            =   FALLOUT_GAMES | OBLIVION;

};

extern "C" {
    VAULTSCRIPT void exec();
    VAULTSCRIPT bool OnClientAuthenticate(std::string, std::string);
    VAULTSCRIPT void OnPlayerDisconnect(vaultmp::ID, vaultmp::Reason);
    VAULTSCRIPT vaultmp::Base OnPlayerRequestGame(vaultmp::ID);
    VAULTSCRIPT void OnSpawn(vaultmp::ID);
    VAULTSCRIPT void OnCellChange(vaultmp::ID, vaultmp::Cell);
    VAULTSCRIPT void OnActorDeath(vaultmp::ID);
    VAULTSCRIPT void OnActorValueChange(vaultmp::ID, vaultmp::Index, vaultmp::Value);
    VAULTSCRIPT void OnActorBaseValueChange(vaultmp::ID, vaultmp::Index, vaultmp::Value);
    VAULTSCRIPT void OnActorStateChange(vaultmp::ID, vaultmp::Index, vaultmp::State);

    VAULTSCRIPT void (*timestamp)();
    VAULTSCRIPT vaultmp::Timer (*CreateTimer)(vaultmp::TimerFunc, vaultmp::Interval);
    VAULTSCRIPT void (*KillTimer)(vaultmp::Timer);

    VAULTSCRIPT void (*SetServerName)(std::string);
    VAULTSCRIPT void (*SetServerMap)(std::string);
    VAULTSCRIPT void (*SetServerRule)(std::string, std::string);
    VAULTSCRIPT vaultmp::Index (*GetGameCode)();

    VAULTSCRIPT std::string (*ValueToString)(vaultmp::Index);
    VAULTSCRIPT std::string (*AxisToString)(vaultmp::Index);
    VAULTSCRIPT std::string (*AnimToString)(vaultmp::Index);

    VAULTSCRIPT void (*GetPos)(vaultmp::ID, vaultmp::Value&, vaultmp::Value&, vaultmp::Value&);
    VAULTSCRIPT void (*GetAngle)(vaultmp::ID, vaultmp::Value&, vaultmp::Value&, vaultmp::Value&);
    VAULTSCRIPT vaultmp::Cell (*GetCell)(vaultmp::ID);
    VAULTSCRIPT vaultmp::Value (*GetActorValue)(vaultmp::ID, vaultmp::Index);
    VAULTSCRIPT vaultmp::Value (*GetActorBaseValue)(vaultmp::ID, vaultmp::Index);
}
