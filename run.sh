app=$1
mode=$2

if [ "$mode" = "cache" ]; then
	type=$3 #inst for icache, and data for dcache
	policy=$4
	prefetch=$5
	missseq=$6
	epsilon=16

	if [ -z "$type" ]; then
		echo "which cache type? [inst/data]"
		exit
	fi
	#for size in 8 16 32 64 128 256 512 1024
	for size in 32
	do
		#for assoc in 4 8 16
		for assoc in 8
		do
			#for line in 16 32 64 128
			for line in 64
			do
				echo "["$size $assoc $line"]" "["$policy $epsilon"]" $prefetch
				sh client.sh $app $mode $type $size $assoc $line $epsilon $policy $prefetch $missseq
			done
		done
	done
elif [ "$mode" = "intrareuse" ]; then
	sh client.sh $app $mode
elif [ "$mode" = "interreuse" ]; then
	sh client.sh $app $mode
elif [ "$mode" = "bp" ]; then
	scheme=$3
	if [ "$scheme" = "global" ]; then
		#for global_size in 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536
		for global_size in 128 256 512 1024 2048 4096 65536
		do
			echo $global_size
			sh client.sh $app $mode $scheme $global_size
		done
	elif [ "$scheme" = "local" ]; then
		#for local_size in 2 4 8 16 32 64 128 256 512 1024 2048 4096
		for local_size in 4096
		do
			#for local_histories in 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384
			for local_histories in 16384
			do
				echo $local_size $local_histories
				sh client.sh $app $mode $scheme $local_size $local_histories
			done
		done
	elif [ "$scheme" = "tournament" ]; then
		global_size=4096
		local_size=4096
		local_histories=256
		tournament_size=4096
		echo $global_size $local_size $local_histories $tournament_size
		sh client.sh $app $mode $scheme $global_size $local_size $local_histories $tournament_size
	else
		echo "which prediction scheme? [global/local]"
	fi
elif [ "$mode" = "tlb" ]; then
	for sets in 2 4 8 16
	do
		for ways in 32
		do
			echo $sets $ways
			sh client.sh $app $mode $sets $ways
		done
	done
	exit
	#for sets in 1 2 4 8 16
	for sets in 1
	do
		for ways in 1 2 4 8 16 32
		do
			echo $sets $ways
			sh client.sh $app $mode $sets $ways
		done
	done
elif [ "$mode" = "inscount" ]; then
	sh client.sh $app $mode
elif [ "$mode" = "insmix" ]; then
	sh client.sh $app $mode
elif [ "$mode" = "footprint" ]; then
	sh client.sh $app $mode
elif [ "$mode" = "vis" ]; then
	sh client.sh $app $mode
elif [ "$mode" = "bt" ]; then
	sh client.sh $app $mode
elif [ "$mode" = "acs" ]; then
	sh client.sh $app $mode
elif [ "$mode" = "branchcdf" ]; then
	sh client.sh $app $mode
elif [ "$mode" = "branchalias" ]; then
	sh client.sh $app $mode
elif [ "$mode" = "reuse" ]; then
	type=$3 #event for per-event, and app for the whole application
	sh client.sh $app $mode $type
else
	echo "which mode? [cache/bp/tlb]"
fi
