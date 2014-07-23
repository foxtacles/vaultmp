WORKDIR = %cd%

CC = gcc.exe
CXX = g++.exe
AR = ar.exe
LD = g++.exe
WINDRES = windres.exe

INC = -I. -Ilib
CFLAGS = -pedantic-errors -pedantic -Wfatal-errors -Wextra -Wall -std=gnu++1y
CFLAGSEXT =
RESINC =
RCFLAGS = -F pe-i386
LIBDIR =
LIB = -Llib\\RakNet -Llib\\iniparser
LDFLAGS = -m32 -liniparser -lRakNet ufmod.obj -lshlwapi -lcomctl32 -lwinmm -lws2_32 -lz -limagehlp -static -mwindows

INC_DEBUG = $(INC) -Ilib\\stack_trace\\include
CFLAGS_DEBUG = $(CFLAGS) -gstabs -DVAULTMP_DEBUG -DPACKAGE -DPACKAGE_VERSION
CFLAGSEXT_DEBUG = $(CFLAGSEXT)
RESINC_DEBUG = $(RESINC)
RCFLAGS_DEBUG = $(RCFLAGS)
LIBDIR_DEBUG = $(LIBDIR)
LIB_DEBUG = $(LIB)
LDFLAGS_DEBUG = -lbfd -liberty $(LDFLAGS)
OBJDIR_DEBUG = .objs\\Debug
DEP_DEBUG =
OUT_DEBUG = vaultmpd.exe

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
OUT_RELEASE = vaultmp.exe

OBJ_DEBUG = \
$(OBJDIR_DEBUG)\\lib\\stack_trace\\src\\stack.o \
$(OBJDIR_DEBUG)\\Utils.o \
$(OBJDIR_DEBUG)\\Shared.o \
$(OBJDIR_DEBUG)\\ServerEntry.o \
$(OBJDIR_DEBUG)\\Reference.o \
$(OBJDIR_DEBUG)\\vaultmp.o \
$(OBJDIR_DEBUG)\\vaultmprc.o \
$(OBJDIR_DEBUG)\\VaultVector.o \
$(OBJDIR_DEBUG)\\VaultFunctor.o \
$(OBJDIR_DEBUG)\\VaultException.o \
$(OBJDIR_DEBUG)\\Value.o \
$(OBJDIR_DEBUG)\\Guarded.o \
$(OBJDIR_DEBUG)\\GameFactory.o \
$(OBJDIR_DEBUG)\\Game.o \
$(OBJDIR_DEBUG)\\CriticalSection.o \
$(OBJDIR_DEBUG)\\Container.o \
$(OBJDIR_DEBUG)\\ItemList.o \
$(OBJDIR_DEBUG)\\Bethesda.o \
$(OBJDIR_DEBUG)\\Actor.o \
$(OBJDIR_DEBUG)\\API.o \
$(OBJDIR_DEBUG)\\Player.o \
$(OBJDIR_DEBUG)\\Pipe.o \
$(OBJDIR_DEBUG)\\Object.o \
$(OBJDIR_DEBUG)\\NetworkClient.o \
$(OBJDIR_DEBUG)\\Network.o \
$(OBJDIR_DEBUG)\\Lockable.o \
$(OBJDIR_DEBUG)\\Item.o \
$(OBJDIR_DEBUG)\\Interface.o \
$(OBJDIR_DEBUG)\\Expected.o \
$(OBJDIR_DEBUG)\\Window.o \
$(OBJDIR_DEBUG)\\Button.o \
$(OBJDIR_DEBUG)\\Text.o \
$(OBJDIR_DEBUG)\\Edit.o \
$(OBJDIR_DEBUG)\\Checkbox.o \
$(OBJDIR_DEBUG)\\Base.o \
$(OBJDIR_DEBUG)\\RadioButton.o \
$(OBJDIR_DEBUG)\\ListItem.o \
$(OBJDIR_DEBUG)\\List.o \
$(OBJDIR_DEBUG)\\Debug.o

