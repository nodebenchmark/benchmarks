app=$1
mode=$2

if [ "$mode" = "cache" ]; then
	type=$3 #inst for icache, and data for dcache
	if [ -z "$type" ]; then
		echo "which cache type? [inst/data]"
		exit
	fi
	for size in 8 16 32 64 128 256 512 1024
	do
		for assoc in 4 8
		do
			for line in 16 32 64
			do
				echo $size $assoc $line
				sh client.sh $app $mode $type $size $assoc $line
			done
		done
	done
elif [ "$mode" = "bp" ]; then
	scheme=$3
	if [ "$scheme" = "global" ]; then
		for global_size in 2 4 8 16 32 64 128 256 512 1024 2048 4096
		do
			echo $global_size
			sh client.sh $app $mode $scheme $global_size
		done
	elif [ "$scheme" = "local" ]; then
		for local_size in 2 4 8 16 32 64 128 256 512 1024 2048 4096
		do
			for local_histories in 2 4 8 16 32 64 128 256 512 1024
			do
				echo $local_size $local_histories
				sh client.sh $app $mode $scheme $local_size $local_histories
			done
		done
	else
		echo "which prediction scheme? [global/local]"
	fi
elif [ "$mode" = "tlb" ]; then
	for sets in 1 2 4 8 16
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
else
	echo "which mode? [cache/bp/tlb]"
fi
