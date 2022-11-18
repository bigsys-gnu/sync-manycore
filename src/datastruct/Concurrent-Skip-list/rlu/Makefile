LIBS = -pthread
CXXFLAGS = -Wall -std=c++17

CFLSGS = -Wall
CFLAGS += -D_REENTRANT
CFLAGS += -Werror
CFLAGS += -mrtm

ifdef DEBUG
	CFLAGS += -O0 -g3
else
	CFLAGS += -DNDEBUG
	CFLAGS += -O1 -g
endif

CC = gcc
CXX = g++
C_OBJ =
CXX_OBJ = skip_list.o port.o
TARGET = benchmark


${TARGET}: %: %.cc ${C_OBJ} ${CXX_OBJ}
	$(CXX) $^ -o $@


${CXX_OBJ}: %.o: %.cc
	$(CXX) -c $< $(LIBS) $(CXXFLAGS)


${C_OBJ}: %.o: %.c
	$(CC) -c $< $(LIBS) $(CFLAGS)


# C libraries
rlu.o: rlu.c rlu.h
	$(CC) $(CFLAGS) $(DEFINES) -c -o $@ $<


rlu-ordo.o: rlu.c rlu.h
	$(CC) $(CFLAGS) $(DEFINES) -DRLU_ORDO_TIMESTAMPING -c -o $@ $<


numa-config.h:
	$(TOOLS_DIR)/cpu-topology.py > $(CUR_DIR)/numa-config.h


clean:
	rm -f ${C_OBJ} ${CXX_OBJ} ${TARGET}
