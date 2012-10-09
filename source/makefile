WORKDIR = %cd%

CC = mingw32-gcc.exe
CXX = mingw32-g++.exe
AR = ar.exe
LD = mingw32-g++.exe
WINDRES = windres.exe

INC =  -I. -Istack_trace\\include
CFLAGS =  -pedantic-errors -pedantic -Wfatal-errors -Wextra -Wall -std=gnu++0x
CLFAGSEXT =
RESINC =
LIBDIR =
LIB =
LDFLAGS = ufmod.obj -lshlwapi -lcomctl32 -lwinmm -lws2_32 -limagehlp -static -mwindows

INC_DEBUG =  $(INC)
CFLAGS_DEBUG =  $(CFLAGS) -gstabs -DVAULTMP_DEBUG
CLFAGSEXT_DEBUG =  $(CFLAGSEXT) -gstabs
RESINC_DEBUG =  $(RESINC)
RCFLAGS_DEBUG =  $(RCFLAGS)
LIBDIR_DEBUG =  $(LIBDIR)
LIB_DEBUG = $(LIB)
LDFLAGS_DEBUG =  -lbfd -liberty $(LDFLAGS)
OBJDIR_DEBUG = .objs\\Debug
DEP_DEBUG =
OUT_DEBUG = vaultmpd.exe

INC_RELEASE =  $(INC)
CFLAGS_RELEASE =  $(CFLAGS) -O2 -DNDEBUG
CLFAGSEXT_RELEASE =  $(CFLAGSEXT) -O2 -DNDEBUG
RESINC_RELEASE =  $(RESINC)
RCFLAGS_RELEASE =  $(RCFLAGS)
LIBDIR_RELEASE =  $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE =  -s $(LDFLAGS)
OBJDIR_RELEASE = .objs\\Release
DEP_RELEASE =
OUT_RELEASE = vaultmp.exe

OBJ_DEBUG = $(OBJDIR_DEBUG)\\RakNet\\ReadyEvent.o $(OBJDIR_DEBUG)\\RakNet\\Rand.o $(OBJDIR_DEBUG)\\RakNet\\RakWString.o $(OBJDIR_DEBUG)\\RakNet\\RakThread.o $(OBJDIR_DEBUG)\\RakNet\\RakString.o $(OBJDIR_DEBUG)\\RakNet\\RakSleep.o $(OBJDIR_DEBUG)\\RakNet\\RakPeer.o $(OBJDIR_DEBUG)\\RakNet\\RakNetTypes.o $(OBJDIR_DEBUG)\\RakNet\\SimpleMutex.o $(OBJDIR_DEBUG)\\RakNet\\SignaledEvent.o $(OBJDIR_DEBUG)\\RakNet\\SendToThread.o $(OBJDIR_DEBUG)\\RakNet\\SecureHandshake.o $(OBJDIR_DEBUG)\\RakNet\\SHA1.o $(OBJDIR_DEBUG)\\RakNet\\Router2.o $(OBJDIR_DEBUG)\\RakNet\\ReplicaManager3.o $(OBJDIR_DEBUG)\\RakNet\\ReliabilityLayer.o $(OBJDIR_DEBUG)\\RakNet\\RPC4Plugin.o $(OBJDIR_DEBUG)\\RakNet\\PluginInterface2.o $(OBJDIR_DEBUG)\\RakNet\\PacketizedTCP.o $(OBJDIR_DEBUG)\\RakNet\\PacketOutputWindowLogger.o $(OBJDIR_DEBUG)\\RakNet\\PacketLogger.o $(OBJDIR_DEBUG)\\RakNet\\PacketFileLogger.o $(OBJDIR_DEBUG)\\RakNet\\PacketConsoleLogger.o $(OBJDIR_DEBUG)\\RakNet\\NetworkIDObject.o $(OBJDIR_DEBUG)\\RakNet\\RakNetTransport2.o $(OBJDIR_DEBUG)\\RakNet\\RakNetStatistics.o $(OBJDIR_DEBUG)\\RakNet\\RakNetSocket.o $(OBJDIR_DEBUG)\\RakNet\\RakNetCommandParser.o $(OBJDIR_DEBUG)\\RakNet\\RakMemoryOverride.o $(OBJDIR_DEBUG)\\RakNet\\Rackspace.o $(OBJDIR_DEBUG)\\Utils.o $(OBJDIR_DEBUG)\\Shared.o $(OBJDIR_DEBUG)\\ServerEntry.o $(OBJDIR_DEBUG)\\Reference.o $(OBJDIR_DEBUG)\\RakNet\\rdlmalloc.o $(OBJDIR_DEBUG)\\RakNet\\gettimeofday.o $(OBJDIR_DEBUG)\\RakNet\\_FindFirst.o $(OBJDIR_DEBUG)\\RakNet\\WSAStartupSingleton.o $(OBJDIR_DEBUG)\\vaultmp.o $(OBJDIR_DEBUG)\\vaultmprc.o $(OBJDIR_DEBUG)\\stack_trace\\src\\stack.o $(OBJDIR_DEBUG)\\iniparser\\src\\iniparser.o $(OBJDIR_DEBUG)\\iniparser\\src\\dictionary.o $(OBJDIR_DEBUG)\\VaultVector.o $(OBJDIR_DEBUG)\\VaultFunctor.o $(OBJDIR_DEBUG)\\VaultException.o $(OBJDIR_DEBUG)\\Value.o $(OBJDIR_DEBUG)\\RakNet\\TelnetTransport.o $(OBJDIR_DEBUG)\\RakNet\\TeamBalancer.o $(OBJDIR_DEBUG)\\RakNet\\TableSerializer.o $(OBJDIR_DEBUG)\\RakNet\\TCPInterface.o $(OBJDIR_DEBUG)\\RakNet\\SuperFastHash.o $(OBJDIR_DEBUG)\\RakNet\\StringTable.o $(OBJDIR_DEBUG)\\RakNet\\StringCompressor.o $(OBJDIR_DEBUG)\\RakNet\\SocketLayer.o $(OBJDIR_DEBUG)\\RakNet\\VariadicSQLParser.o $(OBJDIR_DEBUG)\\RakNet\\VariableListDeltaTracker.o $(OBJDIR_DEBUG)\\RakNet\\VariableDeltaSerializer.o $(OBJDIR_DEBUG)\\RakNet\\UDPProxyServer.o $(OBJDIR_DEBUG)\\RakNet\\UDPProxyCoordinator.o $(OBJDIR_DEBUG)\\RakNet\\UDPProxyClient.o $(OBJDIR_DEBUG)\\RakNet\\UDPForwarder.o $(OBJDIR_DEBUG)\\RakNet\\TwoWayAuthentication.o $(OBJDIR_DEBUG)\\RakNet\\ThreadsafePacketLogger.o $(OBJDIR_DEBUG)\\RakNet\\ConnectionGraph2.o $(OBJDIR_DEBUG)\\RakNet\\CommandParserInterface.o $(OBJDIR_DEBUG)\\RakNet\\CloudServer.o $(OBJDIR_DEBUG)\\RakNet\\CloudCommon.o $(OBJDIR_DEBUG)\\RakNet\\CloudClient.o $(OBJDIR_DEBUG)\\RakNet\\CheckSum.o $(OBJDIR_DEBUG)\\RakNet\\CCRakNetUDT.o $(OBJDIR_DEBUG)\\RakNet\\CCRakNetSlidingWindow.o $(OBJDIR_DEBUG)\\RakNet\\BitStream.o $(OBJDIR_DEBUG)\\RakNet\\DS_HuffmanEncodingTree.o $(OBJDIR_DEBUG)\\RakNet\\DS_ByteQueue.o $(OBJDIR_DEBUG)\\RakNet\\DS_BytePool.o $(OBJDIR_DEBUG)\\RakNet\\ConsoleServer.o $(OBJDIR_DEBUG)\\Guarded.o $(OBJDIR_DEBUG)\\GameFactory.o $(OBJDIR_DEBUG)\\Game.o $(OBJDIR_DEBUG)\\Debug.o $(OBJDIR_DEBUG)\\CriticalSection.o $(OBJDIR_DEBUG)\\Container.o $(OBJDIR_DEBUG)\\Bethesda.o $(OBJDIR_DEBUG)\\Actor.o $(OBJDIR_DEBUG)\\API.o $(OBJDIR_DEBUG)\\Player.o $(OBJDIR_DEBUG)\\Pipe.o $(OBJDIR_DEBUG)\\PacketFactory.o $(OBJDIR_DEBUG)\\Object.o $(OBJDIR_DEBUG)\\NetworkClient.o $(OBJDIR_DEBUG)\\Network.o $(OBJDIR_DEBUG)\\Lockable.o $(OBJDIR_DEBUG)\\Item.o $(OBJDIR_DEBUG)\\Interface.o $(OBJDIR_DEBUG)\\RakNet\\LogCommandParser.o $(OBJDIR_DEBUG)\\RakNet\\LocklessTypes.o $(OBJDIR_DEBUG)\\RakNet\\LinuxStrings.o $(OBJDIR_DEBUG)\\RakNet\\Itoa.o $(OBJDIR_DEBUG)\\RakNet\\IncrementalReadInterface.o $(OBJDIR_DEBUG)\\RakNet\\HTTPConnection.o $(OBJDIR_DEBUG)\\RakNet\\GridSectorizer.o $(OBJDIR_DEBUG)\\RakNet\\Gets.o $(OBJDIR_DEBUG)\\RakNet\\Getche.o $(OBJDIR_DEBUG)\\RakNet\\NetworkIDManager.o $(OBJDIR_DEBUG)\\RakNet\\NatTypeDetectionServer.o $(OBJDIR_DEBUG)\\RakNet\\NatTypeDetectionCommon.o $(OBJDIR_DEBUG)\\RakNet\\NatTypeDetectionClient.o $(OBJDIR_DEBUG)\\RakNet\\NatPunchthroughServer.o $(OBJDIR_DEBUG)\\RakNet\\NatPunchthroughClient.o $(OBJDIR_DEBUG)\\RakNet\\MessageFilter.o $(OBJDIR_DEBUG)\\RakNet\\EmailSender.o $(OBJDIR_DEBUG)\\RakNet\\DynDNS.o $(OBJDIR_DEBUG)\\RakNet\\DirectoryDeltaTransfer.o $(OBJDIR_DEBUG)\\RakNet\\DataCompressor.o $(OBJDIR_DEBUG)\\RakNet\\DS_Table.o $(OBJDIR_DEBUG)\\RakNet\\GetTime.o $(OBJDIR_DEBUG)\\RakNet\\FullyConnectedMesh2.o $(OBJDIR_DEBUG)\\RakNet\\FormatString.o $(OBJDIR_DEBUG)\\RakNet\\FileOperations.o $(OBJDIR_DEBUG)\\RakNet\\FileListTransfer.o $(OBJDIR_DEBUG)\\RakNet\\FileList.o $(OBJDIR_DEBUG)\\RakNet\\EpochTimeToString.o $(OBJDIR_DEBUG)\\RakNet\\EncodeClassName.o

