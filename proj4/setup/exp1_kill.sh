
lsof -i:$bbr_port -t | xargs kill -9
lsof -i:$loss_port -t | xargs kill -9
sudo tc qdisc del dev eth0 root
sudo tc qdisc del dev eth1 root
sudo tc qdisc del dev eth2 root
