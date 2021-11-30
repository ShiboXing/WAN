
if [ -z "$1" ] 
then
	echo "please enter a BDP number for the bottleneck queue"
else
	HOST=`hostname`
	echo "hostname: $HOST"

	BDP=$(( $1 * 200 ))
	echo "queue size: ${BDP}kb"

	if [[ "$HOST" == *"router"* ]]; then
		echo 'setting router queue to '$1' BDPs'
		sudo tc qdisc add dev eth2 root tbf rate 100mbit limit ${BDP}kb burst 10kb
		sudo tc qdisc replace dev eth2 root tbf rate 100mbit limit ${BDP}kb burst 10kb
	fi
fi

