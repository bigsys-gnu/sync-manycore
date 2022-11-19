LIBS = -pthread
CXXFLAGS = -Wall -std=c++17
CFLSGS = -Wall
CC = gcc
CXX = g++
C_OBJ = new-urcu.o
CXX_OBJ = rcu_api.o skip_list.o port.o
TARGET = benchmark


${TARGET}: %: %.cc ${C_OBJ} ${CXX_OBJ}
	$(CXX) $^ -o $@ $(LIBS)


${CXX_OBJ}: %.o: %.cc
	$(CXX) -c $< $(CXXFLAGS)


${C_OBJ}: %.o: %.c
	$(CC) -c $< $(CFLAGS)


clean:
	rm -f ${C_OBJ} ${CXX_OBJ} ${TARGET}
