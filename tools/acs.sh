cat $1/bblacs.out |
awk 'BEGIN
	{
		FS=":";
	}
	{
		if(NR!=1)
		{
			for(int i = 8;i <= 64;i+=8)
			{
				gap = $1 - b;
				if(gap > 32 * 1024 || gap < 32 * 1024) t++;
			}
		}
		a=$1;
		b=$2;
	}
	END
	{
		print t;
	}'
