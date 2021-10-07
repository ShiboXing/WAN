CC=g++
CFLAGS = -c -Wall -pedantic -g
lr=0
svr_ip=localhost
svr_port=5555
app_port=4444

all: udp_stream udp_stream_rcv rt_rcv rt_svr

rt_rcv: rt_rcv.o
		$(CC) -o rt_rcv rt_rcv.o
rt_svr: rt_svr.o
		$(CC) -o rt_svr rt_svr.o
udp_stream: udp_stream.o
	    $(CC) -o udp_stream udp_stream.o
udp_stream_rcv: udp_stream_rcv.o
	    $(CC) -o udp_stream_rcv udp_stream_rcv.o
run_rcv: 
	./rt_rcv $(lr) $(svr_ip):$(svr_port) $(app_port)
run_svr:
	./rt_svr $(lr) $(app_port) $(svr_port)
clean:
	rm *.o
	rm udp_stream
	rm udp_stream_rcv
	rm rt_rcv
	rm rt_svr

%.o:    %.cpp
	$(CC) $(CFLAGS) $*.cpp


