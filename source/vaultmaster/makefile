WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC =  -I..
CFLAGS =  -std=gnu++0x
RESINC = 
LIBDIR = 
LIB = 
LDFLAGS = -lpthread

INC_UNIX =  $(INC)
CFLAGS_UNIX =  $(CFLAGS) -O2 -Wredundant-decls -Wunreachable-code
RESINC_UNIX =  $(RESINC)
RCFLAGS_UNIX =  $(RCFLAGS)
LIBDIR_UNIX =  $(LIBDIR)
LIB_UNIX = $(LIB)
LDFLAGS_UNIX =  $(LDFLAGS) -s
OBJDIR_UNIX = obj
DEP_UNIX = 
OUT_UNIX = vaultmaster

OBJ_UNIX = $(OBJDIR_UNIX)/../RakNet/RakPeer.o $(OBJDIR_UNIX)/../RakNet/RakSleep.o $(OBJDIR_UNIX)/../RakNet/RakString.o $(OBJDIR_UNIX)/../RakNet/RakThread.o $(OBJDIR_UNIX)/../RakNet/RakWString.o $(OBJDIR_UNIX)/../RakNet/Rand.o $(OBJDIR_UNIX)/../RakNet/ReadyEvent.o $(OBJDIR_UNIX)/../RakNet/ReliabilityLayer.o $(OBJDIR_UNIX)/../RakNet/ReplicaManager3.o $(OBJDIR_UNIX)/../RakNet/Router2.o $(OBJDIR_UNIX)/../RakNet/SHA1.o $(OBJDIR_UNIX)/../RakNet/SecureHandshake.o $(OBJDIR_UNIX)/../RakNet/SendToThread.o $(OBJDIR_UNIX)/../RakNet/PacketOutputWindowLogger.o $(OBJDIR_UNIX)/../RakNet/PacketizedTCP.o $(OBJDIR_UNIX)/../RakNet/PluginInterface2.o $(OBJDIR_UNIX)/../RakNet/RPC4Plugin.o $(OBJDIR_UNIX)/../RakNet/Rackspace.o $(OBJDIR_UNIX)/../RakNet/RakMemoryOverride.o $(OBJDIR_UNIX)/../RakNet/RakNetCommandParser.o $(OBJDIR_UNIX)/../RakNet/RakNetSocket.o $(OBJDIR_UNIX)/../RakNet/RakNetSocket2.o $(OBJDIR_UNIX)/../RakNet/RakNetStatistics.o $(OBJDIR_UNIX)/../RakNet/RakNetTransport2.o $(OBJDIR_UNIX)/../RakNet/RakNetTypes.o $(OBJDIR_UNIX)/../RakNet/UDPProxyCoordinator.o $(OBJDIR_UNIX)/../RakNet/UDPProxyServer.o $(OBJDIR_UNIX)/../RakNet/VariableDeltaSerializer.o $(OBJDIR_UNIX)/../RakNet/VariableListDeltaTracker.o $(OBJDIR_UNIX)/../RakNet/VariadicSQLParser.o $(OBJDIR_UNIX)/../RakNet/WSAStartupSingleton.o $(OBJDIR_UNIX)/../RakNet/_FindFirst.o $(OBJDIR_UNIX)/../RakNet/gettimeofday.o $(OBJDIR_UNIX)/../RakNet/rdlmalloc.o $(OBJDIR_UNIX)/../ServerEntry.o $(OBJDIR_UNIX)/../Utils.o $(OBJDIR_UNIX)/MasterServer.o $(OBJDIR_UNIX)/vaultmaster.o $(OBJDIR_UNIX)/../RakNet/SignaledEvent.o $(OBJDIR_UNIX)/../RakNet/SimpleMutex.o $(OBJDIR_UNIX)/../RakNet/SocketLayer.o $(OBJDIR_UNIX)/../RakNet/StringCompressor.o $(OBJDIR_UNIX)/../RakNet/StringTable.o $(OBJDIR_UNIX)/../RakNet/SuperFastHash.o $(OBJDIR_UNIX)/../RakNet/TCPInterface.o $(OBJDIR_UNIX)/../RakNet/TableSerializer.o $(OBJDIR_UNIX)/../RakNet/TeamBalancer.o $(OBJDIR_UNIX)/../RakNet/TelnetTransport.o $(OBJDIR_UNIX)/../RakNet/ThreadsafePacketLogger.o $(OBJDIR_UNIX)/../RakNet/TwoWayAuthentication.o $(OBJDIR_UNIX)/../RakNet/UDPForwarder.o $(OBJDIR_UNIX)/../RakNet/UDPProxyClient.o $(OBJDIR_UNIX)/../RakNet/DS_HuffmanEncodingTree.o $(OBJDIR_UNIX)/../RakNet/DS_Table.o $(OBJDIR_UNIX)/../RakNet/DataCompressor.o $(OBJDIR_UNIX)/../RakNet/DirectoryDeltaTransfer.o $(OBJDIR_UNIX)/../RakNet/DynDNS.o $(OBJDIR_UNIX)/../RakNet/EmailSender.o $(OBJDIR_UNIX)/../RakNet/EncodeClassName.o $(OBJDIR_UNIX)/../RakNet/EpochTimeToString.o $(OBJDIR_UNIX)/../RakNet/Base64Encoder.o $(OBJDIR_UNIX)/../RakNet/BitStream.o $(OBJDIR_UNIX)/../RakNet/CCRakNetSlidingWindow.o $(OBJDIR_UNIX)/../RakNet/CCRakNetUDT.o $(OBJDIR_UNIX)/../RakNet/CheckSum.o $(OBJDIR_UNIX)/../RakNet/CloudClient.o $(OBJDIR_UNIX)/../RakNet/CloudCommon.o $(OBJDIR_UNIX)/../RakNet/CloudServer.o $(OBJDIR_UNIX)/../RakNet/CommandParserInterface.o $(OBJDIR_UNIX)/../RakNet/ConnectionGraph2.o $(OBJDIR_UNIX)/../RakNet/ConsoleServer.o $(OBJDIR_UNIX)/../RakNet/DS_BytePool.o $(OBJDIR_UNIX)/../RakNet/DS_ByteQueue.o $(OBJDIR_UNIX)/../RakNet/LocklessTypes.o $(OBJDIR_UNIX)/../RakNet/LogCommandParser.o $(OBJDIR_UNIX)/../RakNet/MessageFilter.o $(OBJDIR_UNIX)/../RakNet/NatPunchthroughClient.o $(OBJDIR_UNIX)/../RakNet/NatPunchthroughServer.o $(OBJDIR_UNIX)/../RakNet/NatTypeDetectionClient.o $(OBJDIR_UNIX)/../RakNet/NatTypeDetectionCommon.o $(OBJDIR_UNIX)/../RakNet/NatTypeDetectionServer.o $(OBJDIR_UNIX)/../RakNet/NetworkIDManager.o $(OBJDIR_UNIX)/../RakNet/NetworkIDObject.o $(OBJDIR_UNIX)/../RakNet/PacketConsoleLogger.o $(OBJDIR_UNIX)/../RakNet/PacketFileLogger.o $(OBJDIR_UNIX)/../RakNet/PacketLogger.o $(OBJDIR_UNIX)/../RakNet/FileList.o $(OBJDIR_UNIX)/../RakNet/FileListTransfer.o $(OBJDIR_UNIX)/../RakNet/FileOperations.o $(OBJDIR_UNIX)/../RakNet/FormatString.o $(OBJDIR_UNIX)/../RakNet/FullyConnectedMesh2.o $(OBJDIR_UNIX)/../RakNet/GetTime.o $(OBJDIR_UNIX)/../RakNet/Getche.o $(OBJDIR_UNIX)/../RakNet/Gets.o $(OBJDIR_UNIX)/../RakNet/GridSectorizer.o $(OBJDIR_UNIX)/../RakNet/HTTPConnection.o $(OBJDIR_UNIX)/../RakNet/IncrementalReadInterface.o $(OBJDIR_UNIX)/../RakNet/Itoa.o $(OBJDIR_UNIX)/../RakNet/LinuxStrings.o

