mode=$2

if [ "$mode" = "event" ]; then
	#head -n -2 $1/reuse.event.out | awk '{if(NR!=1) {a=0; for(i=3;i<=NF;i++) a+=$i; for(i=3;i<=NF;i++) b[i]+=$i/a*100;}}END{for(i=3;i<=NF;i++) print b[i]/(NR-1)}'
	head -n -2 $1/reuse.event.out | awk '{if(NR!=1) {a=0; for(i=3;i<=NF;i++) a+=$i; for(i=3;i<=NF;i++) printf("%s ", $i/a*100); printf("\n")}}'
elif [ "$mode" = "app" ]; then
	head -1 $1/reuse.app.out | awk '{for(i=3;i<=NF;i++) a+=$i; for(i=3;i<=NF;i++) print $i/a*100}'
else
	echo "mode?"
	exit
fi

