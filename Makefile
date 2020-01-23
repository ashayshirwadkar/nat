CC=gcc
CFLAGS=-I. -g

all: nat_flow
nat_flow: nat_flow.o 
	$(CC) -o nat_flow nat_flow.o

clean:
	rm -f *.o *~ *.out nat_flow