all: unix

clean: clean_unix

before_unix: 
	test -d $(OBJDIR_UNIX)/../RakNet || mkdir -p $(OBJDIR_UNIX)/../RakNet
	test -d $(OBJDIR_UNIX)/.. || mkdir -p $(OBJDIR_UNIX)/..
	test -d $(OBJDIR_UNIX) || mkdir -p $(OBJDIR_UNIX)

after_unix: 

unix: before_unix out_unix after_unix

out_unix: $(OBJ_UNIX) $(DEP_UNIX)
	$(LD) $(LDFLAGS_UNIX) $(LIBDIR_UNIX) $(OBJ_UNIX) $(LIB_UNIX) -o $(OUT_UNIX)

$(OBJDIR_UNIX)/../RakNet/RakPeer.o: ../RakNet/RakPeer.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/RakPeer.cpp -o $(OBJDIR_UNIX)/../RakNet/RakPeer.o

$(OBJDIR_UNIX)/../RakNet/RakSleep.o: ../RakNet/RakSleep.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/RakSleep.cpp -o $(OBJDIR_UNIX)/../RakNet/RakSleep.o

$(OBJDIR_UNIX)/../RakNet/RakString.o: ../RakNet/RakString.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/RakString.cpp -o $(OBJDIR_UNIX)/../RakNet/RakString.o

