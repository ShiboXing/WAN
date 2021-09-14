CC=gcc
CFLAGS = -g -c -Wall -pedantic

all: dbg npc rcv main_npc main_rcv
debug: dbg npc rcv

dbg: sendto_dbg.o
	$(CC) $(CFLAGS) sendto_dbg.c
npc: npc.o
	$(CC) $(CFLAGS) npc.c
rcv: rcv.o
	$(CC) $(CFLAGS) rcv.c 
main_npc:
	$(CC) -o npc sendto_dbg.o npc.o
main_rcv:
	$(CC) -o rcv sendto_dbg.o rcv.o

run_npc: 
	./npc localhost:5555
run_rcv:
	./rcv 5555
clean:
	rm *.o *.out npc rcv 

# %.o:    %.c
# 	$(CC) $(CFLAGS) $*.c