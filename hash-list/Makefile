BINS = bench
OBJS = bench.o hash-list.o

CC = gcc
CFLAGS = -Wall -g -O2
LDFLAGS = -lpthread

.PHONY: all clean

all: $(BINS)

$(BINS): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

bench.o: bench.c
	$(CC) $(CFLAGS) -c -o $@ $<

hash-list.o: hash-list.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(BINS) $(OBJS)