OBJ_RELEASE = $(OBJDIR_RELEASE)\\RakNet\\ReadyEvent.o $(OBJDIR_RELEASE)\\RakNet\\Rand.o $(OBJDIR_RELEASE)\\RakNet\\RakWString.o $(OBJDIR_RELEASE)\\RakNet\\RakThread.o $(OBJDIR_RELEASE)\\RakNet\\RakString.o $(OBJDIR_RELEASE)\\RakNet\\RakSleep.o $(OBJDIR_RELEASE)\\RakNet\\RakPeer.o $(OBJDIR_RELEASE)\\RakNet\\RakNetTypes.o $(OBJDIR_RELEASE)\\RakNet\\SimpleMutex.o $(OBJDIR_RELEASE)\\RakNet\\SignaledEvent.o $(OBJDIR_RELEASE)\\RakNet\\SendToThread.o $(OBJDIR_RELEASE)\\RakNet\\SecureHandshake.o $(OBJDIR_RELEASE)\\RakNet\\SHA1.o $(OBJDIR_RELEASE)\\RakNet\\Router2.o $(OBJDIR_RELEASE)\\RakNet\\ReplicaManager3.o $(OBJDIR_RELEASE)\\RakNet\\ReliabilityLayer.o $(OBJDIR_RELEASE)\\RakNet\\RPC4Plugin.o $(OBJDIR_RELEASE)\\RakNet\\PluginInterface2.o $(OBJDIR_RELEASE)\\RakNet\\PacketizedTCP.o $(OBJDIR_RELEASE)\\RakNet\\PacketOutputWindowLogger.o $(OBJDIR_RELEASE)\\RakNet\\PacketLogger.o $(OBJDIR_RELEASE)\\RakNet\\PacketFileLogger.o $(OBJDIR_RELEASE)\\RakNet\\PacketConsoleLogger.o $(OBJDIR_RELEASE)\\RakNet\\NetworkIDObject.o $(OBJDIR_RELEASE)\\RakNet\\RakNetTransport2.o $(OBJDIR_RELEASE)\\RakNet\\RakNetStatistics.o $(OBJDIR_RELEASE)\\RakNet\\RakNetSocket.o $(OBJDIR_RELEASE)\\RakNet\\RakNetCommandParser.o $(OBJDIR_RELEASE)\\RakNet\\RakMemoryOverride.o $(OBJDIR_RELEASE)\\RakNet\\Rackspace.o $(OBJDIR_RELEASE)\\Utils.o $(OBJDIR_RELEASE)\\Shared.o $(OBJDIR_RELEASE)\\ServerEntry.o $(OBJDIR_RELEASE)\\Reference.o $(OBJDIR_RELEASE)\\RakNet\\rdlmalloc.o $(OBJDIR_RELEASE)\\RakNet\\gettimeofday.o $(OBJDIR_RELEASE)\\RakNet\\_FindFirst.o $(OBJDIR_RELEASE)\\RakNet\\WSAStartupSingleton.o $(OBJDIR_RELEASE)\\vaultmp.o $(OBJDIR_RELEASE)\\vaultmprc.o $(OBJDIR_RELEASE)\\iniparser\\src\\iniparser.o $(OBJDIR_RELEASE)\\iniparser\\src\\dictionary.o $(OBJDIR_RELEASE)\\VaultVector.o $(OBJDIR_RELEASE)\\VaultFunctor.o $(OBJDIR_RELEASE)\\VaultException.o $(OBJDIR_RELEASE)\\Value.o $(OBJDIR_RELEASE)\\RakNet\\TelnetTransport.o $(OBJDIR_RELEASE)\\RakNet\\TeamBalancer.o $(OBJDIR_RELEASE)\\RakNet\\TableSerializer.o $(OBJDIR_RELEASE)\\RakNet\\TCPInterface.o $(OBJDIR_RELEASE)\\RakNet\\SuperFastHash.o $(OBJDIR_RELEASE)\\RakNet\\StringTable.o $(OBJDIR_RELEASE)\\RakNet\\StringCompressor.o $(OBJDIR_RELEASE)\\RakNet\\SocketLayer.o $(OBJDIR_RELEASE)\\RakNet\\VariadicSQLParser.o $(OBJDIR_RELEASE)\\RakNet\\VariableListDeltaTracker.o $(OBJDIR_RELEASE)\\RakNet\\VariableDeltaSerializer.o $(OBJDIR_RELEASE)\\RakNet\\UDPProxyServer.o $(OBJDIR_RELEASE)\\RakNet\\UDPProxyCoordinator.o $(OBJDIR_RELEASE)\\RakNet\\UDPProxyClient.o $(OBJDIR_RELEASE)\\RakNet\\UDPForwarder.o $(OBJDIR_RELEASE)\\RakNet\\TwoWayAuthentication.o $(OBJDIR_RELEASE)\\RakNet\\ThreadsafePacketLogger.o $(OBJDIR_RELEASE)\\RakNet\\ConnectionGraph2.o $(OBJDIR_RELEASE)\\RakNet\\CommandParserInterface.o $(OBJDIR_RELEASE)\\RakNet\\CloudServer.o $(OBJDIR_RELEASE)\\RakNet\\CloudCommon.o $(OBJDIR_RELEASE)\\RakNet\\CloudClient.o $(OBJDIR_RELEASE)\\RakNet\\CheckSum.o $(OBJDIR_RELEASE)\\RakNet\\CCRakNetUDT.o $(OBJDIR_RELEASE)\\RakNet\\CCRakNetSlidingWindow.o $(OBJDIR_RELEASE)\\RakNet\\BitStream.o $(OBJDIR_RELEASE)\\RakNet\\DS_HuffmanEncodingTree.o $(OBJDIR_RELEASE)\\RakNet\\DS_ByteQueue.o $(OBJDIR_RELEASE)\\RakNet\\DS_BytePool.o $(OBJDIR_RELEASE)\\RakNet\\ConsoleServer.o $(OBJDIR_RELEASE)\\Guarded.o $(OBJDIR_RELEASE)\\GameFactory.o $(OBJDIR_RELEASE)\\Game.o $(OBJDIR_RELEASE)\\Debug.o $(OBJDIR_RELEASE)\\CriticalSection.o $(OBJDIR_RELEASE)\\Container.o $(OBJDIR_RELEASE)\\Bethesda.o $(OBJDIR_RELEASE)\\Actor.o $(OBJDIR_RELEASE)\\API.o $(OBJDIR_RELEASE)\\Player.o $(OBJDIR_RELEASE)\\Pipe.o $(OBJDIR_RELEASE)\\PacketFactory.o $(OBJDIR_RELEASE)\\Object.o $(OBJDIR_RELEASE)\\NetworkClient.o $(OBJDIR_RELEASE)\\Network.o $(OBJDIR_RELEASE)\\Lockable.o $(OBJDIR_RELEASE)\\Item.o $(OBJDIR_RELEASE)\\Interface.o $(OBJDIR_RELEASE)\\RakNet\\LogCommandParser.o $(OBJDIR_RELEASE)\\RakNet\\LocklessTypes.o $(OBJDIR_RELEASE)\\RakNet\\LinuxStrings.o $(OBJDIR_RELEASE)\\RakNet\\Itoa.o $(OBJDIR_RELEASE)\\RakNet\\IncrementalReadInterface.o $(OBJDIR_RELEASE)\\RakNet\\HTTPConnection.o $(OBJDIR_RELEASE)\\RakNet\\GridSectorizer.o $(OBJDIR_RELEASE)\\RakNet\\Gets.o $(OBJDIR_RELEASE)\\RakNet\\Getche.o $(OBJDIR_RELEASE)\\RakNet\\NetworkIDManager.o $(OBJDIR_RELEASE)\\RakNet\\NatTypeDetectionServer.o $(OBJDIR_RELEASE)\\RakNet\\NatTypeDetectionCommon.o $(OBJDIR_RELEASE)\\RakNet\\NatTypeDetectionClient.o $(OBJDIR_RELEASE)\\RakNet\\NatPunchthroughServer.o $(OBJDIR_RELEASE)\\RakNet\\NatPunchthroughClient.o $(OBJDIR_RELEASE)\\RakNet\\MessageFilter.o $(OBJDIR_RELEASE)\\RakNet\\EmailSender.o $(OBJDIR_RELEASE)\\RakNet\\DynDNS.o $(OBJDIR_RELEASE)\\RakNet\\DirectoryDeltaTransfer.o $(OBJDIR_RELEASE)\\RakNet\\DataCompressor.o $(OBJDIR_RELEASE)\\RakNet\\DS_Table.o $(OBJDIR_RELEASE)\\RakNet\\GetTime.o $(OBJDIR_RELEASE)\\RakNet\\FullyConnectedMesh2.o $(OBJDIR_RELEASE)\\RakNet\\FormatString.o $(OBJDIR_RELEASE)\\RakNet\\FileOperations.o $(OBJDIR_RELEASE)\\RakNet\\FileListTransfer.o $(OBJDIR_RELEASE)\\RakNet\\FileList.o $(OBJDIR_RELEASE)\\RakNet\\EpochTimeToString.o $(OBJDIR_RELEASE)\\RakNet\\EncodeClassName.o

