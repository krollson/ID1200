CC = gcc

CFLAGS = -Wall

EXEC = main

OBJS = main.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)

cas.o: cas.c
	$(CC) $(CFLAGS) -c pthread_sum.c

clean:
	rm -f $(OBJS) $(EXEC)