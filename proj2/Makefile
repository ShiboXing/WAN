CC=g++
CFLAGS = -std=c++11 -c -Wall -pedantic -g 
lr=1
svr_ip=localhost
svr_port=5555
rcv_app_port=4444
svr_app_port=3333

all: udp_stream udp_stream_rcv rt_rcv rt_svr

rt_rcv: sendto_dbg.o rt_rcv.o
		$(CC) -o rt_rcv rt_rcv.o sendto_dbg.o
rt_svr: sendto_dbg.o rt_svr.o
		$(CC) -o rt_svr rt_svr.o sendto_dbg.o
udp_stream: udp_stream.o
	    $(CC) -o udp_stream udp_stream.o
udp_stream_rcv: udp_stream_rcv.o
	    $(CC) -o udp_stream_rcv udp_stream_rcv.o
run_rcv: 
	./rt_rcv $(lr) $(svr_ip):$(svr_port) $(rcv_app_port)
run_svr:
	./rt_svr $(lr) $(svr_app_port) $(svr_port)
run_stream:
	./udp_stream localhost:${svr_app_port} 8000
run_stream_rcv:
	./udp_stream_rcv ${rcv_app_port}
clean:
	rm *.o
	rm udp_stream
	rm udp_stream_rcv
	rm rt_rcv
	rm rt_svr

%.o:    %.cpp
	$(CC) $(CFLAGS) $*.cpp


