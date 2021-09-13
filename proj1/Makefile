CC=gcc

all: npc rcv

npc: npc.o
	$(CC) -o npc npc.o
rcv: rcv.o
	$(CC) -o rcv rcv.o
run: 
	./npc localhost:5555

clean:
	rm *.o *.out npc rcv 