CC=gcc

all: npc rcv

npc: npc.o
	$(CC) -o npc npc.o
rcv: rcv.o
	$(CC) -o rcv rcv.o
run_npc: 
	./npc localhost:5555
run_rcv:
	./rcv 5555
clean:
	rm *.o *.out npc rcv 