OBJ_RELEASE = \
$(OBJDIR_RELEASE)\\Utils.o \
$(OBJDIR_RELEASE)\\Shared.o \
$(OBJDIR_RELEASE)\\ServerEntry.o \
$(OBJDIR_RELEASE)\\Reference.o \
$(OBJDIR_RELEASE)\\vaultmp.o \
$(OBJDIR_RELEASE)\\vaultmprc.o \
$(OBJDIR_RELEASE)\\VaultVector.o \
$(OBJDIR_RELEASE)\\VaultFunctor.o \
$(OBJDIR_RELEASE)\\VaultException.o \
$(OBJDIR_RELEASE)\\Value.o \
$(OBJDIR_RELEASE)\\Guarded.o \
$(OBJDIR_RELEASE)\\GameFactory.o \
$(OBJDIR_RELEASE)\\Game.o \
$(OBJDIR_RELEASE)\\CriticalSection.o \
$(OBJDIR_RELEASE)\\Container.o \
$(OBJDIR_RELEASE)\\ItemList.o \
$(OBJDIR_RELEASE)\\Bethesda.o \
$(OBJDIR_RELEASE)\\Actor.o \
$(OBJDIR_RELEASE)\\API.o \
$(OBJDIR_RELEASE)\\Player.o \
$(OBJDIR_RELEASE)\\Pipe.o \
$(OBJDIR_RELEASE)\\Object.o \
$(OBJDIR_RELEASE)\\NetworkClient.o \
$(OBJDIR_RELEASE)\\Network.o \
$(OBJDIR_RELEASE)\\Lockable.o \
$(OBJDIR_RELEASE)\\Item.o \
$(OBJDIR_RELEASE)\\Interface.o \
$(OBJDIR_RELEASE)\\Expected.o \
$(OBJDIR_RELEASE)\\Window.o \
$(OBJDIR_RELEASE)\\Button.o \
$(OBJDIR_RELEASE)\\Text.o \
$(OBJDIR_RELEASE)\\Edit.o \
$(OBJDIR_RELEASE)\\Checkbox.o \
$(OBJDIR_RELEASE)\\Base.o \
$(OBJDIR_RELEASE)\\RadioButton.o \
$(OBJDIR_RELEASE)\\ListItem.o \
$(OBJDIR_RELEASE)\\List.o

all: debug release

clean: clean_debug clean_release

before_debug:
	cmd /c if not exist $(OBJDIR_DEBUG) md $(OBJDIR_DEBUG)
	cmd /c if not exist $(OBJDIR_DEBUG)\\lib\\stack_trace\\src md $(OBJDIR_DEBUG)\\lib\\stack_trace\\src

after_debug:

debug: before_debug out_debug after_debug

out_debug: $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) $(LIBDIR_DEBUG) $(OBJ_DEBUG) $(LIB_DEBUG) -o $(OUT_DEBUG) $(LDFLAGS_DEBUG)

$(OBJDIR_DEBUG)\\lib\\stack_trace\\src\\stack.o: lib\\stack_trace\\src\\stack.cpp
	$(CXX) $(CFLAGSEXT_DEBUG) $(INC_DEBUG) -c lib\\stack_trace\\src\\stack.cpp -o $(OBJDIR_DEBUG)\\lib\\stack_trace\\src\\stack.o

$(OBJDIR_DEBUG)\\Utils.o: Utils.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Utils.cpp -o $(OBJDIR_DEBUG)\\Utils.o

$(OBJDIR_DEBUG)\\Shared.o: Shared.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Shared.cpp -o $(OBJDIR_DEBUG)\\Shared.o

$(OBJDIR_DEBUG)\\ServerEntry.o: ServerEntry.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c ServerEntry.cpp -o $(OBJDIR_DEBUG)\\ServerEntry.o

$(OBJDIR_DEBUG)\\Reference.o: Reference.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Reference.cpp -o $(OBJDIR_DEBUG)\\Reference.o

$(OBJDIR_DEBUG)\\vaultmp.o: vaultmp.rc vaultmp.cpp
	$(WINDRES) $(RCFLAGS_DEBUG) $(RESINC_DEBUG) -o $(OBJDIR_DEBUG)\\vaultmprc.o vaultmp.rc
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c vaultmp.cpp -o $(OBJDIR_DEBUG)\\vaultmp.o

