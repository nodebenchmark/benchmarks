HOME= ###YOUR HOME DIRECTORY HERE###
NODE_HOME=$HOME/node

app=$1

cd $app
eval $(../nodebench -n $NODE_HOME/node -b $app --server-only) &

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

pid=`ps ux | grep "${NODE_HOME}/node" | grep $app | awk '{if(NR==1) print $2}'`
kill -INT $pid
sleep 3
