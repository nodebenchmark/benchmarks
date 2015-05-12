#!/bin/bash

# dumping the instruction address to function name mapping
# input files are mapping.out, which is directly dumped by InstFootprint pintool
paste <(cat $1/mapping.out | awk '{print $1}') <(cat $1/mapping.out | awk '{split($2, temp, "."); if(temp[1]=="") print $2; else print temp[1]}' | c++filt | awk 'BEGIN{FS="::"}{a=split($1, temp, " "); print temp[a]}') > $1/mapping.out.final
exit

# dump the static instruction counts for each kernel
cat $1/mapping.out.final |
awk '
{
	if($2 == "node")
	{
		node++;
	}
	else if($2 == "v8")
	{
		v8++;
	}
	else if($2 == "invalid_rtn")
	{
		native++;
	}
	else
	{
		other++;
	}
}
END {
	print "Static footprint:"
	print "v8: "v8/NR*100;
	print "node: "node/NR*100;
	print "native: "native/NR*100;
	print "others: "other/NR*100;
}'
exit

# collecting the percentage of misses from each kernels
# first dismangle the c++ functions
# input files are funchm.prefetch.out or funchm.out
paste <(cat $1/funchm.prefetch.out | awk '{split($1, temp, "."); if(temp[1]=="") print $1; else print temp[1]}' | c++filt | awk 'BEGIN{FS="::"}{a=split($1, temp, " "); print temp[a]}') <(cat $1/funchm.prefetch.out | awk '{print $2"\t"$3}') |
awk '
{
	if($1 == "node")
	{
		node_hit += $2;
		node_miss += $3;
	}
	else if($1 == "v8")
	{
		v8_hit += $2;
		v8_miss += $3;
	}
	else if($1 == "invalid_rtn")
	{
		native_hit += $2;
		native_miss += $3;
	}
	else
	{
		other_hit += $2;
		other_miss += $3;
	}
}
END {
	tot_miss = node_miss + v8_miss + native_miss + other_miss;
	print "Miss distributions:"
	print "v8: "v8_miss/tot_miss*100;
	print "node: "node_miss/tot_miss*100;
	print "native: "native_miss/tot_miss*100;
	print "others: "other_miss/tot_miss*100;
}
'
exit

