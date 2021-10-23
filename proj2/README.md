# CS 2520 PROJECT 2
real time data transmission with smooth app delivery <br>

## make
```
make all
```
## run rt_rcv and rt_svr
```
rt_rcv <loss_rate_percent> <server_ip>:<server_port> <app_port>
rt_srv <loss_rate_percent> <app_port> <client_port>
```
##### or by changing the parameters in Makefile
```
lr:loss_rate_percent
srv_ip:server_ip
srv_port:server_port
rcv_app_port:app_port
srv_app_port:client_port
```
##### then
```
make run_rcv
make run_srv
```

## clean
```
make clean
```

