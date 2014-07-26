# Vault-Tec Multiplayer Mod

*Vault-Tec Multiplayer Mod* ("VaultMP") is a multiplayer mod for the PC version of Bethesda's award-winning role-playing game [Fallout 3](http://en.wikipedia.org/wiki/Fallout_3 "Fallout 3"). It's currently in development, but is going to support the following:

* Dedicated server, MasterServer and client including a server browser
* Syncing of movement, actions / animations and AI
* In-game GUI overlay for chatting
* Supporting DLCs as well as Fallout user mods
* Server-side scripting to define gameplay rules

The main goal is to enable players to roam the open world of the wasteland and experience all the exciting features of the game online.

# The application

![Application structure](http://www.brickster.net/files/vaultmp/structure.png "Application structure")

# Compilation

All sources (except source/vaultgui) can be compiled with >=GCC 4.9 (std::thread is required). On Windows operating systems, you should use a MinGW setup. The following makefiles are available:

* source/makefile (main application, Windows only)
* source/vaultserver/makefile.windows (dedicated server, Windows)
* source/vaultserver/makefile.unix (dedicated server, Linux)
* source/vaultmaster/makefile.windows (master server, Windows)
* source/vaultmaster/makefile.unix (master server, Linux)
* source/vaultscript/makefile (default C++ script, Linux)

Code::Blocks project files are available for the parts for which there are no makefiles yet:

* source/vaultmpdll/vaultmpDLL.cbp
* source/vaultscript/vaultscript.cbp

# Scripting

An essential part of vaultmp is the server-side scripting interface component. Documentation and examples on how use it are available on vaultmp.com. See:

* http://www.vaultmp.com/showwiki.php?title=Category:Scripting
* http://www.vaultmp.com/showwiki.php?title=Category:VaultMP+interface

# Help

vaultmp is far from complete. Any help (mostly coding, documentation) is very appreciated.
