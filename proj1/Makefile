CC=g++
CFLAGS = -g -c -Wall -pedantic -std=c++11

all: .utils npc rcv main_npc main_rcv

.utils: 
	$(CC) $(CFLAGS) utils/sendto_dbg.cpp 
	$(CC) $(CFLAGS) utils/file_helper.cpp 
npc: 
	$(CC) $(CFLAGS) npc.cpp 
rcv: 
	$(CC) $(CFLAGS) rcv.cpp 
main_npc:
	$(CC) -o npc npc.o file_helper.o sendto_dbg.o
main_rcv:
	$(CC) -o rcv rcv.o file_helper.o sendto_dbg.o

run_npc: 
	./npc localhost:5555
run_rcv:
	./rcv 5555
clean:
	rm *.o *.out npc rcv rcv_payload/* 

# %.o:    
# 	$(CC) $(CFLAGS) $*.cpp