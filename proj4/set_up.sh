# Step 1: set bandwidth to 10mbps (during GENI node setup)
# Step 2: set RTT to 40ms
sudo tc qdisc add dev eth1 root netem delay 40ms