all: debug release

clean: clean_debug clean_release

before_debug:
	cmd /c if not exist $(OBJDIR_DEBUG)\\RakNet md $(OBJDIR_DEBUG)\\RakNet
	cmd /c if not exist $(OBJDIR_DEBUG) md $(OBJDIR_DEBUG)
	cmd /c if not exist $(OBJDIR_DEBUG)\\stack_trace\\src md $(OBJDIR_DEBUG)\\stack_trace\\src
	cmd /c if not exist $(OBJDIR_DEBUG)\\iniparser\\src md $(OBJDIR_DEBUG)\\iniparser\\src

after_debug:

debug: before_debug out_debug after_debug

out_debug: $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) $(LIBDIR_DEBUG) $(OBJ_DEBUG) $(LIB_DEBUG) -o $(OUT_DEBUG) $(LDFLAGS_DEBUG)

$(OBJDIR_DEBUG)\\RakNet\\ReadyEvent.o: RakNet\\ReadyEvent.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\ReadyEvent.cpp -o $(OBJDIR_DEBUG)\\RakNet\\ReadyEvent.o

$(OBJDIR_DEBUG)\\RakNet\\Rand.o: RakNet\\Rand.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\Rand.cpp -o $(OBJDIR_DEBUG)\\RakNet\\Rand.o

$(OBJDIR_DEBUG)\\RakNet\\RakWString.o: RakNet\\RakWString.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\RakWString.cpp -o $(OBJDIR_DEBUG)\\RakNet\\RakWString.o

$(OBJDIR_DEBUG)\\RakNet\\RakThread.o: RakNet\\RakThread.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\RakThread.cpp -o $(OBJDIR_DEBUG)\\RakNet\\RakThread.o

$(OBJDIR_DEBUG)\\RakNet\\RakString.o: RakNet\\RakString.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\RakString.cpp -o $(OBJDIR_DEBUG)\\RakNet\\RakString.o

$(OBJDIR_DEBUG)\\RakNet\\RakSleep.o: RakNet\\RakSleep.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\RakSleep.cpp -o $(OBJDIR_DEBUG)\\RakNet\\RakSleep.o

$(OBJDIR_DEBUG)\\RakNet\\RakPeer.o: RakNet\\RakPeer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\RakPeer.cpp -o $(OBJDIR_DEBUG)\\RakNet\\RakPeer.o

$(OBJDIR_DEBUG)\\RakNet\\RakNetTypes.o: RakNet\\RakNetTypes.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\RakNetTypes.cpp -o $(OBJDIR_DEBUG)\\RakNet\\RakNetTypes.o

$(OBJDIR_DEBUG)\\RakNet\\SimpleMutex.o: RakNet\\SimpleMutex.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\SimpleMutex.cpp -o $(OBJDIR_DEBUG)\\RakNet\\SimpleMutex.o

$(OBJDIR_DEBUG)\\RakNet\\SignaledEvent.o: RakNet\\SignaledEvent.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\SignaledEvent.cpp -o $(OBJDIR_DEBUG)\\RakNet\\SignaledEvent.o

$(OBJDIR_DEBUG)\\RakNet\\SendToThread.o: RakNet\\SendToThread.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\SendToThread.cpp -o $(OBJDIR_DEBUG)\\RakNet\\SendToThread.o

$(OBJDIR_DEBUG)\\RakNet\\SecureHandshake.o: RakNet\\SecureHandshake.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\SecureHandshake.cpp -o $(OBJDIR_DEBUG)\\RakNet\\SecureHandshake.o

$(OBJDIR_DEBUG)\\RakNet\\SHA1.o: RakNet\\SHA1.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\SHA1.cpp -o $(OBJDIR_DEBUG)\\RakNet\\SHA1.o

$(OBJDIR_DEBUG)\\RakNet\\Router2.o: RakNet\\Router2.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\Router2.cpp -o $(OBJDIR_DEBUG)\\RakNet\\Router2.o

$(OBJDIR_DEBUG)\\RakNet\\ReplicaManager3.o: RakNet\\ReplicaManager3.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\ReplicaManager3.cpp -o $(OBJDIR_DEBUG)\\RakNet\\ReplicaManager3.o

$(OBJDIR_DEBUG)\\RakNet\\ReliabilityLayer.o: RakNet\\ReliabilityLayer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\ReliabilityLayer.cpp -o $(OBJDIR_DEBUG)\\RakNet\\ReliabilityLayer.o

$(OBJDIR_DEBUG)\\RakNet\\RPC4Plugin.o: RakNet\\RPC4Plugin.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\RPC4Plugin.cpp -o $(OBJDIR_DEBUG)\\RakNet\\RPC4Plugin.o

$(OBJDIR_DEBUG)\\RakNet\\PluginInterface2.o: RakNet\\PluginInterface2.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\PluginInterface2.cpp -o $(OBJDIR_DEBUG)\\RakNet\\PluginInterface2.o

$(OBJDIR_DEBUG)\\RakNet\\PacketizedTCP.o: RakNet\\PacketizedTCP.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\PacketizedTCP.cpp -o $(OBJDIR_DEBUG)\\RakNet\\PacketizedTCP.o

$(OBJDIR_DEBUG)\\RakNet\\PacketOutputWindowLogger.o: RakNet\\PacketOutputWindowLogger.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\PacketOutputWindowLogger.cpp -o $(OBJDIR_DEBUG)\\RakNet\\PacketOutputWindowLogger.o

$(OBJDIR_DEBUG)\\RakNet\\PacketLogger.o: RakNet\\PacketLogger.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\PacketLogger.cpp -o $(OBJDIR_DEBUG)\\RakNet\\PacketLogger.o

$(OBJDIR_DEBUG)\\RakNet\\PacketFileLogger.o: RakNet\\PacketFileLogger.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\PacketFileLogger.cpp -o $(OBJDIR_DEBUG)\\RakNet\\PacketFileLogger.o

$(OBJDIR_DEBUG)\\RakNet\\PacketConsoleLogger.o: RakNet\\PacketConsoleLogger.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\PacketConsoleLogger.cpp -o $(OBJDIR_DEBUG)\\RakNet\\PacketConsoleLogger.o

$(OBJDIR_DEBUG)\\RakNet\\NetworkIDObject.o: RakNet\\NetworkIDObject.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\NetworkIDObject.cpp -o $(OBJDIR_DEBUG)\\RakNet\\NetworkIDObject.o

$(OBJDIR_DEBUG)\\RakNet\\RakNetTransport2.o: RakNet\\RakNetTransport2.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\RakNetTransport2.cpp -o $(OBJDIR_DEBUG)\\RakNet\\RakNetTransport2.o

$(OBJDIR_DEBUG)\\RakNet\\RakNetStatistics.o: RakNet\\RakNetStatistics.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\RakNetStatistics.cpp -o $(OBJDIR_DEBUG)\\RakNet\\RakNetStatistics.o

$(OBJDIR_DEBUG)\\RakNet\\RakNetSocket.o: RakNet\\RakNetSocket.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\RakNetSocket.cpp -o $(OBJDIR_DEBUG)\\RakNet\\RakNetSocket.o

$(OBJDIR_DEBUG)\\RakNet\\RakNetCommandParser.o: RakNet\\RakNetCommandParser.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\RakNetCommandParser.cpp -o $(OBJDIR_DEBUG)\\RakNet\\RakNetCommandParser.o

$(OBJDIR_DEBUG)\\RakNet\\RakMemoryOverride.o: RakNet\\RakMemoryOverride.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\RakMemoryOverride.cpp -o $(OBJDIR_DEBUG)\\RakNet\\RakMemoryOverride.o

$(OBJDIR_DEBUG)\\RakNet\\Rackspace.o: RakNet\\Rackspace.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\Rackspace.cpp -o $(OBJDIR_DEBUG)\\RakNet\\Rackspace.o

$(OBJDIR_DEBUG)\\Utils.o: Utils.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Utils.cpp -o $(OBJDIR_DEBUG)\\Utils.o

$(OBJDIR_DEBUG)\\Shared.o: Shared.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Shared.cpp -o $(OBJDIR_DEBUG)\\Shared.o

$(OBJDIR_DEBUG)\\ServerEntry.o: ServerEntry.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c ServerEntry.cpp -o $(OBJDIR_DEBUG)\\ServerEntry.o

$(OBJDIR_DEBUG)\\Reference.o: Reference.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Reference.cpp -o $(OBJDIR_DEBUG)\\Reference.o

