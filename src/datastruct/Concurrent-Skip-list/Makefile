LIBS = -pthread
CFLAGS = -Wall -std=c++17 ${LIBS} -g
CXX = g++
SRCS = skip_list
OBJS = $(addsuffix .o, ${SRCS})
TESTS = unit_test_1 unit_test_2 unit_test_3
TARGETS = skiplist benchmark ${TESTS}
TARGET_SRCS = $(addsuffix .cpp, ${TARGETS})

.PHONY = all run_test

all: ${TARGETS}

run_test: ${TESTS}
	for ut in $^; do \
		./$$ut;      \
	done

${TARGETS}: %: %.cpp ${OBJS}
	$(CXX) $(CFLAGS) -o $@ $^

${OBJS}: %.o: %.cpp
	$(CXX) $(CFLAGS) -c $^

clean:
	rm -f ${TARGETS} ${OBJS}
