WORKDIR = %cd%

CC = gcc.exe
CXX = g++.exe
AR = ar.exe
LD = g++.exe
WINDRES = windres.exe

INC = -I.
CFLAGS = -pedantic-errors -pedantic -Wfatal-errors -Wextra -Wall -std=gnu++1y
CFLAGSEXT =
RESINC =
RCFLAGS = -F pe-i386
LIBDIR =
LIB =
LDFLAGS = -m32 -lshlwapi -lcomctl32 -lwinmm -lws2_32 -lz -limagehlp -static -mwindows

INC_DEBUG = $(INC)
CFLAGS_DEBUG = $(CFLAGS) -gstabs
CFLAGSEXT_DEBUG = $(CFLAGSEXT)
RESINC_DEBUG = $(RESINC)
RCFLAGS_DEBUG = $(RCFLAGS)
LIBDIR_DEBUG = $(LIBDIR)
LIB_DEBUG = $(LIB)
LDFLAGS_DEBUG = $(LDFLAGS)
OBJDIR_DEBUG = .objs\\Debug
DEP_DEBUG =
OUT_DEBUG = libRakNetd.a

INC_RELEASE = $(INC)
CFLAGS_RELEASE = $(CFLAGS) -O2 -DNDEBUG
CFLAGSEXT_RELEASE = $(CFLAGSEXT) -O2 -DNDEBUG
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE = -s $(LDFLAGS)
OBJDIR_RELEASE = .objs\\Release
DEP_RELEASE =
OUT_RELEASE = libRakNet.a

OBJ_DEBUG = \
$(OBJDIR_DEBUG)\\RakNetSocket2.o \
$(OBJDIR_DEBUG)\\Rand.o \
$(OBJDIR_DEBUG)\\RakThread.o \
$(OBJDIR_DEBUG)\\RakString.o \
$(OBJDIR_DEBUG)\\RakSleep.o \
$(OBJDIR_DEBUG)\\RakPeer.o \
$(OBJDIR_DEBUG)\\RakNetTypes.o \
$(OBJDIR_DEBUG)\\SimpleMutex.o \
$(OBJDIR_DEBUG)\\SignaledEvent.o \
$(OBJDIR_DEBUG)\\DR_SHA1.o \
$(OBJDIR_DEBUG)\\ReliabilityLayer.o \
$(OBJDIR_DEBUG)\\PluginInterface2.o \
$(OBJDIR_DEBUG)\\PacketizedTCP.o \
$(OBJDIR_DEBUG)\\NetworkIDObject.o \
$(OBJDIR_DEBUG)\\RakNetSocket.o \
$(OBJDIR_DEBUG)\\RakMemoryOverride.o \
$(OBJDIR_DEBUG)\\WSAStartupSingleton.o \
$(OBJDIR_DEBUG)\\TCPInterface.o \
$(OBJDIR_DEBUG)\\SuperFastHash.o \
$(OBJDIR_DEBUG)\\StringTable.o \
$(OBJDIR_DEBUG)\\StringCompressor.o \
$(OBJDIR_DEBUG)\\SocketLayer.o \
$(OBJDIR_DEBUG)\\VariableListDeltaTracker.o \
$(OBJDIR_DEBUG)\\CCRakNetSlidingWindow.o \
$(OBJDIR_DEBUG)\\BitStream.o \
$(OBJDIR_DEBUG)\\DS_HuffmanEncodingTree.o \
$(OBJDIR_DEBUG)\\DS_ByteQueue.o \
$(OBJDIR_DEBUG)\\LocklessTypes.o \
$(OBJDIR_DEBUG)\\Itoa.o \
$(OBJDIR_DEBUG)\\NetworkIDManager.o \
$(OBJDIR_DEBUG)\\GetTime.o \
$(OBJDIR_DEBUG)\\FileOperations.o \
$(OBJDIR_DEBUG)\\FileListTransfer.o \
$(OBJDIR_DEBUG)\\FileList.o \
$(OBJDIR_DEBUG)\\IncrementalReadInterface.o