$(OBJDIR_UNIX)/../RakNet/RakThread.o: ../RakNet/RakThread.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/RakThread.cpp -o $(OBJDIR_UNIX)/../RakNet/RakThread.o

$(OBJDIR_UNIX)/../RakNet/RakWString.o: ../RakNet/RakWString.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/RakWString.cpp -o $(OBJDIR_UNIX)/../RakNet/RakWString.o

$(OBJDIR_UNIX)/../RakNet/Rand.o: ../RakNet/Rand.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/Rand.cpp -o $(OBJDIR_UNIX)/../RakNet/Rand.o

$(OBJDIR_UNIX)/../RakNet/ReadyEvent.o: ../RakNet/ReadyEvent.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/ReadyEvent.cpp -o $(OBJDIR_UNIX)/../RakNet/ReadyEvent.o

$(OBJDIR_UNIX)/../RakNet/ReliabilityLayer.o: ../RakNet/ReliabilityLayer.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/ReliabilityLayer.cpp -o $(OBJDIR_UNIX)/../RakNet/ReliabilityLayer.o

$(OBJDIR_UNIX)/../RakNet/ReplicaManager3.o: ../RakNet/ReplicaManager3.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/ReplicaManager3.cpp -o $(OBJDIR_UNIX)/../RakNet/ReplicaManager3.o

$(OBJDIR_UNIX)/../RakNet/Router2.o: ../RakNet/Router2.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/Router2.cpp -o $(OBJDIR_UNIX)/../RakNet/Router2.o

$(OBJDIR_UNIX)/../RakNet/SHA1.o: ../RakNet/SHA1.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/SHA1.cpp -o $(OBJDIR_UNIX)/../RakNet/SHA1.o

$(OBJDIR_UNIX)/../RakNet/SecureHandshake.o: ../RakNet/SecureHandshake.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/SecureHandshake.cpp -o $(OBJDIR_UNIX)/../RakNet/SecureHandshake.o

$(OBJDIR_UNIX)/../RakNet/SendToThread.o: ../RakNet/SendToThread.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/SendToThread.cpp -o $(OBJDIR_UNIX)/../RakNet/SendToThread.o

$(OBJDIR_UNIX)/../RakNet/PacketOutputWindowLogger.o: ../RakNet/PacketOutputWindowLogger.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/PacketOutputWindowLogger.cpp -o $(OBJDIR_UNIX)/../RakNet/PacketOutputWindowLogger.o

