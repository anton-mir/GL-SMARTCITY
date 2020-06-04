pkill gpsfake
pkill cloud_hub
pkill -9 obdgpslogger
pkill cluster-app
pkill sleep
kill $(cat logs/log_ps_id 2> /dev/null) 2> /dev/null
echo "AirC stopped"
