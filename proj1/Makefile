CC=g++
CFLAGS = -g -c -Wall -pedantic -std=c++14
LR=25 # loss rate
ADDR=localhost
S_FNAME_1=ncp_pd/pd1.txt
D_FNAME_1=rcv_pd/pd1_dst.txt
S_FNAME_2=ncp_pd/pd2.txt
D_FNAME_2=rcv_pd/pd2_dst.txt
S_FNAME_3=ncp_pd/pd3.txt
D_FNAME_3=rcv_pd/pd3_dst.txt
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

all: .utils ncp rcv main_ncp main_rcv ncp_pd .sender_cache rcv_pd ncp_pd/pd1.txt ncp_pd/pd2.txt ncp_pd/pd3.txt 

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
ncp_pd:
	mkdir ncp_pd 
rcv_pd:
	mkdir rcv_pd
.sender_cache:
	mkdir .sender_cache
ncp_pd/pd1.txt:
	python gen_file/gen_file.py -f ncp_pd/pd1.txt -b 20000001
ncp_pd/pd2.txt:
	python gen_file/gen_file.py -f ncp_pd/pd2.txt -b 20000020
ncp_pd/pd3.txt:
	python gen_file/gen_file.py -f ncp_pd/pd3.txt -b 20000300

run_ncp_1: 
	./ncp $(LR) $(ENV) $(S_FNAME_1) $(D_FNAME_1)@$(ADDR):$(PORT)
run_ncp_2: 
	./ncp $(LR) $(ENV) $(S_FNAME_2) $(D_FNAME_2)@$(ADDR):$(PORT)
run_ncp_3: 
	./ncp $(LR) $(ENV) $(S_FNAME_3) $(D_FNAME_3)@$(ADDR):$(PORT)
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
	rm -rf rcv_pd ncp_pd .sender_cache
clean:
	rm -rf *.o *.out ncp rcv t_ncp t_rcv rcv_pd/* .sender_cache/*

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