cat $1 |
awk '
	{
		if(NR==1)
		{
			tot = ($6+$7+$8+$9+$10+$11+$12+$13+$14+$15+$16)/100;
			#print "tot\t"tot;
			#print "mem_read\t"$2"\t"$2/tot;
			#print "mem_write\t"$3"\t"$3/tot;
			#print "direct_br\t"$4"\t"$4/tot;
			#print "indirect_br\t"$5"\t"$5/tot;
			#print "syscall\t"$6"\t"$6/tot;
			#print "ctrl\t"$7"\t"$7/tot;
			#print "int\t"$8"\t"$8/tot;
			#print "fp\t"$9"\t"$9/tot;
			#print "stack\t"$10"\t"$10/tot;
			#print "shift\t"$11"\t"$11/tot;
			#print "stringop\t"$12"\t"$12/tot;
			#print "sse\t"$13"\t"$13/tot;
			#print "others\t"$14+$15+$16"\t"($14+$15+$16)/tot

			print $4/$7*100, $5/$7*100;
			#print $7/tot, $8/tot, $10/tot, $9/tot, $13/tot;
			#print $1, $5/$8, $6/$8;
		}
	}
'
