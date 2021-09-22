CC=g++
CFLAGS = -g -c -Wall -pedantic -std=c++14
LR=25 # loss rate
ADDR=localhost
S_FNAME=payload.txt
D_FNAME=payload_dst.txt
PORT=5555
ENV=LAN

USER= qil77
IDENTITYFILE= ~/.ssh/id_geni_ssh_rsa

NODE0 =  pc1.instageni.uvm.edu
port0 = 25210

NODE1= pc3.instageni.uvm.edu
PORT1=25210

NODE2=pc3.instageni.utdallas.edu
PORT2=26210

PROJECTROOT = ~/project1

all: .utils ncp rcv main_ncp main_rcv ncp_payload rcv_payload ncp_payload/payload.txt 

bench: .utils t_ncp t_rcv main_t_ncp main_t_rcv

.utils: 
	$(CC) $(CFLAGS) utils/sendto_dbg.cpp 
	$(CC) $(CFLAGS) utils/file_helper.cpp 
ncp: 
	$(CC) $(CFLAGS) ncp.cpp 
rcv: 
	$(CC) $(CFLAGS) rcv.cpp 
main_ncp:
	$(CC) -o ncp ncp.o file_helper.o sendto_dbg.o
main_rcv:
	$(CC) -o rcv rcv.o file_helper.o sendto_dbg.o
ncp_payload:
	mkdir ncp_payload 
rcv_payload:
	mkdir rcv_payload
ncp_payload/payload.txt:
	python gen_file/gen_file.py -f ncp_payload/payload.txt -b 3000000

run_ncp: 
	./ncp $(LR) $(ENV) $(S_FNAME) $(D_FNAME)@$(ADDR):$(PORT)
run_rcv:
	./rcv $(LR) $(PORT) $(ENV)

t_ncp:
	$(CC) $(CFLAGS) t_ncp.cpp
t_rcv:
	$(CC) $(CFLAGS) t_rcv.cpp

main_t_ncp:
	$(CC) -o t_ncp t_ncp.o 
main_t_rcv:
	$(CC) -o t_rcv t_rcv.o 
run_t_ncp:
	./t_ncp localhost:5555
run_t_rcv:
	./t_rcv 5555

clean_all: clean
	rm -rf rcv_payload ncp_payload 
clean:
	rm -f *.o *.out ncp rcv rcv_payload/*  t_ncp t_rcv

upload_node0: # run sender on node0
	scp -P 25210 -i $(IDENTITYFILE) -r ../proj1 $(USER)@$(NODE0):$(PROJECTROOT)
upload_node1: # run receiver on node1 as local environment,the ip address is 10.0.1.101
	scp -P 25210 -i $(IDENTITYFILE) -r ../proj1 $(USER)@$(NODE1):$(PROJECTROOT)
upload_node2:# run receiver on node2 as wide area network environment,the ip address is 10.0.2.202
	scp -P 26210 -i $(IDENTITYFILE) -r ../proj1 $(USER)@$(NODE2):$(PROJECTROOT)
upload_all: upload_node0 upload_node1 upload_node2  #passphrase is 980616

ncp_to_node2:
	./ncp 10.0.2.202:5555 $(LR)
ncp_to_node1:
	./ncp 10.0.1.101:5555 $(LR)
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