$(OBJDIR_UNIX)/../RakNet/PacketizedTCP.o: ../RakNet/PacketizedTCP.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/PacketizedTCP.cpp -o $(OBJDIR_UNIX)/../RakNet/PacketizedTCP.o

$(OBJDIR_UNIX)/../RakNet/PluginInterface2.o: ../RakNet/PluginInterface2.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/PluginInterface2.cpp -o $(OBJDIR_UNIX)/../RakNet/PluginInterface2.o

$(OBJDIR_UNIX)/../RakNet/RPC4Plugin.o: ../RakNet/RPC4Plugin.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/RPC4Plugin.cpp -o $(OBJDIR_UNIX)/../RakNet/RPC4Plugin.o

$(OBJDIR_UNIX)/../RakNet/Rackspace.o: ../RakNet/Rackspace.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/Rackspace.cpp -o $(OBJDIR_UNIX)/../RakNet/Rackspace.o

$(OBJDIR_UNIX)/../RakNet/RakMemoryOverride.o: ../RakNet/RakMemoryOverride.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/RakMemoryOverride.cpp -o $(OBJDIR_UNIX)/../RakNet/RakMemoryOverride.o

$(OBJDIR_UNIX)/../RakNet/RakNetCommandParser.o: ../RakNet/RakNetCommandParser.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/RakNetCommandParser.cpp -o $(OBJDIR_UNIX)/../RakNet/RakNetCommandParser.o

$(OBJDIR_UNIX)/../RakNet/RakNetSocket.o: ../RakNet/RakNetSocket.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/RakNetSocket.cpp -o $(OBJDIR_UNIX)/../RakNet/RakNetSocket.o

$(OBJDIR_UNIX)/../RakNet/RakNetSocket2.o: ../RakNet/RakNetSocket2.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/RakNetSocket2.cpp -o $(OBJDIR_UNIX)/../RakNet/RakNetSocket2.o

$(OBJDIR_UNIX)/../RakNet/RakNetStatistics.o: ../RakNet/RakNetStatistics.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/RakNetStatistics.cpp -o $(OBJDIR_UNIX)/../RakNet/RakNetStatistics.o

$(OBJDIR_UNIX)/../RakNet/RakNetTransport2.o: ../RakNet/RakNetTransport2.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/RakNetTransport2.cpp -o $(OBJDIR_UNIX)/../RakNet/RakNetTransport2.o

$(OBJDIR_UNIX)/../RakNet/RakNetTypes.o: ../RakNet/RakNetTypes.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/RakNetTypes.cpp -o $(OBJDIR_UNIX)/../RakNet/RakNetTypes.o

$(OBJDIR_UNIX)/../RakNet/UDPProxyCoordinator.o: ../RakNet/UDPProxyCoordinator.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/UDPProxyCoordinator.cpp -o $(OBJDIR_UNIX)/../RakNet/UDPProxyCoordinator.o

$(OBJDIR_UNIX)/../RakNet/UDPProxyServer.o: ../RakNet/UDPProxyServer.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/UDPProxyServer.cpp -o $(OBJDIR_UNIX)/../RakNet/UDPProxyServer.o

$(OBJDIR_UNIX)/../RakNet/VariableDeltaSerializer.o: ../RakNet/VariableDeltaSerializer.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/VariableDeltaSerializer.cpp -o $(OBJDIR_UNIX)/../RakNet/VariableDeltaSerializer.o

$(OBJDIR_UNIX)/../RakNet/VariableListDeltaTracker.o: ../RakNet/VariableListDeltaTracker.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/VariableListDeltaTracker.cpp -o $(OBJDIR_UNIX)/../RakNet/VariableListDeltaTracker.o

$(OBJDIR_UNIX)/../RakNet/VariadicSQLParser.o: ../RakNet/VariadicSQLParser.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/VariadicSQLParser.cpp -o $(OBJDIR_UNIX)/../RakNet/VariadicSQLParser.o

$(OBJDIR_UNIX)/../RakNet/WSAStartupSingleton.o: ../RakNet/WSAStartupSingleton.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/WSAStartupSingleton.cpp -o $(OBJDIR_UNIX)/../RakNet/WSAStartupSingleton.o

