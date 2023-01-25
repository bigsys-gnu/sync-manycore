LIBS = -lmvrlu-ordo
CXXFLAGS = -Wall -std=c++17 -pthread
INCLUDES := -I./include
TCLAP = -I../tclap/include/		# header only option parser
LIB_DIR := -L./lib

CFLSGS = -Wall
CFLAGS += -D_REENTRANT
CFLAGS += -Werror
CFLAGS += -mrtm
CFLAGS += -pthread

ifdef DEBUG
	CFLAGS += -Og -g3
	CXXFLAGS += -Og -g3 -DDEBUG
else
	CFLAGS += -DNDEBUG
	CFLAGS += -O2 -g
	CXXFLAGS += -O2 -g
endif

ifdef TRACE
TRACE_MACRO += TRACE=true
CXXFLAGS += -DMVRLU_ENABLE_STATS
endif

ifdef GCC
CC := gcc
CXX := g++
else
CC := clang
CXX := clang++
CXXFLAGS += -stdlib=libc++
USE_CLANG += CLANG=true
endif

C_OBJ =
CXX_OBJ = mvrlu_api.o skip_list.o
BIN = benchmark


benchmark: benchmark.cc mvrlu $(CXX_OBJ)
	$(CXX) -o $@ $< $(CXX_OBJ) $(LIBS) $(CXXFLAGS) $(INCLUDES) $(LIB_DIR) $(TCLAP)


${CXX_OBJ}: %.o: %.cc
	$(CXX) -c $< $(CXXFLAGS) $(INCLUDES)


${C_OBJ}: %.o: %.c
	$(CC) -c $< $(CFLAGS) $(INCLUDES)


# C libraries
rlu.o: rlu.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c -o $@ $<


rlu-ordo.o: rlu.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -DRLU_ORDO_TIMESTAMPING -c -o $@ $<


mvrlu:
	$(MAKE) -C ./lib/ ${TRACE_MACRO} ${USE_CLANG}


numa-config.h:
	$(TOOLS_DIR)/cpu-topology.py > $(CUR_DIR)/numa-config.h


clean:
	rm -f *.o ${BIN}
	$(MAKE) -C ./lib/ clean
