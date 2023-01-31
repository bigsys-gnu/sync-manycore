LIBS = -pthread
TCLAP = -I../tclap/include/		# header only option parser
CXXFLAGS = -Wall -std=c++17 -O3 -g
CFLAGS = -Wall -O3 -g

ifdef GCC
CC = gcc
CXX = g++
else
CC = clang
CXX = clang++
CXXFLAGS += -stdlib=libc++
endif

ifdef STAT
CXXFLAGS += -DSTAT
endif

C_OBJ = new-urcu.o
CXX_OBJ = rcu_api.o skip_list.o port.o
TARGET = benchmark


${TARGET}: %: %.cc ${C_OBJ} ${CXX_OBJ}
	$(CXX) $^ -o $@ $(LIBS) $(TCLAP) $(CXXFLAGS)


${CXX_OBJ}: %.o: %.cc
	$(CXX) -c $< $(CXXFLAGS)


${C_OBJ}: %.o: %.c
	$(CC) -c $< $(CFLAGS)


clean:
	rm -f ${C_OBJ} ${CXX_OBJ} ${TARGET}