$(OBJDIR_UNIX)/../RakNet/_FindFirst.o: ../RakNet/_FindFirst.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/_FindFirst.cpp -o $(OBJDIR_UNIX)/../RakNet/_FindFirst.o

$(OBJDIR_UNIX)/../RakNet/gettimeofday.o: ../RakNet/gettimeofday.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/gettimeofday.cpp -o $(OBJDIR_UNIX)/../RakNet/gettimeofday.o

$(OBJDIR_UNIX)/../RakNet/rdlmalloc.o: ../RakNet/rdlmalloc.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/rdlmalloc.cpp -o $(OBJDIR_UNIX)/../RakNet/rdlmalloc.o

$(OBJDIR_UNIX)/../ServerEntry.o: ../ServerEntry.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../ServerEntry.cpp -o $(OBJDIR_UNIX)/../ServerEntry.o

$(OBJDIR_UNIX)/../Utils.o: ../Utils.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../Utils.cpp -o $(OBJDIR_UNIX)/../Utils.o

$(OBJDIR_UNIX)/MasterServer.o: MasterServer.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c MasterServer.cpp -o $(OBJDIR_UNIX)/MasterServer.o

$(OBJDIR_UNIX)/vaultmaster.o: vaultmaster.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c vaultmaster.cpp -o $(OBJDIR_UNIX)/vaultmaster.o

$(OBJDIR_UNIX)/../RakNet/SignaledEvent.o: ../RakNet/SignaledEvent.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/SignaledEvent.cpp -o $(OBJDIR_UNIX)/../RakNet/SignaledEvent.o

$(OBJDIR_UNIX)/../RakNet/SimpleMutex.o: ../RakNet/SimpleMutex.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/SimpleMutex.cpp -o $(OBJDIR_UNIX)/../RakNet/SimpleMutex.o

$(OBJDIR_UNIX)/../RakNet/SocketLayer.o: ../RakNet/SocketLayer.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/SocketLayer.cpp -o $(OBJDIR_UNIX)/../RakNet/SocketLayer.o

$(OBJDIR_UNIX)/../RakNet/StringCompressor.o: ../RakNet/StringCompressor.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/StringCompressor.cpp -o $(OBJDIR_UNIX)/../RakNet/StringCompressor.o

$(OBJDIR_UNIX)/../RakNet/StringTable.o: ../RakNet/StringTable.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/StringTable.cpp -o $(OBJDIR_UNIX)/../RakNet/StringTable.o

$(OBJDIR_UNIX)/../RakNet/SuperFastHash.o: ../RakNet/SuperFastHash.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/SuperFastHash.cpp -o $(OBJDIR_UNIX)/../RakNet/SuperFastHash.o

$(OBJDIR_UNIX)/../RakNet/TCPInterface.o: ../RakNet/TCPInterface.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/TCPInterface.cpp -o $(OBJDIR_UNIX)/../RakNet/TCPInterface.o

$(OBJDIR_UNIX)/../RakNet/TableSerializer.o: ../RakNet/TableSerializer.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/TableSerializer.cpp -o $(OBJDIR_UNIX)/../RakNet/TableSerializer.o

$(OBJDIR_UNIX)/../RakNet/TeamBalancer.o: ../RakNet/TeamBalancer.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/TeamBalancer.cpp -o $(OBJDIR_UNIX)/../RakNet/TeamBalancer.o

$(OBJDIR_UNIX)/../RakNet/TelnetTransport.o: ../RakNet/TelnetTransport.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/TelnetTransport.cpp -o $(OBJDIR_UNIX)/../RakNet/TelnetTransport.o

$(OBJDIR_UNIX)/../RakNet/ThreadsafePacketLogger.o: ../RakNet/ThreadsafePacketLogger.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/ThreadsafePacketLogger.cpp -o $(OBJDIR_UNIX)/../RakNet/ThreadsafePacketLogger.o

