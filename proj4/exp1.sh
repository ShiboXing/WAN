
HOST=`hostname`
echo "hostname: $HOST"
BDP=$(( $1 * 400 ))
echo "BDP: $BDP"

if [[ "$HOST" == *"router"* ]]; then
	echo 'setting router queue to $BDP BDPs'
	sudo tc qdisc add dev eth2 root tbf rte 100mbit limit $BDPkb burst 10kb
	sudo tc qdisc replace dev eth2 root tbf rate 100mbit limit $BDPkb burst 10kb
fi


