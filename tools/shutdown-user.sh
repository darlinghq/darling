echo "Seeing if Darling is currently running"

PID=`pgrep launchd`

while [[ ! -z $PID ]]
do
	THISPID=$(echo $PID | head -n1)
	RUNNING_USER=$(ps -o uname= -p $THISPID | head -n1)
	echo "Darling currently running for $RUNNING_USER, shutting it down..."
	sudo -u $RUNNING_USER darling shutdown
	sleep 2
	PID=`pgrep launchd`
done

echo "No instances running now"
