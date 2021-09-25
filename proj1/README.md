This program mimics TCP transport using UDP packets with simulated losses on both sides and is to be compiled with C++14. 
### build
    make all 
<br> with default paramters:
### run receiver on localhost
    make run_rcv
### to start custom receiver run
    ./rcv <loss_rate_percent> <port> <env>
### run sender on localhost:
    make run_ncp 
### to start custom sender run
    ./ncp <loss_rate_percent> <env> <source_file_name> <dest_file_name>@<ip_address>:<port>
### clean object files
    make clean
### clean all generated files
    make clean_all