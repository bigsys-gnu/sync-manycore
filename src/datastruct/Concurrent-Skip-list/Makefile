LIBS = -pthread
CFLAGS = -Wall -std=c++17 ${LIBS}
CXX = g++
SRCS = key_value_pair node skip_list
OBJS = $(addsuffix .o, ${SRCS})
TARGETS = skiplist benchmark unit_test_1 unit_test_2 unit_test_3
TARGET_SRCS = $(addsuffix .cpp, ${TARGETS})

all: ${TARGETS}

${TARGETS}: %: %.cpp ${OBJS}
	$(CXX) $(CFLAGS) -o $@ $^

${OBJS}: %.o: %.cpp
	$(CXX) $(CFLAGS) -c $^

clean:
	rm -f ${TARGETS} ${OBJS}