$(OBJDIR_UNIX)/../RakNet/TwoWayAuthentication.o: ../RakNet/TwoWayAuthentication.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/TwoWayAuthentication.cpp -o $(OBJDIR_UNIX)/../RakNet/TwoWayAuthentication.o

$(OBJDIR_UNIX)/../RakNet/UDPForwarder.o: ../RakNet/UDPForwarder.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/UDPForwarder.cpp -o $(OBJDIR_UNIX)/../RakNet/UDPForwarder.o

$(OBJDIR_UNIX)/../RakNet/UDPProxyClient.o: ../RakNet/UDPProxyClient.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/UDPProxyClient.cpp -o $(OBJDIR_UNIX)/../RakNet/UDPProxyClient.o

$(OBJDIR_UNIX)/../RakNet/DS_HuffmanEncodingTree.o: ../RakNet/DS_HuffmanEncodingTree.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/DS_HuffmanEncodingTree.cpp -o $(OBJDIR_UNIX)/../RakNet/DS_HuffmanEncodingTree.o

$(OBJDIR_UNIX)/../RakNet/DS_Table.o: ../RakNet/DS_Table.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/DS_Table.cpp -o $(OBJDIR_UNIX)/../RakNet/DS_Table.o

$(OBJDIR_UNIX)/../RakNet/DataCompressor.o: ../RakNet/DataCompressor.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/DataCompressor.cpp -o $(OBJDIR_UNIX)/../RakNet/DataCompressor.o

$(OBJDIR_UNIX)/../RakNet/DirectoryDeltaTransfer.o: ../RakNet/DirectoryDeltaTransfer.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/DirectoryDeltaTransfer.cpp -o $(OBJDIR_UNIX)/../RakNet/DirectoryDeltaTransfer.o

$(OBJDIR_UNIX)/../RakNet/DynDNS.o: ../RakNet/DynDNS.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/DynDNS.cpp -o $(OBJDIR_UNIX)/../RakNet/DynDNS.o

$(OBJDIR_UNIX)/../RakNet/EmailSender.o: ../RakNet/EmailSender.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/EmailSender.cpp -o $(OBJDIR_UNIX)/../RakNet/EmailSender.o

$(OBJDIR_UNIX)/../RakNet/EncodeClassName.o: ../RakNet/EncodeClassName.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/EncodeClassName.cpp -o $(OBJDIR_UNIX)/../RakNet/EncodeClassName.o

$(OBJDIR_UNIX)/../RakNet/EpochTimeToString.o: ../RakNet/EpochTimeToString.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/EpochTimeToString.cpp -o $(OBJDIR_UNIX)/../RakNet/EpochTimeToString.o

$(OBJDIR_UNIX)/../RakNet/Base64Encoder.o: ../RakNet/Base64Encoder.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/Base64Encoder.cpp -o $(OBJDIR_UNIX)/../RakNet/Base64Encoder.o

$(OBJDIR_UNIX)/../RakNet/BitStream.o: ../RakNet/BitStream.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/BitStream.cpp -o $(OBJDIR_UNIX)/../RakNet/BitStream.o

$(OBJDIR_UNIX)/../RakNet/CCRakNetSlidingWindow.o: ../RakNet/CCRakNetSlidingWindow.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/CCRakNetSlidingWindow.cpp -o $(OBJDIR_UNIX)/../RakNet/CCRakNetSlidingWindow.o

$(OBJDIR_UNIX)/../RakNet/CCRakNetUDT.o: ../RakNet/CCRakNetUDT.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/CCRakNetUDT.cpp -o $(OBJDIR_UNIX)/../RakNet/CCRakNetUDT.o

$(OBJDIR_UNIX)/../RakNet/CheckSum.o: ../RakNet/CheckSum.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/CheckSum.cpp -o $(OBJDIR_UNIX)/../RakNet/CheckSum.o

$(OBJDIR_UNIX)/../RakNet/CloudClient.o: ../RakNet/CloudClient.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/CloudClient.cpp -o $(OBJDIR_UNIX)/../RakNet/CloudClient.o

