all: sim_main

# Env for C
CC = gcc
CFLAGS = -std=c99 -Wall -O0

# Env for C++
CXX = g++
CXXFLAGS = -std=c++11 -Wall -O0

ifeq ($(DEBUG),1)
  CFLAGS += -g
  CXXFLAGS += -g
endif

# Automatically detect whether the core is C or C++
# Must have either sim_core.c or sim_core.cpp - NOT both
SRC_CORE = $(wildcard core_api.c core_api.cpp)
SRC_GIVEN = main.c sim_api.c
EXTRA_DEPS = sim_api.h core_api.h

OBJ_GIVEN = $(patsubst %.c,%.o,$(SRC_GIVEN))
OBJ_CORE = core_api.o
OBJ = $(OBJ_GIVEN) $(OBJ_CORE)

#$(info OBJ=$(OBJ))

ifeq ($(SRC_CORE),core_api.c)
sim_main: $(OBJ)
	gcc -o $@ $(OBJ)

sim_core.o: sim_core.c
	gcc -c $(CFLAGS) -o $@ $<

else
sim_main: $(OBJ)
	g++ -o $@ $(OBJ)

sim_core.o: sim_core.cpp
	g++ -c $(CXXFLAGS) -o $@ $<
endif

$(OBJ_GIVEN): %.o: %.c
	gcc -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f sim_main $(OBJ_GIVEN) $(OBJ_CORE)
