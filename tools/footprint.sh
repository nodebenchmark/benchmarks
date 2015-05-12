head -n -1 $1/footprint.out | awk 'BEGIN{FS=","}{if(NR!=1) {for(i=1;i<=NF-3;i++) a[i]+=$i/$(NF-2)*100;}}END{for(i=1;i<=NF-3;i++) print a[i]/(NR-1);}'
#tail -1 $1/footprint.out | awk 'BEGIN{FS=","}{for(i=1;i<=NF-3;i++) print $i/$(NF-2)*100;}'
