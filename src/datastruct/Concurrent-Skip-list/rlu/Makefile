LIBS = -lmvrlu-ordo
CXXFLAGS = -Wall -std=c++17 -pthread
INCLUDES := -I./include
LIB_DIR := -L./lib

CFLSGS = -Wall
CFLAGS += -D_REENTRANT
CFLAGS += -Werror
CFLAGS += -mrtm
CFLAGS += -pthread

ifdef DEBUG
	CFLAGS += -O0 -g3
else
	CFLAGS += -DNDEBUG
	CFLAGS += -O1 -g
endif

CC = gcc
CXX = g++
C_OBJ =
CXX_OBJ =
BIN = main


main: main.cc mvrlu
	$(CXX) -o $@ $< $(LIBS) $(CXXFLAGS) $(INCLUDES) $(LIB_DIR)


${CXX_OBJ}: %.o: %.cc
	$(CXX) -c $< $(LIBS) $(CXXFLAGS) $(INCLUDES)


${C_OBJ}: %.o: %.c
	$(CC) -c $< $(LIBS) $(CFLAGS) $(INCLUDES)


# C libraries
rlu.o: ./lib/rlu.c ./include/rlu.h
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c -o $@ $<


rlu-ordo.o: ./lib/rlu.c ./include/rlu.h
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -DRLU_ORDO_TIMESTAMPING -c -o $@ $<


mvrlu:
	$(MAKE) -C ./lib/


numa-config.h:
	$(TOOLS_DIR)/cpu-topology.py > $(CUR_DIR)/numa-config.h


clean:
	rm -f *.o ${BIN}
	$(MAKE) -C ./lib/ clean