$(OBJDIR_UNIX)/../RakNet/CloudCommon.o: ../RakNet/CloudCommon.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/CloudCommon.cpp -o $(OBJDIR_UNIX)/../RakNet/CloudCommon.o

$(OBJDIR_UNIX)/../RakNet/CloudServer.o: ../RakNet/CloudServer.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/CloudServer.cpp -o $(OBJDIR_UNIX)/../RakNet/CloudServer.o

$(OBJDIR_UNIX)/../RakNet/CommandParserInterface.o: ../RakNet/CommandParserInterface.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/CommandParserInterface.cpp -o $(OBJDIR_UNIX)/../RakNet/CommandParserInterface.o

$(OBJDIR_UNIX)/../RakNet/ConnectionGraph2.o: ../RakNet/ConnectionGraph2.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/ConnectionGraph2.cpp -o $(OBJDIR_UNIX)/../RakNet/ConnectionGraph2.o

$(OBJDIR_UNIX)/../RakNet/ConsoleServer.o: ../RakNet/ConsoleServer.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/ConsoleServer.cpp -o $(OBJDIR_UNIX)/../RakNet/ConsoleServer.o

$(OBJDIR_UNIX)/../RakNet/DS_BytePool.o: ../RakNet/DS_BytePool.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/DS_BytePool.cpp -o $(OBJDIR_UNIX)/../RakNet/DS_BytePool.o

$(OBJDIR_UNIX)/../RakNet/DS_ByteQueue.o: ../RakNet/DS_ByteQueue.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/DS_ByteQueue.cpp -o $(OBJDIR_UNIX)/../RakNet/DS_ByteQueue.o

$(OBJDIR_UNIX)/../RakNet/LocklessTypes.o: ../RakNet/LocklessTypes.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/LocklessTypes.cpp -o $(OBJDIR_UNIX)/../RakNet/LocklessTypes.o

$(OBJDIR_UNIX)/../RakNet/LogCommandParser.o: ../RakNet/LogCommandParser.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/LogCommandParser.cpp -o $(OBJDIR_UNIX)/../RakNet/LogCommandParser.o

$(OBJDIR_UNIX)/../RakNet/MessageFilter.o: ../RakNet/MessageFilter.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/MessageFilter.cpp -o $(OBJDIR_UNIX)/../RakNet/MessageFilter.o

$(OBJDIR_UNIX)/../RakNet/NatPunchthroughClient.o: ../RakNet/NatPunchthroughClient.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/NatPunchthroughClient.cpp -o $(OBJDIR_UNIX)/../RakNet/NatPunchthroughClient.o

$(OBJDIR_UNIX)/../RakNet/NatPunchthroughServer.o: ../RakNet/NatPunchthroughServer.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/NatPunchthroughServer.cpp -o $(OBJDIR_UNIX)/../RakNet/NatPunchthroughServer.o

$(OBJDIR_UNIX)/../RakNet/NatTypeDetectionClient.o: ../RakNet/NatTypeDetectionClient.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/NatTypeDetectionClient.cpp -o $(OBJDIR_UNIX)/../RakNet/NatTypeDetectionClient.o

$(OBJDIR_UNIX)/../RakNet/NatTypeDetectionCommon.o: ../RakNet/NatTypeDetectionCommon.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/NatTypeDetectionCommon.cpp -o $(OBJDIR_UNIX)/../RakNet/NatTypeDetectionCommon.o

$(OBJDIR_UNIX)/../RakNet/NatTypeDetectionServer.o: ../RakNet/NatTypeDetectionServer.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/NatTypeDetectionServer.cpp -o $(OBJDIR_UNIX)/../RakNet/NatTypeDetectionServer.o

$(OBJDIR_UNIX)/../RakNet/NetworkIDManager.o: ../RakNet/NetworkIDManager.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/NetworkIDManager.cpp -o $(OBJDIR_UNIX)/../RakNet/NetworkIDManager.o

$(OBJDIR_UNIX)/../RakNet/NetworkIDObject.o: ../RakNet/NetworkIDObject.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/NetworkIDObject.cpp -o $(OBJDIR_UNIX)/../RakNet/NetworkIDObject.o

