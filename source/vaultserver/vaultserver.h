#ifndef VAULTSERVER_H
#define VAULTSERVER_H

#define MODFILES_PATH "mods"
#define DATA_PATH "data"
#define PWNFILES_PATH "AMXFILE=files"

#define DB_FALLOUT3 "fallout3.sqlite3"

#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

#ifndef __WIN32__
#define _getcwd getcwd
#endif

#endif
