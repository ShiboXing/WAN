CC=g++
CFLAGS=-std=c++11 -c -Wall -pedantic -g 
lr=0
srv_ip=192.170.230.101
srv_port=5555
rcv_app_port=4444
srv_app_port=3333

all: udp_stream udp_stream_rcv rt_rcv rt_srv

rt_rcv: sendto_dbg.o stat_display.o rt_rcv.o
		$(CC) -o rt_rcv rt_rcv.o sendto_dbg.o stat_display.o
rt_srv: sendto_dbg.o stat_display.o rt_srv.o
		$(CC) -o rt_srv rt_srv.o sendto_dbg.o stat_display.o
udp_stream: udp_stream.o
	    $(CC) -o udp_stream udp_stream.o
udp_stream_rcv: udp_stream_rcv.o
	    $(CC) -o udp_stream_rcv udp_stream_rcv.o
run_rcv: 
	./rt_rcv $(lr) $(srv_ip):$(srv_port) $(rcv_app_port)
run_srv:
	./rt_srv $(lr) $(srv_app_port) $(srv_port)
run_stream:
	./udp_stream localhost:${srv_app_port} 25000
run_stream_rcv:
	./udp_stream_rcv ${rcv_app_port}

run_video:
	~/video_demo/multicat/multicat-2.3/multicat -u -U ~/video_demo/yellowstone_elemental.ts 127.0.0.1:3333
clean:
	rm *.o
	rm udp_stream
	rm udp_stream_rcv
	rm rt_rcv
	rm rt_srv

%.o:    %.cpp
	$(CC) $(CFLAGS) $*.cpp