$(OBJDIR_DEBUG)\\VaultVector.o: VaultVector.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c VaultVector.cpp -o $(OBJDIR_DEBUG)\\VaultVector.o

$(OBJDIR_DEBUG)\\VaultFunctor.o: VaultFunctor.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c VaultFunctor.cpp -o $(OBJDIR_DEBUG)\\VaultFunctor.o

$(OBJDIR_DEBUG)\\VaultException.o: VaultException.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c VaultException.cpp -o $(OBJDIR_DEBUG)\\VaultException.o

$(OBJDIR_DEBUG)\\Value.o: Value.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Value.cpp -o $(OBJDIR_DEBUG)\\Value.o

$(OBJDIR_DEBUG)\\Guarded.o: Guarded.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Guarded.cpp -o $(OBJDIR_DEBUG)\\Guarded.o

$(OBJDIR_DEBUG)\\GameFactory.o: GameFactory.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c GameFactory.cpp -o $(OBJDIR_DEBUG)\\GameFactory.o

$(OBJDIR_DEBUG)\\Game.o: Game.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Game.cpp -o $(OBJDIR_DEBUG)\\Game.o

$(OBJDIR_DEBUG)\\CriticalSection.o: CriticalSection.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c CriticalSection.cpp -o $(OBJDIR_DEBUG)\\CriticalSection.o

$(OBJDIR_DEBUG)\\Container.o: Container.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Container.cpp -o $(OBJDIR_DEBUG)\\Container.o

$(OBJDIR_DEBUG)\\ItemList.o: ItemList.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c ItemList.cpp -o $(OBJDIR_DEBUG)\\ItemList.o

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

$(OBJDIR_DEBUG)\\Debug.o: Debug.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Debug.cpp -o $(OBJDIR_DEBUG)\\Debug.o

$(OBJDIR_DEBUG)\\Expected.o: Expected.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Expected.cpp -o $(OBJDIR_DEBUG)\\Expected.o

$(OBJDIR_DEBUG)\\Window.o: Window.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Window.cpp -o $(OBJDIR_DEBUG)\\Window.o

$(OBJDIR_DEBUG)\\Button.o: Button.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Button.cpp -o $(OBJDIR_DEBUG)\\Button.o

$(OBJDIR_DEBUG)\\Text.o: Text.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Text.cpp -o $(OBJDIR_DEBUG)\\Text.o

$(OBJDIR_DEBUG)\\Edit.o: Edit.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Edit.cpp -o $(OBJDIR_DEBUG)\\Edit.o

$(OBJDIR_DEBUG)\\Checkbox.o: Checkbox.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Checkbox.cpp -o $(OBJDIR_DEBUG)\\Checkbox.o

$(OBJDIR_DEBUG)\\Base.o: Base.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Base.cpp -o $(OBJDIR_DEBUG)\\Base.o

$(OBJDIR_DEBUG)\\RadioButton.o: RadioButton.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c RadioButton.cpp -o $(OBJDIR_DEBUG)\\RadioButton.o

$(OBJDIR_DEBUG)\\ListItem.o: ListItem.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c ListItem.cpp -o $(OBJDIR_DEBUG)\\ListItem.o

$(OBJDIR_DEBUG)\\List.o: List.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c List.cpp -o $(OBJDIR_DEBUG)\\List.o

clean_debug:
	cmd /c del /f $(OBJ_DEBUG) $(OUT_DEBUG)
	cmd /c rd /s/q $(OBJDIR_DEBUG)

before_release:
	cmd /c if not exist $(OBJDIR_RELEASE) md $(OBJDIR_RELEASE)

after_release:

release: before_release out_release after_release

out_release: $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) $(LIBDIR_RELEASE) $(OBJ_RELEASE) $(LIB_RELEASE) -o $(OUT_RELEASE) $(LDFLAGS_RELEASE)

$(OBJDIR_RELEASE)\\Utils.o: Utils.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Utils.cpp -o $(OBJDIR_RELEASE)\\Utils.o

$(OBJDIR_RELEASE)\\Shared.o: Shared.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Shared.cpp -o $(OBJDIR_RELEASE)\\Shared.o