OBJ_RELEASE = \
$(OBJDIR_RELEASE)\\RakNetSocket2.o \
$(OBJDIR_RELEASE)\\Rand.o \
$(OBJDIR_RELEASE)\\RakThread.o \
$(OBJDIR_RELEASE)\\RakString.o \
$(OBJDIR_RELEASE)\\RakSleep.o \
$(OBJDIR_RELEASE)\\RakPeer.o \
$(OBJDIR_RELEASE)\\RakNetTypes.o \
$(OBJDIR_RELEASE)\\SimpleMutex.o \
$(OBJDIR_RELEASE)\\SignaledEvent.o \
$(OBJDIR_RELEASE)\\DR_SHA1.o \
$(OBJDIR_RELEASE)\\ReliabilityLayer.o \
$(OBJDIR_RELEASE)\\PluginInterface2.o \
$(OBJDIR_RELEASE)\\PacketizedTCP.o \
$(OBJDIR_RELEASE)\\NetworkIDObject.o \
$(OBJDIR_RELEASE)\\RakNetSocket.o \
$(OBJDIR_RELEASE)\\RakMemoryOverride.o \
$(OBJDIR_RELEASE)\\WSAStartupSingleton.o \
$(OBJDIR_RELEASE)\\TCPInterface.o \
$(OBJDIR_RELEASE)\\SuperFastHash.o \
$(OBJDIR_RELEASE)\\StringTable.o \
$(OBJDIR_RELEASE)\\StringCompressor.o \
$(OBJDIR_RELEASE)\\SocketLayer.o \
$(OBJDIR_RELEASE)\\VariableListDeltaTracker.o \
$(OBJDIR_RELEASE)\\CCRakNetSlidingWindow.o \
$(OBJDIR_RELEASE)\\BitStream.o \
$(OBJDIR_RELEASE)\\DS_HuffmanEncodingTree.o \
$(OBJDIR_RELEASE)\\DS_ByteQueue.o \
$(OBJDIR_RELEASE)\\LocklessTypes.o \
$(OBJDIR_RELEASE)\\Itoa.o \
$(OBJDIR_RELEASE)\\NetworkIDManager.o \
$(OBJDIR_RELEASE)\\GetTime.o \
$(OBJDIR_RELEASE)\\FileOperations.o \
$(OBJDIR_RELEASE)\\FileListTransfer.o \
$(OBJDIR_RELEASE)\\FileList.o \
$(OBJDIR_RELEASE)\\IncrementalReadInterface.o

all: debug release

clean: clean_debug clean_release

before_debug:
	cmd /c if not exist $(OBJDIR_DEBUG) md $(OBJDIR_DEBUG)

after_debug:

debug: before_debug out_debug after_debug

out_debug: $(OBJ_DEBUG) $(DEP_DEBUG)
	$(AR) -rv $(OUT_DEBUG) $(OBJ_DEBUG)

$(OBJDIR_DEBUG)\\RakNetSocket2.o: RakNetSocket2.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNetSocket2.cpp -o $(OBJDIR_DEBUG)\\RakNetSocket2.o

$(OBJDIR_DEBUG)\\Rand.o: Rand.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c Rand.cpp -o $(OBJDIR_DEBUG)\\Rand.o

$(OBJDIR_DEBUG)\\RakThread.o: RakThread.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakThread.cpp -o $(OBJDIR_DEBUG)\\RakThread.o

$(OBJDIR_DEBUG)\\RakString.o: RakString.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakString.cpp -o $(OBJDIR_DEBUG)\\RakString.o

$(OBJDIR_DEBUG)\\RakSleep.o: RakSleep.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakSleep.cpp -o $(OBJDIR_DEBUG)\\RakSleep.o

$(OBJDIR_DEBUG)\\RakPeer.o: RakPeer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakPeer.cpp -o $(OBJDIR_DEBUG)\\RakPeer.o

$(OBJDIR_DEBUG)\\RakNetTypes.o: RakNetTypes.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNetTypes.cpp -o $(OBJDIR_DEBUG)\\RakNetTypes.o

$(OBJDIR_DEBUG)\\SimpleMutex.o: SimpleMutex.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c SimpleMutex.cpp -o $(OBJDIR_DEBUG)\\SimpleMutex.o

$(OBJDIR_DEBUG)\\SignaledEvent.o: SignaledEvent.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c SignaledEvent.cpp -o $(OBJDIR_DEBUG)\\SignaledEvent.o