$(OBJDIR_DEBUG)\\RakNet\\rdlmalloc.o: RakNet\\rdlmalloc.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\rdlmalloc.cpp -o $(OBJDIR_DEBUG)\\RakNet\\rdlmalloc.o

$(OBJDIR_DEBUG)\\RakNet\\gettimeofday.o: RakNet\\gettimeofday.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\gettimeofday.cpp -o $(OBJDIR_DEBUG)\\RakNet\\gettimeofday.o

$(OBJDIR_DEBUG)\\RakNet\\_FindFirst.o: RakNet\\_FindFirst.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\_FindFirst.cpp -o $(OBJDIR_DEBUG)\\RakNet\\_FindFirst.o

$(OBJDIR_DEBUG)\\RakNet\\WSAStartupSingleton.o: RakNet\\WSAStartupSingleton.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\WSAStartupSingleton.cpp -o $(OBJDIR_DEBUG)\\RakNet\\WSAStartupSingleton.o

$(OBJDIR_DEBUG)\\vaultmp.o: vaultmp.rc vaultmp.cpp
	$(WINDRES) $(RCFLAGS_DEBUG) $(RESINC_DEBUG) -o $(OBJDIR_DEBUG)\\vaultmprc.o vaultmp.rc
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c vaultmp.cpp -o $(OBJDIR_DEBUG)\\vaultmp.o

$(OBJDIR_DEBUG)\\stack_trace\\src\\stack.o: stack_trace\\src\\stack.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c stack_trace\\src\\stack.cpp -o $(OBJDIR_DEBUG)\\stack_trace\\src\\stack.o

$(OBJDIR_DEBUG)\\iniparser\\src\\iniparser.o: iniparser\\src\\iniparser.c
	$(CC) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c iniparser\\src\\iniparser.c -o $(OBJDIR_DEBUG)\\iniparser\\src\\iniparser.o

$(OBJDIR_DEBUG)\\iniparser\\src\\dictionary.o: iniparser\\src\\dictionary.c
	$(CC) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c iniparser\\src\\dictionary.c -o $(OBJDIR_DEBUG)\\iniparser\\src\\dictionary.o

$(OBJDIR_DEBUG)\\VaultVector.o: VaultVector.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c VaultVector.cpp -o $(OBJDIR_DEBUG)\\VaultVector.o

$(OBJDIR_DEBUG)\\VaultFunctor.o: VaultFunctor.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c VaultFunctor.cpp -o $(OBJDIR_DEBUG)\\VaultFunctor.o

$(OBJDIR_DEBUG)\\VaultException.o: VaultException.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c VaultException.cpp -o $(OBJDIR_DEBUG)\\VaultException.o

$(OBJDIR_DEBUG)\\Value.o: Value.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Value.cpp -o $(OBJDIR_DEBUG)\\Value.o

$(OBJDIR_DEBUG)\\RakNet\\TelnetTransport.o: RakNet\\TelnetTransport.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\TelnetTransport.cpp -o $(OBJDIR_DEBUG)\\RakNet\\TelnetTransport.o

$(OBJDIR_DEBUG)\\RakNet\\TeamBalancer.o: RakNet\\TeamBalancer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\TeamBalancer.cpp -o $(OBJDIR_DEBUG)\\RakNet\\TeamBalancer.o

$(OBJDIR_DEBUG)\\RakNet\\TableSerializer.o: RakNet\\TableSerializer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\TableSerializer.cpp -o $(OBJDIR_DEBUG)\\RakNet\\TableSerializer.o

$(OBJDIR_DEBUG)\\RakNet\\TCPInterface.o: RakNet\\TCPInterface.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\TCPInterface.cpp -o $(OBJDIR_DEBUG)\\RakNet\\TCPInterface.o

$(OBJDIR_DEBUG)\\RakNet\\SuperFastHash.o: RakNet\\SuperFastHash.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\SuperFastHash.cpp -o $(OBJDIR_DEBUG)\\RakNet\\SuperFastHash.o

$(OBJDIR_DEBUG)\\RakNet\\StringTable.o: RakNet\\StringTable.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\StringTable.cpp -o $(OBJDIR_DEBUG)\\RakNet\\StringTable.o

$(OBJDIR_DEBUG)\\RakNet\\StringCompressor.o: RakNet\\StringCompressor.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\StringCompressor.cpp -o $(OBJDIR_DEBUG)\\RakNet\\StringCompressor.o

$(OBJDIR_DEBUG)\\RakNet\\SocketLayer.o: RakNet\\SocketLayer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\SocketLayer.cpp -o $(OBJDIR_DEBUG)\\RakNet\\SocketLayer.o

$(OBJDIR_DEBUG)\\RakNet\\VariadicSQLParser.o: RakNet\\VariadicSQLParser.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\VariadicSQLParser.cpp -o $(OBJDIR_DEBUG)\\RakNet\\VariadicSQLParser.o

$(OBJDIR_DEBUG)\\RakNet\\VariableListDeltaTracker.o: RakNet\\VariableListDeltaTracker.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\VariableListDeltaTracker.cpp -o $(OBJDIR_DEBUG)\\RakNet\\VariableListDeltaTracker.o

$(OBJDIR_DEBUG)\\RakNet\\VariableDeltaSerializer.o: RakNet\\VariableDeltaSerializer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\VariableDeltaSerializer.cpp -o $(OBJDIR_DEBUG)\\RakNet\\VariableDeltaSerializer.o

$(OBJDIR_DEBUG)\\RakNet\\UDPProxyServer.o: RakNet\\UDPProxyServer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\UDPProxyServer.cpp -o $(OBJDIR_DEBUG)\\RakNet\\UDPProxyServer.o

$(OBJDIR_DEBUG)\\RakNet\\UDPProxyCoordinator.o: RakNet\\UDPProxyCoordinator.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\UDPProxyCoordinator.cpp -o $(OBJDIR_DEBUG)\\RakNet\\UDPProxyCoordinator.o

$(OBJDIR_DEBUG)\\RakNet\\UDPProxyClient.o: RakNet\\UDPProxyClient.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\UDPProxyClient.cpp -o $(OBJDIR_DEBUG)\\RakNet\\UDPProxyClient.o

$(OBJDIR_DEBUG)\\RakNet\\UDPForwarder.o: RakNet\\UDPForwarder.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\UDPForwarder.cpp -o $(OBJDIR_DEBUG)\\RakNet\\UDPForwarder.o

$(OBJDIR_DEBUG)\\RakNet\\TwoWayAuthentication.o: RakNet\\TwoWayAuthentication.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\TwoWayAuthentication.cpp -o $(OBJDIR_DEBUG)\\RakNet\\TwoWayAuthentication.o

$(OBJDIR_DEBUG)\\RakNet\\ThreadsafePacketLogger.o: RakNet\\ThreadsafePacketLogger.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\ThreadsafePacketLogger.cpp -o $(OBJDIR_DEBUG)\\RakNet\\ThreadsafePacketLogger.o

$(OBJDIR_DEBUG)\\RakNet\\ConnectionGraph2.o: RakNet\\ConnectionGraph2.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\ConnectionGraph2.cpp -o $(OBJDIR_DEBUG)\\RakNet\\ConnectionGraph2.o

$(OBJDIR_DEBUG)\\RakNet\\CommandParserInterface.o: RakNet\\CommandParserInterface.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\CommandParserInterface.cpp -o $(OBJDIR_DEBUG)\\RakNet\\CommandParserInterface.o

$(OBJDIR_DEBUG)\\RakNet\\CloudServer.o: RakNet\\CloudServer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\CloudServer.cpp -o $(OBJDIR_DEBUG)\\RakNet\\CloudServer.o

$(OBJDIR_DEBUG)\\RakNet\\CloudCommon.o: RakNet\\CloudCommon.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\CloudCommon.cpp -o $(OBJDIR_DEBUG)\\RakNet\\CloudCommon.o

$(OBJDIR_DEBUG)\\RakNet\\CloudClient.o: RakNet\\CloudClient.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\CloudClient.cpp -o $(OBJDIR_DEBUG)\\RakNet\\CloudClient.o

$(OBJDIR_DEBUG)\\RakNet\\CheckSum.o: RakNet\\CheckSum.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\CheckSum.cpp -o $(OBJDIR_DEBUG)\\RakNet\\CheckSum.o

$(OBJDIR_DEBUG)\\RakNet\\CCRakNetUDT.o: RakNet\\CCRakNetUDT.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\CCRakNetUDT.cpp -o $(OBJDIR_DEBUG)\\RakNet\\CCRakNetUDT.o

$(OBJDIR_DEBUG)\\RakNet\\CCRakNetSlidingWindow.o: RakNet\\CCRakNetSlidingWindow.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\CCRakNetSlidingWindow.cpp -o $(OBJDIR_DEBUG)\\RakNet\\CCRakNetSlidingWindow.o

$(OBJDIR_DEBUG)\\RakNet\\BitStream.o: RakNet\\BitStream.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\BitStream.cpp -o $(OBJDIR_DEBUG)\\RakNet\\BitStream.o

$(OBJDIR_DEBUG)\\RakNet\\DS_HuffmanEncodingTree.o: RakNet\\DS_HuffmanEncodingTree.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\DS_HuffmanEncodingTree.cpp -o $(OBJDIR_DEBUG)\\RakNet\\DS_HuffmanEncodingTree.o

$(OBJDIR_DEBUG)\\RakNet\\DS_ByteQueue.o: RakNet\\DS_ByteQueue.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\DS_ByteQueue.cpp -o $(OBJDIR_DEBUG)\\RakNet\\DS_ByteQueue.o