$(OBJDIR_RELEASE)\\ServerEntry.o: ServerEntry.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ServerEntry.cpp -o $(OBJDIR_RELEASE)\\ServerEntry.o

$(OBJDIR_RELEASE)\\Reference.o: Reference.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Reference.cpp -o $(OBJDIR_RELEASE)\\Reference.o

$(OBJDIR_RELEASE)\\vaultmp.o: vaultmp.rc vaultmp.cpp
	$(WINDRES) $(RCFLAGS_RELEASE) $(RESINC_RELEASE) -o $(OBJDIR_RELEASE)\\vaultmprc.o vaultmp.rc
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c vaultmp.cpp -o $(OBJDIR_RELEASE)\\vaultmp.o

$(OBJDIR_RELEASE)\\VaultVector.o: VaultVector.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c VaultVector.cpp -o $(OBJDIR_RELEASE)\\VaultVector.o

$(OBJDIR_RELEASE)\\VaultFunctor.o: VaultFunctor.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c VaultFunctor.cpp -o $(OBJDIR_RELEASE)\\VaultFunctor.o

$(OBJDIR_RELEASE)\\VaultException.o: VaultException.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c VaultException.cpp -o $(OBJDIR_RELEASE)\\VaultException.o

$(OBJDIR_RELEASE)\\Value.o: Value.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Value.cpp -o $(OBJDIR_RELEASE)\\Value.o

$(OBJDIR_RELEASE)\\Guarded.o: Guarded.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Guarded.cpp -o $(OBJDIR_RELEASE)\\Guarded.o

$(OBJDIR_RELEASE)\\GameFactory.o: GameFactory.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c GameFactory.cpp -o $(OBJDIR_RELEASE)\\GameFactory.o

$(OBJDIR_RELEASE)\\Game.o: Game.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Game.cpp -o $(OBJDIR_RELEASE)\\Game.o

$(OBJDIR_RELEASE)\\CriticalSection.o: CriticalSection.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c CriticalSection.cpp -o $(OBJDIR_RELEASE)\\CriticalSection.o

$(OBJDIR_RELEASE)\\Container.o: Container.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Container.cpp -o $(OBJDIR_RELEASE)\\Container.o

$(OBJDIR_RELEASE)\\ItemList.o: ItemList.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ItemList.cpp -o $(OBJDIR_RELEASE)\\ItemList.o

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

$(OBJDIR_RELEASE)\\Expected.o: Expected.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Expected.cpp -o $(OBJDIR_RELEASE)\\Expected.o

$(OBJDIR_RELEASE)\\Window.o: Window.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Window.cpp -o $(OBJDIR_RELEASE)\\Window.o

$(OBJDIR_RELEASE)\\Button.o: Button.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Button.cpp -o $(OBJDIR_RELEASE)\\Button.o

$(OBJDIR_RELEASE)\\Text.o: Text.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Text.cpp -o $(OBJDIR_RELEASE)\\Text.o

$(OBJDIR_RELEASE)\\Edit.o: Edit.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Edit.cpp -o $(OBJDIR_RELEASE)\\Edit.o

$(OBJDIR_RELEASE)\\Checkbox.o: Checkbox.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Checkbox.cpp -o $(OBJDIR_RELEASE)\\Checkbox.o

$(OBJDIR_RELEASE)\\Base.o: Base.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Base.cpp -o $(OBJDIR_RELEASE)\\Base.o

$(OBJDIR_RELEASE)\\RadioButton.o: RadioButton.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c RadioButton.cpp -o $(OBJDIR_RELEASE)\\RadioButton.o

$(OBJDIR_RELEASE)\\ListItem.o: ListItem.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ListItem.cpp -o $(OBJDIR_RELEASE)\\ListItem.o

$(OBJDIR_RELEASE)\\List.o: List.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c List.cpp -o $(OBJDIR_RELEASE)\\List.o

clean_release:
	cmd /c del /f $(OBJ_RELEASE) $(OUT_RELEASE)
	cmd /c rd /s/q $(OBJDIR_RELEASE)

.PHONY: before_debug after_debug clean_debug before_release after_release clean_release

