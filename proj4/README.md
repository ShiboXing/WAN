## Usage

```python
# This command will send the receiver node UDP traffic at a bandwidth of 20mb/s for 10 seconds. It will also report back the amount of packet loss it experiences.

#sender:client, receiver:server
iperf -c receiver -u -b 20mb -t 10 & 
 
# This command will change the outgoing interface to send at a rate of 18mb/s.
#  The limit to 1000mb and burst to 10kb. These effectively changed the size of the outgoing queue on the Router. Limit refers to how much information can we store in the queue before we begin dropping packets. (Note: Burst is a space allocation used to shape incoming traffic into the desired outgoing rate and is not important to understand for the purpose of this module.)

sudo tc qdisc add dev eth2 root tbf rate 18mbit limit 1000mb burst 10kb

# add 40ms delay on router
sudo tc qdisc add dev eth2 root netem delay 40ms
#reset delay on rounter
sudo tc qdisc del dev eth2 root
```