$(OBJDIR_DEBUG)\\DR_SHA1.o: DR_SHA1.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c DR_SHA1.cpp -o $(OBJDIR_DEBUG)\\DR_SHA1.o

$(OBJDIR_DEBUG)\\ReliabilityLayer.o: ReliabilityLayer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c ReliabilityLayer.cpp -o $(OBJDIR_DEBUG)\\ReliabilityLayer.o

$(OBJDIR_DEBUG)\\PluginInterface2.o: PluginInterface2.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c PluginInterface2.cpp -o $(OBJDIR_DEBUG)\\PluginInterface2.o

$(OBJDIR_DEBUG)\\PacketizedTCP.o: PacketizedTCP.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c PacketizedTCP.cpp -o $(OBJDIR_DEBUG)\\PacketizedTCP.o

$(OBJDIR_DEBUG)\\NetworkIDObject.o: NetworkIDObject.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c NetworkIDObject.cpp -o $(OBJDIR_DEBUG)\\NetworkIDObject.o

$(OBJDIR_DEBUG)\\RakNetSocket.o: RakNetSocket.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakNetSocket.cpp -o $(OBJDIR_DEBUG)\\RakNetSocket.o

$(OBJDIR_DEBUG)\\RakMemoryOverride.o: RakMemoryOverride.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c RakMemoryOverride.cpp -o $(OBJDIR_DEBUG)\\RakMemoryOverride.o

$(OBJDIR_DEBUG)\\WSAStartupSingleton.o: WSAStartupSingleton.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c WSAStartupSingleton.cpp -o $(OBJDIR_DEBUG)\\WSAStartupSingleton.o

$(OBJDIR_DEBUG)\\TCPInterface.o: TCPInterface.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c TCPInterface.cpp -o $(OBJDIR_DEBUG)\\TCPInterface.o

$(OBJDIR_DEBUG)\\SuperFastHash.o: SuperFastHash.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c SuperFastHash.cpp -o $(OBJDIR_DEBUG)\\SuperFastHash.o

$(OBJDIR_DEBUG)\\StringTable.o: StringTable.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c StringTable.cpp -o $(OBJDIR_DEBUG)\\StringTable.o

$(OBJDIR_DEBUG)\\StringCompressor.o: StringCompressor.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c StringCompressor.cpp -o $(OBJDIR_DEBUG)\\StringCompressor.o

$(OBJDIR_DEBUG)\\SocketLayer.o: SocketLayer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c SocketLayer.cpp -o $(OBJDIR_DEBUG)\\SocketLayer.o

$(OBJDIR_DEBUG)\\VariableListDeltaTracker.o: VariableListDeltaTracker.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c VariableListDeltaTracker.cpp -o $(OBJDIR_DEBUG)\\VariableListDeltaTracker.o

$(OBJDIR_DEBUG)\\CCRakNetSlidingWindow.o: CCRakNetSlidingWindow.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c CCRakNetSlidingWindow.cpp -o $(OBJDIR_DEBUG)\\CCRakNetSlidingWindow.o

$(OBJDIR_DEBUG)\\BitStream.o: BitStream.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c BitStream.cpp -o $(OBJDIR_DEBUG)\\BitStream.o

$(OBJDIR_DEBUG)\\DS_HuffmanEncodingTree.o: DS_HuffmanEncodingTree.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c DS_HuffmanEncodingTree.cpp -o $(OBJDIR_DEBUG)\\DS_HuffmanEncodingTree.o

$(OBJDIR_DEBUG)\\DS_ByteQueue.o: DS_ByteQueue.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c DS_ByteQueue.cpp -o $(OBJDIR_DEBUG)\\DS_ByteQueue.o

$(OBJDIR_DEBUG)\\LocklessTypes.o: LocklessTypes.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c LocklessTypes.cpp -o $(OBJDIR_DEBUG)\\LocklessTypes.o

$(OBJDIR_DEBUG)\\Itoa.o: Itoa.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c Itoa.cpp -o $(OBJDIR_DEBUG)\\Itoa.o

$(OBJDIR_DEBUG)\\NetworkIDManager.o: NetworkIDManager.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c NetworkIDManager.cpp -o $(OBJDIR_DEBUG)\\NetworkIDManager.o

