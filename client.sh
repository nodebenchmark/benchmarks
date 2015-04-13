HOME=/group/users/yzhu/benchmarks
NODE_HOME=$HOME/node
PIN_HOME=$HOME/pin-2.14-71313-gcc.4.4.7-linux

app=$1
mode=$2

if [ "$mode" = "cache" ]; then
	type=$3
	size=$4
	assoc=$5
	line=$6

	if [ "$type" = "inst" ]; then
		prefix=i
	else
		prefix=d
	fi
	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/MyPinTool/obj-intel64/cache.so -${prefix}enable 1 -${prefix}l1size $size -${prefix}l1assoc $assoc -${prefix}l1line $line -o ${prefix}cache.out.${size}.${assoc}.${line} -- "
	./nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app -c 5
elif [ "$mode" = "bp" ]; then
	scheme=$3

	if [ "$scheme" = "global" ]; then
		global_size=$4
		properties="-global-size $global_size -o bp.out.${scheme}.${global_size}"
	else
		local_size=$4
		local_histories=$5
		properties="-local-size $local_size -local-histories $local_histories -o bp.out.${scheme}.${local_size}.${local_histories}"
	fi
	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/MyPinTool/obj-intel64/BranchPredictor.so -type $scheme $properties -- "
	./nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app -c 5
elif [ "$mode" = "tlb" ]; then
	sets=$3
	ways=$4
	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/MyPinTool/obj-intel64/tlbs.so -l1-sets $sets -l1-ways $ways -o tlbsim.out.${sets}.${ways} -- "
	./nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app -c 5
fi

#/group/users/yzhu/pinplay-drdebug-2.1-pin-2.14-71313-gcc.4.4.7-linux/pin.sh -t /group/users/yzhu/pinplay-drdebug-2.1-pin-2.14-71313-gcc.4.4.7-linux/source/tools/MyPinTool/obj-intel64/icache.so -- /group/users/yzhu/node/node /group/users/yzhu/nodebench/etherpad-lite/node_modules/ep_etherpad-lite/node/server.js &