$(OBJDIR_DEBUG)\\RakNet\\DS_BytePool.o: RakNet\\DS_BytePool.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\DS_BytePool.cpp -o $(OBJDIR_DEBUG)\\RakNet\\DS_BytePool.o

$(OBJDIR_DEBUG)\\RakNet\\ConsoleServer.o: RakNet\\ConsoleServer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\ConsoleServer.cpp -o $(OBJDIR_DEBUG)\\RakNet\\ConsoleServer.o

$(OBJDIR_DEBUG)\\Guarded.o: Guarded.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Guarded.cpp -o $(OBJDIR_DEBUG)\\Guarded.o

$(OBJDIR_DEBUG)\\GameFactory.o: GameFactory.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c GameFactory.cpp -o $(OBJDIR_DEBUG)\\GameFactory.o

$(OBJDIR_DEBUG)\\Game.o: Game.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Game.cpp -o $(OBJDIR_DEBUG)\\Game.o

$(OBJDIR_DEBUG)\\Debug.o: Debug.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Debug.cpp -o $(OBJDIR_DEBUG)\\Debug.o

$(OBJDIR_DEBUG)\\CriticalSection.o: CriticalSection.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c CriticalSection.cpp -o $(OBJDIR_DEBUG)\\CriticalSection.o

$(OBJDIR_DEBUG)\\Container.o: Container.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Container.cpp -o $(OBJDIR_DEBUG)\\Container.o

$(OBJDIR_DEBUG)\\Bethesda.o: Bethesda.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Bethesda.cpp -o $(OBJDIR_DEBUG)\\Bethesda.o

$(OBJDIR_DEBUG)\\Actor.o: Actor.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Actor.cpp -o $(OBJDIR_DEBUG)\\Actor.o

$(OBJDIR_DEBUG)\\API.o: API.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c API.cpp -o $(OBJDIR_DEBUG)\\API.o

$(OBJDIR_DEBUG)\\Player.o: Player.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Player.cpp -o $(OBJDIR_DEBUG)\\Player.o

$(OBJDIR_DEBUG)\\Pipe.o: Pipe.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Pipe.cpp -o $(OBJDIR_DEBUG)\\Pipe.o

$(OBJDIR_DEBUG)\\PacketFactory.o: PacketFactory.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c PacketFactory.cpp -o $(OBJDIR_DEBUG)\\PacketFactory.o

$(OBJDIR_DEBUG)\\Object.o: Object.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Object.cpp -o $(OBJDIR_DEBUG)\\Object.o

$(OBJDIR_DEBUG)\\NetworkClient.o: NetworkClient.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c NetworkClient.cpp -o $(OBJDIR_DEBUG)\\NetworkClient.o

$(OBJDIR_DEBUG)\\Network.o: Network.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Network.cpp -o $(OBJDIR_DEBUG)\\Network.o

$(OBJDIR_DEBUG)\\Lockable.o: Lockable.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Lockable.cpp -o $(OBJDIR_DEBUG)\\Lockable.o

$(OBJDIR_DEBUG)\\Item.o: Item.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Item.cpp -o $(OBJDIR_DEBUG)\\Item.o

$(OBJDIR_DEBUG)\\Interface.o: Interface.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Interface.cpp -o $(OBJDIR_DEBUG)\\Interface.o

$(OBJDIR_DEBUG)\\RakNet\\LogCommandParser.o: RakNet\\LogCommandParser.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\LogCommandParser.cpp -o $(OBJDIR_DEBUG)\\RakNet\\LogCommandParser.o

$(OBJDIR_DEBUG)\\RakNet\\LocklessTypes.o: RakNet\\LocklessTypes.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\LocklessTypes.cpp -o $(OBJDIR_DEBUG)\\RakNet\\LocklessTypes.o

$(OBJDIR_DEBUG)\\RakNet\\LinuxStrings.o: RakNet\\LinuxStrings.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\LinuxStrings.cpp -o $(OBJDIR_DEBUG)\\RakNet\\LinuxStrings.o

$(OBJDIR_DEBUG)\\RakNet\\Itoa.o: RakNet\\Itoa.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\Itoa.cpp -o $(OBJDIR_DEBUG)\\RakNet\\Itoa.o

$(OBJDIR_DEBUG)\\RakNet\\IncrementalReadInterface.o: RakNet\\IncrementalReadInterface.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\IncrementalReadInterface.cpp -o $(OBJDIR_DEBUG)\\RakNet\\IncrementalReadInterface.o

$(OBJDIR_DEBUG)\\RakNet\\HTTPConnection.o: RakNet\\HTTPConnection.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\HTTPConnection.cpp -o $(OBJDIR_DEBUG)\\RakNet\\HTTPConnection.o

$(OBJDIR_DEBUG)\\RakNet\\GridSectorizer.o: RakNet\\GridSectorizer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\GridSectorizer.cpp -o $(OBJDIR_DEBUG)\\RakNet\\GridSectorizer.o

$(OBJDIR_DEBUG)\\RakNet\\Gets.o: RakNet\\Gets.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\Gets.cpp -o $(OBJDIR_DEBUG)\\RakNet\\Gets.o

$(OBJDIR_DEBUG)\\RakNet\\Getche.o: RakNet\\Getche.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\Getche.cpp -o $(OBJDIR_DEBUG)\\RakNet\\Getche.o

$(OBJDIR_DEBUG)\\RakNet\\NetworkIDManager.o: RakNet\\NetworkIDManager.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\NetworkIDManager.cpp -o $(OBJDIR_DEBUG)\\RakNet\\NetworkIDManager.o

$(OBJDIR_DEBUG)\\RakNet\\NatTypeDetectionServer.o: RakNet\\NatTypeDetectionServer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\NatTypeDetectionServer.cpp -o $(OBJDIR_DEBUG)\\RakNet\\NatTypeDetectionServer.o

$(OBJDIR_DEBUG)\\RakNet\\NatTypeDetectionCommon.o: RakNet\\NatTypeDetectionCommon.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\NatTypeDetectionCommon.cpp -o $(OBJDIR_DEBUG)\\RakNet\\NatTypeDetectionCommon.o

$(OBJDIR_DEBUG)\\RakNet\\NatTypeDetectionClient.o: RakNet\\NatTypeDetectionClient.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\NatTypeDetectionClient.cpp -o $(OBJDIR_DEBUG)\\RakNet\\NatTypeDetectionClient.o

$(OBJDIR_DEBUG)\\RakNet\\NatPunchthroughServer.o: RakNet\\NatPunchthroughServer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\NatPunchthroughServer.cpp -o $(OBJDIR_DEBUG)\\RakNet\\NatPunchthroughServer.o

$(OBJDIR_DEBUG)\\RakNet\\NatPunchthroughClient.o: RakNet\\NatPunchthroughClient.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\NatPunchthroughClient.cpp -o $(OBJDIR_DEBUG)\\RakNet\\NatPunchthroughClient.o

$(OBJDIR_DEBUG)\\RakNet\\MessageFilter.o: RakNet\\MessageFilter.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\MessageFilter.cpp -o $(OBJDIR_DEBUG)\\RakNet\\MessageFilter.o

$(OBJDIR_DEBUG)\\RakNet\\EmailSender.o: RakNet\\EmailSender.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\EmailSender.cpp -o $(OBJDIR_DEBUG)\\RakNet\\EmailSender.o

$(OBJDIR_DEBUG)\\RakNet\\DynDNS.o: RakNet\\DynDNS.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\DynDNS.cpp -o $(OBJDIR_DEBUG)\\RakNet\\DynDNS.o

$(OBJDIR_DEBUG)\\RakNet\\DirectoryDeltaTransfer.o: RakNet\\DirectoryDeltaTransfer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\DirectoryDeltaTransfer.cpp -o $(OBJDIR_DEBUG)\\RakNet\\DirectoryDeltaTransfer.o

$(OBJDIR_DEBUG)\\RakNet\\DataCompressor.o: RakNet\\DataCompressor.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\DataCompressor.cpp -o $(OBJDIR_DEBUG)\\RakNet\\DataCompressor.o

$(OBJDIR_DEBUG)\\RakNet\\DS_Table.o: RakNet\\DS_Table.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\DS_Table.cpp -o $(OBJDIR_DEBUG)\\RakNet\\DS_Table.o

$(OBJDIR_DEBUG)\\RakNet\\GetTime.o: RakNet\\GetTime.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\GetTime.cpp -o $(OBJDIR_DEBUG)\\RakNet\\GetTime.o

$(OBJDIR_DEBUG)\\RakNet\\FullyConnectedMesh2.o: RakNet\\FullyConnectedMesh2.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\FullyConnectedMesh2.cpp -o $(OBJDIR_DEBUG)\\RakNet\\FullyConnectedMesh2.o

$(OBJDIR_DEBUG)\\RakNet\\FormatString.o: RakNet\\FormatString.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\FormatString.cpp -o $(OBJDIR_DEBUG)\\RakNet\\FormatString.o

$(OBJDIR_DEBUG)\\RakNet\\FileOperations.o: RakNet\\FileOperations.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\FileOperations.cpp -o $(OBJDIR_DEBUG)\\RakNet\\FileOperations.o

$(OBJDIR_DEBUG)\\RakNet\\FileListTransfer.o: RakNet\\FileListTransfer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\FileListTransfer.cpp -o $(OBJDIR_DEBUG)\\RakNet\\FileListTransfer.o

$(OBJDIR_DEBUG)\\RakNet\\FileList.o: RakNet\\FileList.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\FileList.cpp -o $(OBJDIR_DEBUG)\\RakNet\\FileList.o

