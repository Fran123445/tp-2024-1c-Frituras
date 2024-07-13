# Libraries
LIBS=utils commons pthread readline m

# Custom libraries' paths
STATIC_LIBPATHS=../utils

# Compiler flags
CDEBUG=-g -Wall -DDEBUG -fdiagnostics-color=always
CRELEASE=-O3 -Wall -DNDEBUG -fcommon

# Arguments when executing with start, memcheck or helgrind
ARGS= --leak-check=full --show-leak-kinds=all 

# Valgrind flags
MEMCHECK_FLAGS=--track-origins=yes --log-file=memcheck.log
HELGRIND_FLAGS=
