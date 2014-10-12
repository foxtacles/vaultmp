#ifndef VAULTSERVER_H
#define VAULTSERVER_H

#include "vaultmp.hpp"

#define MODFILES_PATH  "mods"
#define SCRIPTS_PATH   "scripts"
#define DATA_PATH      "data"
#define PWNFILES_PATH  "AMXFILE=files"
#define PWNFILES_KEY   "AMXFILE"
#define PWNFILES_VAL   "files"

#define DB_FALLOUT3 "fallout3.sqlite3"

#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

#ifndef __WIN32__
#define _getcwd getcwd
#endif

#ifndef HAVE_STDINT_H
#define HAVE_STDINT_H
#endif

#endif
