CC=gcc
CFLAGS = -g -c -Wall -pedantic

all: dbg npc rcv main
dbg: sendto_dbg.o
	$(CC) $(CFLAGS) sendto_dbg.c
npc: npc.o
	$(CC) $(CFLAGS) npc.c
rcv: rcv.o
	$(CC) $(CFLAGS) rcv.c 
main: 
	$(CC) -o npc sendto_dbg.o npc.o
	$(CC) -o rcv sendto_dbg.o rcv.o
run_npc: 
	./npc localhost:5555
run_rcv:
	./rcv 5555
clean:
	rm *.o *.out npc rcv 

# %.o:    %.c
# 	$(CC) $(CFLAGS) $*.c