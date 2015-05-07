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
	epsilon=$7
	policy=$8
	prefetch=$9
	missseq=${10}

	if [ "$type" = "inst" ]; then
		prefix=i
	else
		prefix=d
	fi

	if [ "$policy" = "opt" ]; then
		tool=opt_cache
	elif [ "$policy" = "lru" ]; then
		tool=cache_lru
	elif [ "$policy" = "bip" ]; then
		tool=cache_bip
	else
		tool=cache
	fi

	if [ "$prefetch" = "1" ]; then
		outfile=${prefix}cache.${policy}.${size}.${assoc}.${line}.prefetch
		seqoutfile=missseq.prefetch.out
	else
		outfile=${prefix}cache.${policy}.${size}.${assoc}.${line}
		seqoutfile=missseq.out
	fi

	if [ -s $app/${outfile} ]; then
		exit
	fi

	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/MyPinTool/obj-intel64/${tool}.so -${prefix}enable 1 -${prefix}l1size $size -${prefix}l1assoc $assoc -${prefix}l1line $line -${prefix}l1pf $prefetch -${prefix}l1epsilon $epsilon -${prefix}l1missseq ${missseq} -f ${seqoutfile} -o ${outfile} -- "
	cd $app
	eval $(../nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app --server-only) &
elif [ "$mode" = "intrareuse" ]; then
	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/CacheReuse/obj-intel64/intra-reuse.so -- "
	cd $app
	eval $(../nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app --server-only) &
elif [ "$mode" = "interreuse" ]; then
	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/CacheReuse/obj-intel64/inter-reuse.so -- "
	cd $app
	eval $(../nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app --server-only) &
elif [ "$mode" = "bp" ]; then
	scheme=$3

	if [ "$scheme" = "global" ]; then
		global_size=$4
		outfile="bp.out.${scheme}.${global_size}"
		properties="-global-size $global_size -o ${outfile}"
	elif [ "$scheme" = "local" ]; then
		local_size=$4
		local_histories=$5
		outfile="bp.out.${scheme}.${local_size}.${local_histories}"
		properties="-local-size $local_size -local-histories $local_histories -o ${outfile}"
	elif [ "$scheme" = "tournament" ]; then
		global_size=$4
		local_size=$5
		local_histories=$6
		tournament_size=$7
		outfile="bp.out.${scheme}.${global_size}.${local_size}.${local_histories}.${tournament_size}"
		properties="-global-size $global_size -local-size $local_size -local-histories $local_histories -tournament-size $tournament_size -o ${outfile}"
	fi
	if [ -s $app/${outfile} ]; then
		exit
	fi
	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/MyPinTool/obj-intel64/BranchPredictor.so -type $scheme $properties -- "
	cd $app
	eval $(../nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app --server-only) &
elif [ "$mode" = "tlb" ]; then
	sets=$3
	ways=$4
	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/MyPinTool/obj-intel64/tlbs.so -l1-sets $sets -l1-ways $ways -o tlbsim.out.${sets}.${ways} -- "
	if [ -s $app/tlbsim.out.${sets}.${ways} ]; then
		exit
	fi
	cd $app
	eval $(../nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app --server-only) &
elif [ "$mode" = "inscount" ]; then
	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/MyPinTool/obj-intel64/inscount.so -o inscount.out -- "
	cd $app
	eval $(../nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app --server-only) &
elif [ "$mode" = "insmix" ]; then
	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/Mica/obj-intel64/mica.so -o insmix.out -- "
	cd $app
	eval $(../nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app --server-only) &
elif [ "$mode" = "footprint" ]; then
	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/MyPinTool/obj-intel64/InstFootprint.so -o footprint.out -- "
	cd $app
	eval $(../nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app --server-only) &
elif [ "$mode" = "vis" ]; then
	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/MyPinTool/obj-intel64/vis.so -o visaddr.out -- "
	cd $app
	eval $(../nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app --server-only) &
elif [ "$mode" = "bt" ]; then
	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/MyPinTool/obj-intel64/BranchTargets.so -o bt.out -- "
	cd $app
	eval $(../nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app --server-only) &
elif [ "$mode" = "acs" ]; then
	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/MyPinTool/obj-intel64/AcsPattern.so -o bblacs.out -- "
	cd $app
	eval $(../nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app --server-only) &
elif [ "$mode" = "branchcdf" ]; then
	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/MyPinTool/obj-intel64/BranchCDF.so -o branchcdf.out -- "
	cd $app
	eval $(../nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app --server-only) &
elif [ "$mode" = "branchalias" ]; then
	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/BranchAliasing/obj-intel64/BranchAliasing.so -o branchaliases.out -- "
	cd $app
	eval $(../nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app --server-only) &
elif [ "$mode" = "reuse" ]; then
	cd $app
	type=$3
	if [ "$type" = "event" ]; then
		cp $PIN_HOME/source/tools/Mica/mica.conf.reuse-event mica.conf
		outfile="reuse.event.out"
	else
		cp $PIN_HOME/source/tools/Mica/mica.conf.reuse-app mica.conf
		outfile="reuse.app.out"
	fi

	instrumentation="$PIN_HOME/pin.sh -t $PIN_HOME/source/tools/Mica/obj-intel64/mica.so -o ${outfile} -- "
	eval $(../nodebench -i "$instrumentation" -n $NODE_HOME/node -b $app --server-only) &
fi

rm -f /tmp/$app
while [ ! -e /tmp/$app ]
do
	sleep 2
done
sleep 35
rm /tmp/$app
../nodebench -b $app -c 100 --client-only
cd ..
sleep 5

pid=`ps ux | grep "/group/users/yzhu/benchmarks/node/node" | grep $app | awk '{if(NR==1) print $2}'`
kill -INT $pid
sleep 3
