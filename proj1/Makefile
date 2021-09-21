CC=g++
CFLAGS = -g -c -Wall -pedantic -std=c++14
LR=5 # loss rate

USER= qil77
IDENTITYFILE= ~/.ssh/id_geni_ssh_rsa

NODE0 =  pc1.instageni.uvm.edu
port0 = 25210

NODE1= pc3.instageni.uvm.edu
PORT1=25210

NODE2=pc3.instageni.utdallas.edu
PORT2=26210

PROJECTROOT = ~/project1

all: .utils npc rcv main_npc main_rcv npc_payload rcv_payload npc_payload/payload.txt 

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
npc_payload:
	mkdir npc_payload 
rcv_payload:
	mkdir rcv_payload
npc_payload/payload.txt:
	python gen_file/gen_file.py -f npc_payload/payload.txt -b 2000000

run_npc: 
	./npc localhost:5555 $(LR)
run_rcv:
	./rcv 5555 $(LR)
clean_all: clean
	rm -rf rcv_payload npc_payload 
clean:
	rm -f *.o *.out npc rcv rcv_payload/* 

upload_node0: # run sender on node0
	scp -P 25210 -i $(IDENTITYFILE) -r ../proj1 $(USER)@$(NODE0):$(PROJECTROOT)
upload_node1: # run receiver on node1 as local environment,the ip address is 10.0.1.101
	scp -P 25210 -i $(IDENTITYFILE) -r ../proj1 $(USER)@$(NODE1):$(PROJECTROOT)
upload_node2:# run receiver on node2 as wide area network environment,the ip address is 10.0.2.202
	scp -P 26210 -i $(IDENTITYFILE) -r ../proj1 $(USER)@$(NODE2):$(PROJECTROOT)
upload_all: upload_node0 upload_node1 upload_node2  #passphrase is 980616

npc_to_node2:
	./npc 10.0.2.202:5555 $(LR)
npc_to_node1:
	./npc 10.0.1.101:5555 $(LR)
rcv_from_node:
	./rcv 5555 $(LR)

# %.o:    
# 	$(CC) $(CFLAGS) $*.cpp

#~./ssh/congig 
# Host node0
# HostName pc1.instageni.uvm.edu
# User your username
# Port 25210
# IdentityFile ~/.ssh/id_geni_ssh_rsa