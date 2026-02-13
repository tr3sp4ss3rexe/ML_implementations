# =============================================================================
# Usage:
#   make                     	# build optimized release: ./kmeans
#   make debug               	# build debug (no opts, symbols): ./kmeans
#   make vg ARGS="file.txt 3"	# run under Valgrind (use current build)
#
# Notes:
#   - 'make debug' overwrites ./kmeans with a debug build.
#   - Valgrind mode is fully interactive (no stdin piping).
# =============================================================================

CC      = gcc
CSTD    = -std=c99
WARN    = -Wall -Wextra
SRC     = main.c kmeans.c
OBJ     = $(SRC:.c=.o)
LDLIBS = -lm

CFLAGS_R = $(WARN) $(CSTD) -O2
CFLAGS_D = $(WARN) $(CSTD) -O0 -g

all: CFLAGS = $(CFLAGS_R)
all: kmeans

debug: clean
	$(MAKE) CFLAGS="$(CFLAGS_D)" kmeans

kmeans: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

VGFLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes \
          --errors-for-leak-kinds=all --num-callers=30
ARGS ?=
vg: kmeans
	valgrind $(VGFLAGS) ./kmeans $(ARGS)

clean:
	rm -f kmeans *.o

.PHONY: all debug vg clean