$(OBJDIR_DEBUG)\\GetTime.o: GetTime.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c GetTime.cpp -o $(OBJDIR_DEBUG)\\GetTime.o

$(OBJDIR_DEBUG)\\FileOperations.o: FileOperations.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c FileOperations.cpp -o $(OBJDIR_DEBUG)\\FileOperations.o

$(OBJDIR_DEBUG)\\FileListTransfer.o: FileListTransfer.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c FileListTransfer.cpp -o $(OBJDIR_DEBUG)\\FileListTransfer.o

$(OBJDIR_DEBUG)\\FileList.o: FileList.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c FileList.cpp -o $(OBJDIR_DEBUG)\\FileList.o
	
$(OBJDIR_DEBUG)\\IncrementalReadInterface.o: IncrementalReadInterface.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c IncrementalReadInterface.cpp -o $(OBJDIR_DEBUG)\\IncrementalReadInterface.o
	
clean_debug:
	cmd /c del /f $(OUT_DEBUG)
	cmd /c rd /s/q $(OBJDIR_DEBUG)

before_release:
	cmd /c if not exist $(OBJDIR_RELEASE) md $(OBJDIR_RELEASE)

after_release:

release: before_release out_release after_release

out_release: $(OBJ_RELEASE) $(DEP_RELEASE)
	$(AR) -rv $(OUT_RELEASE) $(OBJ_RELEASE)

$(OBJDIR_RELEASE)\\RakNetSocket2.o: RakNetSocket2.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNetSocket2.cpp -o $(OBJDIR_RELEASE)\\RakNetSocket2.o

$(OBJDIR_RELEASE)\\Rand.o: Rand.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c Rand.cpp -o $(OBJDIR_RELEASE)\\Rand.o

$(OBJDIR_RELEASE)\\RakThread.o: RakThread.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakThread.cpp -o $(OBJDIR_RELEASE)\\RakThread.o

$(OBJDIR_RELEASE)\\RakString.o: RakString.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakString.cpp -o $(OBJDIR_RELEASE)\\RakString.o

$(OBJDIR_RELEASE)\\RakSleep.o: RakSleep.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakSleep.cpp -o $(OBJDIR_RELEASE)\\RakSleep.o

$(OBJDIR_RELEASE)\\RakPeer.o: RakPeer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakPeer.cpp -o $(OBJDIR_RELEASE)\\RakPeer.o

$(OBJDIR_RELEASE)\\RakNetTypes.o: RakNetTypes.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNetTypes.cpp -o $(OBJDIR_RELEASE)\\RakNetTypes.o

$(OBJDIR_RELEASE)\\SimpleMutex.o: SimpleMutex.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c SimpleMutex.cpp -o $(OBJDIR_RELEASE)\\SimpleMutex.o

$(OBJDIR_RELEASE)\\SignaledEvent.o: SignaledEvent.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c SignaledEvent.cpp -o $(OBJDIR_RELEASE)\\SignaledEvent.o

$(OBJDIR_RELEASE)\\DR_SHA1.o: DR_SHA1.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c DR_SHA1.cpp -o $(OBJDIR_RELEASE)\\DR_SHA1.o

$(OBJDIR_RELEASE)\\ReliabilityLayer.o: ReliabilityLayer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c ReliabilityLayer.cpp -o $(OBJDIR_RELEASE)\\ReliabilityLayer.o

$(OBJDIR_RELEASE)\\PluginInterface2.o: PluginInterface2.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c PluginInterface2.cpp -o $(OBJDIR_RELEASE)\\PluginInterface2.o

$(OBJDIR_RELEASE)\\PacketizedTCP.o: PacketizedTCP.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c PacketizedTCP.cpp -o $(OBJDIR_RELEASE)\\PacketizedTCP.o

$(OBJDIR_RELEASE)\\NetworkIDObject.o: NetworkIDObject.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c NetworkIDObject.cpp -o $(OBJDIR_RELEASE)\\NetworkIDObject.o

$(OBJDIR_RELEASE)\\RakNetSocket.o: RakNetSocket.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakNetSocket.cpp -o $(OBJDIR_RELEASE)\\RakNetSocket.o

