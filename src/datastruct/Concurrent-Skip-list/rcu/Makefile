LIBS = -pthread
TCLAP = -I../tclap/include/		# header only option parser
CXXFLAGS = -Wall -std=c++17 -O1 -g
CFLAGS = -Wall -O1 -g
CC = gcc
CXX = g++
C_OBJ = new-urcu.o
CXX_OBJ = rcu_api.o skip_list.o port.o
TARGET = benchmark


${TARGET}: %: %.cc ${C_OBJ} ${CXX_OBJ}
	$(CXX) $^ -o $@ $(LIBS) $(TCLAP)


${CXX_OBJ}: %.o: %.cc
	$(CXX) -c $< $(CXXFLAGS)


${C_OBJ}: %.o: %.c
	$(CC) -c $< $(CFLAGS)


clean:
	rm -f ${C_OBJ} ${CXX_OBJ} ${TARGET}