$(OBJDIR_DEBUG)\\RakNet\\EpochTimeToString.o: RakNet\\EpochTimeToString.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\EpochTimeToString.cpp -o $(OBJDIR_DEBUG)\\RakNet\\EpochTimeToString.o

$(OBJDIR_DEBUG)\\RakNet\\EncodeClassName.o: RakNet\\EncodeClassName.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNet\\EncodeClassName.cpp -o $(OBJDIR_DEBUG)\\RakNet\\EncodeClassName.o

clean_debug:
	cmd /c del /f $(OBJ_DEBUG) $(OUT_DEBUG)
	cmd /c rd $(OBJDIR_DEBUG)\\RakNet
	cmd /c rd $(OBJDIR_DEBUG)
	cmd /c rd $(OBJDIR_DEBUG)\\stack_trace\\src
	cmd /c rd $(OBJDIR_DEBUG)\\iniparser\\src

before_release:
	cmd /c if not exist $(OBJDIR_RELEASE)\\RakNet md $(OBJDIR_RELEASE)\\RakNet
	cmd /c if not exist $(OBJDIR_RELEASE) md $(OBJDIR_RELEASE)
	cmd /c if not exist $(OBJDIR_RELEASE)\\iniparser\\src md $(OBJDIR_RELEASE)\\iniparser\\src

after_release:

release: before_release out_release after_release

out_release: $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) $(LIBDIR_RELEASE) $(OBJ_RELEASE) $(LIB_RELEASE) -o $(OUT_RELEASE) $(LDFLAGS_RELEASE)

$(OBJDIR_RELEASE)\\RakNet\\ReadyEvent.o: RakNet\\ReadyEvent.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\ReadyEvent.cpp -o $(OBJDIR_RELEASE)\\RakNet\\ReadyEvent.o

$(OBJDIR_RELEASE)\\RakNet\\Rand.o: RakNet\\Rand.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\Rand.cpp -o $(OBJDIR_RELEASE)\\RakNet\\Rand.o

$(OBJDIR_RELEASE)\\RakNet\\RakWString.o: RakNet\\RakWString.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\RakWString.cpp -o $(OBJDIR_RELEASE)\\RakNet\\RakWString.o

$(OBJDIR_RELEASE)\\RakNet\\RakThread.o: RakNet\\RakThread.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\RakThread.cpp -o $(OBJDIR_RELEASE)\\RakNet\\RakThread.o

$(OBJDIR_RELEASE)\\RakNet\\RakString.o: RakNet\\RakString.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\RakString.cpp -o $(OBJDIR_RELEASE)\\RakNet\\RakString.o

$(OBJDIR_RELEASE)\\RakNet\\RakSleep.o: RakNet\\RakSleep.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\RakSleep.cpp -o $(OBJDIR_RELEASE)\\RakNet\\RakSleep.o

$(OBJDIR_RELEASE)\\RakNet\\RakPeer.o: RakNet\\RakPeer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\RakPeer.cpp -o $(OBJDIR_RELEASE)\\RakNet\\RakPeer.o

$(OBJDIR_RELEASE)\\RakNet\\RakNetTypes.o: RakNet\\RakNetTypes.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\RakNetTypes.cpp -o $(OBJDIR_RELEASE)\\RakNet\\RakNetTypes.o

$(OBJDIR_RELEASE)\\RakNet\\SimpleMutex.o: RakNet\\SimpleMutex.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\SimpleMutex.cpp -o $(OBJDIR_RELEASE)\\RakNet\\SimpleMutex.o

$(OBJDIR_RELEASE)\\RakNet\\SignaledEvent.o: RakNet\\SignaledEvent.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\SignaledEvent.cpp -o $(OBJDIR_RELEASE)\\RakNet\\SignaledEvent.o

$(OBJDIR_RELEASE)\\RakNet\\SendToThread.o: RakNet\\SendToThread.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\SendToThread.cpp -o $(OBJDIR_RELEASE)\\RakNet\\SendToThread.o

$(OBJDIR_RELEASE)\\RakNet\\SecureHandshake.o: RakNet\\SecureHandshake.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\SecureHandshake.cpp -o $(OBJDIR_RELEASE)\\RakNet\\SecureHandshake.o

$(OBJDIR_RELEASE)\\RakNet\\SHA1.o: RakNet\\SHA1.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\SHA1.cpp -o $(OBJDIR_RELEASE)\\RakNet\\SHA1.o

$(OBJDIR_RELEASE)\\RakNet\\Router2.o: RakNet\\Router2.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\Router2.cpp -o $(OBJDIR_RELEASE)\\RakNet\\Router2.o

$(OBJDIR_RELEASE)\\RakNet\\ReplicaManager3.o: RakNet\\ReplicaManager3.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\ReplicaManager3.cpp -o $(OBJDIR_RELEASE)\\RakNet\\ReplicaManager3.o

$(OBJDIR_RELEASE)\\RakNet\\ReliabilityLayer.o: RakNet\\ReliabilityLayer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\ReliabilityLayer.cpp -o $(OBJDIR_RELEASE)\\RakNet\\ReliabilityLayer.o

$(OBJDIR_RELEASE)\\RakNet\\RPC4Plugin.o: RakNet\\RPC4Plugin.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\RPC4Plugin.cpp -o $(OBJDIR_RELEASE)\\RakNet\\RPC4Plugin.o

$(OBJDIR_RELEASE)\\RakNet\\PluginInterface2.o: RakNet\\PluginInterface2.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\PluginInterface2.cpp -o $(OBJDIR_RELEASE)\\RakNet\\PluginInterface2.o

$(OBJDIR_RELEASE)\\RakNet\\PacketizedTCP.o: RakNet\\PacketizedTCP.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\PacketizedTCP.cpp -o $(OBJDIR_RELEASE)\\RakNet\\PacketizedTCP.o

$(OBJDIR_RELEASE)\\RakNet\\PacketOutputWindowLogger.o: RakNet\\PacketOutputWindowLogger.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\PacketOutputWindowLogger.cpp -o $(OBJDIR_RELEASE)\\RakNet\\PacketOutputWindowLogger.o

$(OBJDIR_RELEASE)\\RakNet\\PacketLogger.o: RakNet\\PacketLogger.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\PacketLogger.cpp -o $(OBJDIR_RELEASE)\\RakNet\\PacketLogger.o

$(OBJDIR_RELEASE)\\RakNet\\PacketFileLogger.o: RakNet\\PacketFileLogger.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\PacketFileLogger.cpp -o $(OBJDIR_RELEASE)\\RakNet\\PacketFileLogger.o

$(OBJDIR_RELEASE)\\RakNet\\PacketConsoleLogger.o: RakNet\\PacketConsoleLogger.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\PacketConsoleLogger.cpp -o $(OBJDIR_RELEASE)\\RakNet\\PacketConsoleLogger.o

$(OBJDIR_RELEASE)\\RakNet\\NetworkIDObject.o: RakNet\\NetworkIDObject.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\NetworkIDObject.cpp -o $(OBJDIR_RELEASE)\\RakNet\\NetworkIDObject.o

$(OBJDIR_RELEASE)\\RakNet\\RakNetTransport2.o: RakNet\\RakNetTransport2.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\RakNetTransport2.cpp -o $(OBJDIR_RELEASE)\\RakNet\\RakNetTransport2.o

$(OBJDIR_RELEASE)\\RakNet\\RakNetStatistics.o: RakNet\\RakNetStatistics.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\RakNetStatistics.cpp -o $(OBJDIR_RELEASE)\\RakNet\\RakNetStatistics.o

$(OBJDIR_RELEASE)\\RakNet\\RakNetSocket.o: RakNet\\RakNetSocket.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\RakNetSocket.cpp -o $(OBJDIR_RELEASE)\\RakNet\\RakNetSocket.o

$(OBJDIR_RELEASE)\\RakNet\\RakNetCommandParser.o: RakNet\\RakNetCommandParser.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\RakNetCommandParser.cpp -o $(OBJDIR_RELEASE)\\RakNet\\RakNetCommandParser.o

$(OBJDIR_RELEASE)\\RakNet\\RakMemoryOverride.o: RakNet\\RakMemoryOverride.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\RakMemoryOverride.cpp -o $(OBJDIR_RELEASE)\\RakNet\\RakMemoryOverride.o

$(OBJDIR_RELEASE)\\RakNet\\Rackspace.o: RakNet\\Rackspace.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\Rackspace.cpp -o $(OBJDIR_RELEASE)\\RakNet\\Rackspace.o

$(OBJDIR_RELEASE)\\Utils.o: Utils.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Utils.cpp -o $(OBJDIR_RELEASE)\\Utils.o

$(OBJDIR_RELEASE)\\Shared.o: Shared.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Shared.cpp -o $(OBJDIR_RELEASE)\\Shared.o

$(OBJDIR_RELEASE)\\ServerEntry.o: ServerEntry.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ServerEntry.cpp -o $(OBJDIR_RELEASE)\\ServerEntry.o

$(OBJDIR_RELEASE)\\Reference.o: Reference.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Reference.cpp -o $(OBJDIR_RELEASE)\\Reference.o

$(OBJDIR_RELEASE)\\RakNet\\rdlmalloc.o: RakNet\\rdlmalloc.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\rdlmalloc.cpp -o $(OBJDIR_RELEASE)\\RakNet\\rdlmalloc.o

$(OBJDIR_RELEASE)\\RakNet\\gettimeofday.o: RakNet\\gettimeofday.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\gettimeofday.cpp -o $(OBJDIR_RELEASE)\\RakNet\\gettimeofday.o

$(OBJDIR_RELEASE)\\RakNet\\_FindFirst.o: RakNet\\_FindFirst.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\_FindFirst.cpp -o $(OBJDIR_RELEASE)\\RakNet\\_FindFirst.o