$(OBJDIR_RELEASE)\\RakMemoryOverride.o: RakMemoryOverride.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c RakMemoryOverride.cpp -o $(OBJDIR_RELEASE)\\RakMemoryOverride.o

$(OBJDIR_RELEASE)\\WSAStartupSingleton.o: WSAStartupSingleton.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c WSAStartupSingleton.cpp -o $(OBJDIR_RELEASE)\\WSAStartupSingleton.o

$(OBJDIR_RELEASE)\\TCPInterface.o: TCPInterface.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c TCPInterface.cpp -o $(OBJDIR_RELEASE)\\TCPInterface.o

$(OBJDIR_RELEASE)\\SuperFastHash.o: SuperFastHash.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c SuperFastHash.cpp -o $(OBJDIR_RELEASE)\\SuperFastHash.o

$(OBJDIR_RELEASE)\\StringTable.o: StringTable.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c StringTable.cpp -o $(OBJDIR_RELEASE)\\StringTable.o

$(OBJDIR_RELEASE)\\StringCompressor.o: StringCompressor.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c StringCompressor.cpp -o $(OBJDIR_RELEASE)\\StringCompressor.o

$(OBJDIR_RELEASE)\\SocketLayer.o: SocketLayer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c SocketLayer.cpp -o $(OBJDIR_RELEASE)\\SocketLayer.o

$(OBJDIR_RELEASE)\\VariableListDeltaTracker.o: VariableListDeltaTracker.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c VariableListDeltaTracker.cpp -o $(OBJDIR_RELEASE)\\VariableListDeltaTracker.o

$(OBJDIR_RELEASE)\\CCRakNetSlidingWindow.o: CCRakNetSlidingWindow.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c CCRakNetSlidingWindow.cpp -o $(OBJDIR_RELEASE)\\CCRakNetSlidingWindow.o

$(OBJDIR_RELEASE)\\BitStream.o: BitStream.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c BitStream.cpp -o $(OBJDIR_RELEASE)\\BitStream.o

$(OBJDIR_RELEASE)\\DS_HuffmanEncodingTree.o: DS_HuffmanEncodingTree.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c DS_HuffmanEncodingTree.cpp -o $(OBJDIR_RELEASE)\\DS_HuffmanEncodingTree.o

$(OBJDIR_RELEASE)\\DS_ByteQueue.o: DS_ByteQueue.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c DS_ByteQueue.cpp -o $(OBJDIR_RELEASE)\\DS_ByteQueue.o

$(OBJDIR_RELEASE)\\LocklessTypes.o: LocklessTypes.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c LocklessTypes.cpp -o $(OBJDIR_RELEASE)\\LocklessTypes.o

$(OBJDIR_RELEASE)\\Itoa.o: Itoa.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c Itoa.cpp -o $(OBJDIR_RELEASE)\\Itoa.o

$(OBJDIR_RELEASE)\\NetworkIDManager.o: NetworkIDManager.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c NetworkIDManager.cpp -o $(OBJDIR_RELEASE)\\NetworkIDManager.o

$(OBJDIR_RELEASE)\\GetTime.o: GetTime.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c GetTime.cpp -o $(OBJDIR_RELEASE)\\GetTime.o

$(OBJDIR_RELEASE)\\FileOperations.o: FileOperations.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c FileOperations.cpp -o $(OBJDIR_RELEASE)\\FileOperations.o

$(OBJDIR_RELEASE)\\FileListTransfer.o: FileListTransfer.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c FileListTransfer.cpp -o $(OBJDIR_RELEASE)\\FileListTransfer.o

$(OBJDIR_RELEASE)\\FileList.o: FileList.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c FileList.cpp -o $(OBJDIR_RELEASE)\\FileList.o
	
$(OBJDIR_RELEASE)\\IncrementalReadInterface.o: IncrementalReadInterface.cpp
	$(CXX) $(CFLAGSEXT_RELEASE) $(INC_RELEASE) -c IncrementalReadInterface.cpp -o $(OBJDIR_RELEASE)\\IncrementalReadInterface.o

clean_release:
	cmd /c del /f $(OUT_RELEASE)
	cmd /c rd /s/q $(OBJDIR_RELEASE)

.PHONY: before_debug after_debug clean_debug before_release after_release clean_release