$(OBJDIR_UNIX)/../RakNet/PacketConsoleLogger.o: ../RakNet/PacketConsoleLogger.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/PacketConsoleLogger.cpp -o $(OBJDIR_UNIX)/../RakNet/PacketConsoleLogger.o

$(OBJDIR_UNIX)/../RakNet/PacketFileLogger.o: ../RakNet/PacketFileLogger.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/PacketFileLogger.cpp -o $(OBJDIR_UNIX)/../RakNet/PacketFileLogger.o

$(OBJDIR_UNIX)/../RakNet/PacketLogger.o: ../RakNet/PacketLogger.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/PacketLogger.cpp -o $(OBJDIR_UNIX)/../RakNet/PacketLogger.o

$(OBJDIR_UNIX)/../RakNet/FileList.o: ../RakNet/FileList.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/FileList.cpp -o $(OBJDIR_UNIX)/../RakNet/FileList.o

$(OBJDIR_UNIX)/../RakNet/FileListTransfer.o: ../RakNet/FileListTransfer.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/FileListTransfer.cpp -o $(OBJDIR_UNIX)/../RakNet/FileListTransfer.o

$(OBJDIR_UNIX)/../RakNet/FileOperations.o: ../RakNet/FileOperations.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/FileOperations.cpp -o $(OBJDIR_UNIX)/../RakNet/FileOperations.o

$(OBJDIR_UNIX)/../RakNet/FormatString.o: ../RakNet/FormatString.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/FormatString.cpp -o $(OBJDIR_UNIX)/../RakNet/FormatString.o

$(OBJDIR_UNIX)/../RakNet/FullyConnectedMesh2.o: ../RakNet/FullyConnectedMesh2.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/FullyConnectedMesh2.cpp -o $(OBJDIR_UNIX)/../RakNet/FullyConnectedMesh2.o

$(OBJDIR_UNIX)/../RakNet/GetTime.o: ../RakNet/GetTime.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/GetTime.cpp -o $(OBJDIR_UNIX)/../RakNet/GetTime.o

$(OBJDIR_UNIX)/../RakNet/Getche.o: ../RakNet/Getche.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/Getche.cpp -o $(OBJDIR_UNIX)/../RakNet/Getche.o

$(OBJDIR_UNIX)/../RakNet/Gets.o: ../RakNet/Gets.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/Gets.cpp -o $(OBJDIR_UNIX)/../RakNet/Gets.o

$(OBJDIR_UNIX)/../RakNet/GridSectorizer.o: ../RakNet/GridSectorizer.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/GridSectorizer.cpp -o $(OBJDIR_UNIX)/../RakNet/GridSectorizer.o

$(OBJDIR_UNIX)/../RakNet/HTTPConnection.o: ../RakNet/HTTPConnection.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/HTTPConnection.cpp -o $(OBJDIR_UNIX)/../RakNet/HTTPConnection.o

$(OBJDIR_UNIX)/../RakNet/IncrementalReadInterface.o: ../RakNet/IncrementalReadInterface.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/IncrementalReadInterface.cpp -o $(OBJDIR_UNIX)/../RakNet/IncrementalReadInterface.o

$(OBJDIR_UNIX)/../RakNet/Itoa.o: ../RakNet/Itoa.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/Itoa.cpp -o $(OBJDIR_UNIX)/../RakNet/Itoa.o

$(OBJDIR_UNIX)/../RakNet/LinuxStrings.o: ../RakNet/LinuxStrings.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c ../RakNet/LinuxStrings.cpp -o $(OBJDIR_UNIX)/../RakNet/LinuxStrings.o

clean_unix: 
	rm -f $(OBJ_UNIX) $(OUT_UNIX)
	rm -rf $(OBJDIR_UNIX)/../RakNet
	rm -rf $(OBJDIR_UNIX)/..
	rm -rf $(OBJDIR_UNIX)

.PHONY: before_unix after_unix clean_unix

