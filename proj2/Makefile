CC=g++

CFLAGS = -g -c -Wall -pedantic -std=c++14


all: srv rcv srv_stream rcv_stream


srv: 
	$(CC) $(CFLAGS) rt_srv.cpp 
rcv: 
	$(CC) $(CFLAGS) rt_rcv.cpp 

srv_stream: rt_srv.o
	    $(CC) -o rt_srv rt_srv.o

rcv_stream: rt_rcv.o
	    $(CC) -o rt_rcv rt_rcv.o
run_rcv:
	./rt_rcv 5555
run_srv:
	./rt_srv localhost:5555 5
clean:
	rm *.o rt_rcv rt_srv

veryclean:
	rm udp_stream
	rm udp_stream_rcv

%.o:    %.c
	$(CC) $(CFLAGS) $*.c


