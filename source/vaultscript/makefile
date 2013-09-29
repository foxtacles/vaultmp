WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = -I. -Idefault
CFLAGS = -std=gnu++0x
RESINC = 
LIBDIR = 
LIB = 
LDFLAGS =

INC_UNIX =  $(INC)
CFLAGS_UNIX =  $(CFLAGS) -m32 -O2 -Wall
RESINC_UNIX =  $(RESINC)
RCFLAGS_UNIX =  $(RCFLAGS)
LIBDIR_UNIX =  $(LIBDIR)
LIB_UNIX = $(LIB)
LDFLAGS_UNIX =  $(LDFLAGS) -m32 -s
OBJDIR_UNIX = obj/Unix
DEP_UNIX = 
OUT_UNIX = vaultscript.so

OBJ_UNIX = $(OBJDIR_UNIX)/vaultscript.o

all: unix

clean: clean_unix

before_unix: 
	test -d $(OBJDIR_UNIX) || mkdir -p $(OBJDIR_UNIX)

after_unix: 

unix: before_unix out_unix after_unix

out_unix: $(OBJ_UNIX) $(DEP_UNIX)
	$(LD) -shared $(LDFLAGS_UNIX) $(LIBDIR_UNIX) $(OBJ_UNIX) $(LIB_UNIX) -o $(OUT_UNIX)

$(OBJDIR_UNIX)/vaultscript.o: vaultscript.cpp
	$(CXX) $(CFLAGS_UNIX) $(INC_UNIX) -c vaultscript.cpp -o $(OBJDIR_UNIX)/vaultscript.o

clean_unix: 
	rm -f $(OBJ_UNIX) $(OUT_UNIX)
	rm -rf $(OBJDIR_UNIX)

.PHONY: before_unix after_unix clean_unix

