#
# sqlite3 Makefile
#

CC      = gcc
AR	    = ar
CFLAGS  = -O2 -fPIC -Wall -ansi -pedantic

SHLD = ${CC} ${CFLAGS}
LDSHFLAGS =
LDFLAGS =

RM      = cmd /c del /f


COMPILE.c=$(CC) $(CFLAGS) -c
.c.o:
	$(COMPILE.c) -o $@ $<


SRCS = sqlite3.c

OBJS = $(SRCS:.c=.o)


default:	libsqlite3.a

libsqlite3.a:	$(OBJS)
	$(AR) -rv libsqlite3.a $(OBJS)

clean:
	$(RM) $(OBJS)