$(OBJDIR_RELEASE)\\RakNet\\WSAStartupSingleton.o: RakNet\\WSAStartupSingleton.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\WSAStartupSingleton.cpp -o $(OBJDIR_RELEASE)\\RakNet\\WSAStartupSingleton.o

$(OBJDIR_RELEASE)\\vaultmp.o: vaultmp.rc vaultmp.cpp
	$(WINDRES) $(RCFLAGS_RELEASE) $(RESINC_RELEASE) vaultmp.rc -o $(OBJDIR_RELEASE)\\vaultmprc.o
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c vaultmp.cpp -o $(OBJDIR_RELEASE)\\vaultmp.o

$(OBJDIR_RELEASE)\\iniparser\\src\\iniparser.o: iniparser\\src\\iniparser.c
	$(CC) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c iniparser\\src\\iniparser.c -o $(OBJDIR_RELEASE)\\iniparser\\src\\iniparser.o

$(OBJDIR_RELEASE)\\iniparser\\src\\dictionary.o: iniparser\\src\\dictionary.c
	$(CC) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c iniparser\\src\\dictionary.c -o $(OBJDIR_RELEASE)\\iniparser\\src\\dictionary.o

$(OBJDIR_RELEASE)\\VaultVector.o: VaultVector.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c VaultVector.cpp -o $(OBJDIR_RELEASE)\\VaultVector.o

$(OBJDIR_RELEASE)\\VaultFunctor.o: VaultFunctor.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c VaultFunctor.cpp -o $(OBJDIR_RELEASE)\\VaultFunctor.o

$(OBJDIR_RELEASE)\\VaultException.o: VaultException.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c VaultException.cpp -o $(OBJDIR_RELEASE)\\VaultException.o

$(OBJDIR_RELEASE)\\Value.o: Value.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Value.cpp -o $(OBJDIR_RELEASE)\\Value.o

$(OBJDIR_RELEASE)\\RakNet\\TelnetTransport.o: RakNet\\TelnetTransport.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\TelnetTransport.cpp -o $(OBJDIR_RELEASE)\\RakNet\\TelnetTransport.o

$(OBJDIR_RELEASE)\\RakNet\\TeamBalancer.o: RakNet\\TeamBalancer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\TeamBalancer.cpp -o $(OBJDIR_RELEASE)\\RakNet\\TeamBalancer.o

$(OBJDIR_RELEASE)\\RakNet\\TableSerializer.o: RakNet\\TableSerializer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\TableSerializer.cpp -o $(OBJDIR_RELEASE)\\RakNet\\TableSerializer.o

$(OBJDIR_RELEASE)\\RakNet\\TCPInterface.o: RakNet\\TCPInterface.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\TCPInterface.cpp -o $(OBJDIR_RELEASE)\\RakNet\\TCPInterface.o

$(OBJDIR_RELEASE)\\RakNet\\SuperFastHash.o: RakNet\\SuperFastHash.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\SuperFastHash.cpp -o $(OBJDIR_RELEASE)\\RakNet\\SuperFastHash.o

$(OBJDIR_RELEASE)\\RakNet\\StringTable.o: RakNet\\StringTable.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\StringTable.cpp -o $(OBJDIR_RELEASE)\\RakNet\\StringTable.o

$(OBJDIR_RELEASE)\\RakNet\\StringCompressor.o: RakNet\\StringCompressor.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\StringCompressor.cpp -o $(OBJDIR_RELEASE)\\RakNet\\StringCompressor.o

$(OBJDIR_RELEASE)\\RakNet\\SocketLayer.o: RakNet\\SocketLayer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\SocketLayer.cpp -o $(OBJDIR_RELEASE)\\RakNet\\SocketLayer.o

$(OBJDIR_RELEASE)\\RakNet\\VariadicSQLParser.o: RakNet\\VariadicSQLParser.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\VariadicSQLParser.cpp -o $(OBJDIR_RELEASE)\\RakNet\\VariadicSQLParser.o

$(OBJDIR_RELEASE)\\RakNet\\VariableListDeltaTracker.o: RakNet\\VariableListDeltaTracker.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\VariableListDeltaTracker.cpp -o $(OBJDIR_RELEASE)\\RakNet\\VariableListDeltaTracker.o

$(OBJDIR_RELEASE)\\RakNet\\VariableDeltaSerializer.o: RakNet\\VariableDeltaSerializer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\VariableDeltaSerializer.cpp -o $(OBJDIR_RELEASE)\\RakNet\\VariableDeltaSerializer.o

$(OBJDIR_RELEASE)\\RakNet\\UDPProxyServer.o: RakNet\\UDPProxyServer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\UDPProxyServer.cpp -o $(OBJDIR_RELEASE)\\RakNet\\UDPProxyServer.o

$(OBJDIR_RELEASE)\\RakNet\\UDPProxyCoordinator.o: RakNet\\UDPProxyCoordinator.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\UDPProxyCoordinator.cpp -o $(OBJDIR_RELEASE)\\RakNet\\UDPProxyCoordinator.o

$(OBJDIR_RELEASE)\\RakNet\\UDPProxyClient.o: RakNet\\UDPProxyClient.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\UDPProxyClient.cpp -o $(OBJDIR_RELEASE)\\RakNet\\UDPProxyClient.o

$(OBJDIR_RELEASE)\\RakNet\\UDPForwarder.o: RakNet\\UDPForwarder.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\UDPForwarder.cpp -o $(OBJDIR_RELEASE)\\RakNet\\UDPForwarder.o

$(OBJDIR_RELEASE)\\RakNet\\TwoWayAuthentication.o: RakNet\\TwoWayAuthentication.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\TwoWayAuthentication.cpp -o $(OBJDIR_RELEASE)\\RakNet\\TwoWayAuthentication.o

$(OBJDIR_RELEASE)\\RakNet\\ThreadsafePacketLogger.o: RakNet\\ThreadsafePacketLogger.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\ThreadsafePacketLogger.cpp -o $(OBJDIR_RELEASE)\\RakNet\\ThreadsafePacketLogger.o

$(OBJDIR_RELEASE)\\RakNet\\ConnectionGraph2.o: RakNet\\ConnectionGraph2.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\ConnectionGraph2.cpp -o $(OBJDIR_RELEASE)\\RakNet\\ConnectionGraph2.o

$(OBJDIR_RELEASE)\\RakNet\\CommandParserInterface.o: RakNet\\CommandParserInterface.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\CommandParserInterface.cpp -o $(OBJDIR_RELEASE)\\RakNet\\CommandParserInterface.o

$(OBJDIR_RELEASE)\\RakNet\\CloudServer.o: RakNet\\CloudServer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\CloudServer.cpp -o $(OBJDIR_RELEASE)\\RakNet\\CloudServer.o

$(OBJDIR_RELEASE)\\RakNet\\CloudCommon.o: RakNet\\CloudCommon.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\CloudCommon.cpp -o $(OBJDIR_RELEASE)\\RakNet\\CloudCommon.o

$(OBJDIR_RELEASE)\\RakNet\\CloudClient.o: RakNet\\CloudClient.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\CloudClient.cpp -o $(OBJDIR_RELEASE)\\RakNet\\CloudClient.o

$(OBJDIR_RELEASE)\\RakNet\\CheckSum.o: RakNet\\CheckSum.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\CheckSum.cpp -o $(OBJDIR_RELEASE)\\RakNet\\CheckSum.o

$(OBJDIR_RELEASE)\\RakNet\\CCRakNetUDT.o: RakNet\\CCRakNetUDT.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\CCRakNetUDT.cpp -o $(OBJDIR_RELEASE)\\RakNet\\CCRakNetUDT.o

$(OBJDIR_RELEASE)\\RakNet\\CCRakNetSlidingWindow.o: RakNet\\CCRakNetSlidingWindow.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\CCRakNetSlidingWindow.cpp -o $(OBJDIR_RELEASE)\\RakNet\\CCRakNetSlidingWindow.o

$(OBJDIR_RELEASE)\\RakNet\\BitStream.o: RakNet\\BitStream.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\BitStream.cpp -o $(OBJDIR_RELEASE)\\RakNet\\BitStream.o

$(OBJDIR_RELEASE)\\RakNet\\DS_HuffmanEncodingTree.o: RakNet\\DS_HuffmanEncodingTree.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\DS_HuffmanEncodingTree.cpp -o $(OBJDIR_RELEASE)\\RakNet\\DS_HuffmanEncodingTree.o

$(OBJDIR_RELEASE)\\RakNet\\DS_ByteQueue.o: RakNet\\DS_ByteQueue.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\DS_ByteQueue.cpp -o $(OBJDIR_RELEASE)\\RakNet\\DS_ByteQueue.o

$(OBJDIR_RELEASE)\\RakNet\\DS_BytePool.o: RakNet\\DS_BytePool.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\DS_BytePool.cpp -o $(OBJDIR_RELEASE)\\RakNet\\DS_BytePool.o

$(OBJDIR_RELEASE)\\RakNet\\ConsoleServer.o: RakNet\\ConsoleServer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\ConsoleServer.cpp -o $(OBJDIR_RELEASE)\\RakNet\\ConsoleServer.o

$(OBJDIR_RELEASE)\\Guarded.o: Guarded.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Guarded.cpp -o $(OBJDIR_RELEASE)\\Guarded.o

$(OBJDIR_RELEASE)\\GameFactory.o: GameFactory.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c GameFactory.cpp -o $(OBJDIR_RELEASE)\\GameFactory.o

