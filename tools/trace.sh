# based on visaddr.out dump the instruction trace as well as their function name
# the output is the input of sched_cache for simulating events reordering

paste <(cat $1/visaddr.out | awk '{print $1, $2}') <(cat $1/visaddr.out | awk '{split($3, temp, "."); if(temp[1]=="") print $3; else print temp[1]}' | c++filt -p | awk 'BEGIN{FS="::"}{print $1}') -d ' ' | awk '{if($1=="event") print $0; else if($3=="invalid_rtn") print $1, $2}' > $1/trace.in
