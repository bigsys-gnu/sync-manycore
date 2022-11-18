LIBS = -pthread
CXXFLAGS = -Wall -std=c++17
CFLSGS = -Wall
CC = gcc
CXX = g++
C_OBJ = new-urcu.o
CXX_OBJ = rcu_api.o skip_list.o port.o
TARGET = benchmark


${TARGET}: %: %.cc ${C_OBJ} ${CXX_OBJ}
	$(CXX) $^ -o $@


${CXX_OBJ}: %.o: %.cc
	$(CXX) -c $< $(LIBS) $(CXXFLAGS)


${C_OBJ}: %.o: %.c
	$(CC) -c $< $(LIBS) $(CFLAGS)

# port.o: port.cc
# 	$(CXX) -c $< $(CXXFLAGS) -fpic

clean:
	rm -f ${C_OBJ} ${CXX_OBJ} ${TARGET}