$(OBJDIR_RELEASE)\\Game.o: Game.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Game.cpp -o $(OBJDIR_RELEASE)\\Game.o

$(OBJDIR_RELEASE)\\Debug.o: Debug.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Debug.cpp -o $(OBJDIR_RELEASE)\\Debug.o

$(OBJDIR_RELEASE)\\CriticalSection.o: CriticalSection.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c CriticalSection.cpp -o $(OBJDIR_RELEASE)\\CriticalSection.o

$(OBJDIR_RELEASE)\\Container.o: Container.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Container.cpp -o $(OBJDIR_RELEASE)\\Container.o

$(OBJDIR_RELEASE)\\Bethesda.o: Bethesda.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Bethesda.cpp -o $(OBJDIR_RELEASE)\\Bethesda.o

$(OBJDIR_RELEASE)\\Actor.o: Actor.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Actor.cpp -o $(OBJDIR_RELEASE)\\Actor.o

$(OBJDIR_RELEASE)\\API.o: API.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c API.cpp -o $(OBJDIR_RELEASE)\\API.o

$(OBJDIR_RELEASE)\\Player.o: Player.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Player.cpp -o $(OBJDIR_RELEASE)\\Player.o

$(OBJDIR_RELEASE)\\Pipe.o: Pipe.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Pipe.cpp -o $(OBJDIR_RELEASE)\\Pipe.o

$(OBJDIR_RELEASE)\\PacketFactory.o: PacketFactory.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c PacketFactory.cpp -o $(OBJDIR_RELEASE)\\PacketFactory.o

$(OBJDIR_RELEASE)\\Object.o: Object.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Object.cpp -o $(OBJDIR_RELEASE)\\Object.o

$(OBJDIR_RELEASE)\\NetworkClient.o: NetworkClient.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c NetworkClient.cpp -o $(OBJDIR_RELEASE)\\NetworkClient.o

$(OBJDIR_RELEASE)\\Network.o: Network.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Network.cpp -o $(OBJDIR_RELEASE)\\Network.o

$(OBJDIR_RELEASE)\\Lockable.o: Lockable.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Lockable.cpp -o $(OBJDIR_RELEASE)\\Lockable.o

$(OBJDIR_RELEASE)\\Item.o: Item.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Item.cpp -o $(OBJDIR_RELEASE)\\Item.o

$(OBJDIR_RELEASE)\\Interface.o: Interface.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Interface.cpp -o $(OBJDIR_RELEASE)\\Interface.o

$(OBJDIR_RELEASE)\\RakNet\\LogCommandParser.o: RakNet\\LogCommandParser.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\LogCommandParser.cpp -o $(OBJDIR_RELEASE)\\RakNet\\LogCommandParser.o

$(OBJDIR_RELEASE)\\RakNet\\LocklessTypes.o: RakNet\\LocklessTypes.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\LocklessTypes.cpp -o $(OBJDIR_RELEASE)\\RakNet\\LocklessTypes.o

$(OBJDIR_RELEASE)\\RakNet\\LinuxStrings.o: RakNet\\LinuxStrings.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\LinuxStrings.cpp -o $(OBJDIR_RELEASE)\\RakNet\\LinuxStrings.o

$(OBJDIR_RELEASE)\\RakNet\\Itoa.o: RakNet\\Itoa.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\Itoa.cpp -o $(OBJDIR_RELEASE)\\RakNet\\Itoa.o

$(OBJDIR_RELEASE)\\RakNet\\IncrementalReadInterface.o: RakNet\\IncrementalReadInterface.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\IncrementalReadInterface.cpp -o $(OBJDIR_RELEASE)\\RakNet\\IncrementalReadInterface.o

$(OBJDIR_RELEASE)\\RakNet\\HTTPConnection.o: RakNet\\HTTPConnection.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\HTTPConnection.cpp -o $(OBJDIR_RELEASE)\\RakNet\\HTTPConnection.o

$(OBJDIR_RELEASE)\\RakNet\\GridSectorizer.o: RakNet\\GridSectorizer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\GridSectorizer.cpp -o $(OBJDIR_RELEASE)\\RakNet\\GridSectorizer.o

$(OBJDIR_RELEASE)\\RakNet\\Gets.o: RakNet\\Gets.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\Gets.cpp -o $(OBJDIR_RELEASE)\\RakNet\\Gets.o

$(OBJDIR_RELEASE)\\RakNet\\Getche.o: RakNet\\Getche.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\Getche.cpp -o $(OBJDIR_RELEASE)\\RakNet\\Getche.o

$(OBJDIR_RELEASE)\\RakNet\\NetworkIDManager.o: RakNet\\NetworkIDManager.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\NetworkIDManager.cpp -o $(OBJDIR_RELEASE)\\RakNet\\NetworkIDManager.o

$(OBJDIR_RELEASE)\\RakNet\\NatTypeDetectionServer.o: RakNet\\NatTypeDetectionServer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\NatTypeDetectionServer.cpp -o $(OBJDIR_RELEASE)\\RakNet\\NatTypeDetectionServer.o

$(OBJDIR_RELEASE)\\RakNet\\NatTypeDetectionCommon.o: RakNet\\NatTypeDetectionCommon.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\NatTypeDetectionCommon.cpp -o $(OBJDIR_RELEASE)\\RakNet\\NatTypeDetectionCommon.o

$(OBJDIR_RELEASE)\\RakNet\\NatTypeDetectionClient.o: RakNet\\NatTypeDetectionClient.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\NatTypeDetectionClient.cpp -o $(OBJDIR_RELEASE)\\RakNet\\NatTypeDetectionClient.o

$(OBJDIR_RELEASE)\\RakNet\\NatPunchthroughServer.o: RakNet\\NatPunchthroughServer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\NatPunchthroughServer.cpp -o $(OBJDIR_RELEASE)\\RakNet\\NatPunchthroughServer.o

$(OBJDIR_RELEASE)\\RakNet\\NatPunchthroughClient.o: RakNet\\NatPunchthroughClient.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\NatPunchthroughClient.cpp -o $(OBJDIR_RELEASE)\\RakNet\\NatPunchthroughClient.o

$(OBJDIR_RELEASE)\\RakNet\\MessageFilter.o: RakNet\\MessageFilter.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\MessageFilter.cpp -o $(OBJDIR_RELEASE)\\RakNet\\MessageFilter.o

$(OBJDIR_RELEASE)\\RakNet\\EmailSender.o: RakNet\\EmailSender.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\EmailSender.cpp -o $(OBJDIR_RELEASE)\\RakNet\\EmailSender.o

$(OBJDIR_RELEASE)\\RakNet\\DynDNS.o: RakNet\\DynDNS.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\DynDNS.cpp -o $(OBJDIR_RELEASE)\\RakNet\\DynDNS.o

$(OBJDIR_RELEASE)\\RakNet\\DirectoryDeltaTransfer.o: RakNet\\DirectoryDeltaTransfer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\DirectoryDeltaTransfer.cpp -o $(OBJDIR_RELEASE)\\RakNet\\DirectoryDeltaTransfer.o

$(OBJDIR_RELEASE)\\RakNet\\DataCompressor.o: RakNet\\DataCompressor.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\DataCompressor.cpp -o $(OBJDIR_RELEASE)\\RakNet\\DataCompressor.o

$(OBJDIR_RELEASE)\\RakNet\\DS_Table.o: RakNet\\DS_Table.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\DS_Table.cpp -o $(OBJDIR_RELEASE)\\RakNet\\DS_Table.o

$(OBJDIR_RELEASE)\\RakNet\\GetTime.o: RakNet\\GetTime.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\GetTime.cpp -o $(OBJDIR_RELEASE)\\RakNet\\GetTime.o

$(OBJDIR_RELEASE)\\RakNet\\FullyConnectedMesh2.o: RakNet\\FullyConnectedMesh2.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\FullyConnectedMesh2.cpp -o $(OBJDIR_RELEASE)\\RakNet\\FullyConnectedMesh2.o

$(OBJDIR_RELEASE)\\RakNet\\FormatString.o: RakNet\\FormatString.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\FormatString.cpp -o $(OBJDIR_RELEASE)\\RakNet\\FormatString.o

$(OBJDIR_RELEASE)\\RakNet\\FileOperations.o: RakNet\\FileOperations.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\FileOperations.cpp -o $(OBJDIR_RELEASE)\\RakNet\\FileOperations.o

$(OBJDIR_RELEASE)\\RakNet\\FileListTransfer.o: RakNet\\FileListTransfer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\FileListTransfer.cpp -o $(OBJDIR_RELEASE)\\RakNet\\FileListTransfer.o

$(OBJDIR_RELEASE)\\RakNet\\FileList.o: RakNet\\FileList.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\FileList.cpp -o $(OBJDIR_RELEASE)\\RakNet\\FileList.o

$(OBJDIR_RELEASE)\\RakNet\\EpochTimeToString.o: RakNet\\EpochTimeToString.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\EpochTimeToString.cpp -o $(OBJDIR_RELEASE)\\RakNet\\EpochTimeToString.o

$(OBJDIR_RELEASE)\\RakNet\\EncodeClassName.o: RakNet\\EncodeClassName.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNet\\EncodeClassName.cpp -o $(OBJDIR_RELEASE)\\RakNet\\EncodeClassName.o

clean_release:
	cmd /c del /f $(OBJ_RELEASE) $(OUT_RELEASE)
	cmd /c rd $(OBJDIR_RELEASE)\\RakNet
	cmd /c rd $(OBJDIR_RELEASE)
	cmd /c rd $(OBJDIR_RELEASE)\\iniparser\\src

.PHONY: before_debug after_debug clean_debug before_release after_release clean_release

