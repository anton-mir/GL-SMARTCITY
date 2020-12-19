#!/bin/bash

# trap ctrl-c and call ctrl_c()
trap ctrl_c INT

function ctrl_c() {
        echo " *** Trapped CTRL-C, exitting and stopping airc..."
	sudo ../stop_airc.sh
}

#sh stop_airc.sh > /dev/null
mkdir -p logs

#echo -en "Running GPS simulation...\n"
#bash -c 'gpsfake -c 1 ~/src/GL-SMARTCITY/sbc-platform/car_path_simulation.nmea &' >> logs/gpsfake.log 2>> logs/gpsfake.err
#sleep 1

echo -en "\nRunning obdgpslogger...\n"
bash -c 'cd ~/src/GL-SMARTCITY/sbc-platform/build/src/sbc-car/bin && ./obdgpslogger -x box --airc-device box --airc-box-ip 192.168.1.121 &' >> logs/obdgpslogger.log 2>> logs/obdgpslogger.err
sleep 3

echo -en "\nRunning car cluster...\n"
bash -c 'cd ~/src/GL-SMARTCITY/sbc-platform/build/src/IC-Lib && ./cluster-app conf/config.json &' >> logs/cluster-app.log 2>> logs/cluster-app.err
sleep 5

echo -en "\nRunning CloudHub...\n\n"
bash -c 'cd ~/src/GL-SMARTCITY/sbc-platform/build/src/CloudHub && ./cloud_hub conf/config.json &' >> logs/cloud_hub.log 2>> logs/cloud_hub.err
sleep 3

cd logs || exit
bash -c 'while true; do tail -10 cloud_hub.log > cloud_hub_.log; cat cloud_hub_.log > cloud_hub.log; tail -10 cluster-app.err > cluster-app_.err; cat cluster-app_.err > cluster-app.err; tail -10 obdgpslogger.log > obdgpslogger_.log; cat obdgpslogger_.log > obdgpslogger.log; sleep 300; done;' &
echo $! > log_ps_id

echo -en "!!!! ============>> FOR EXIT PLEASE PRESS Ctrl-C <<============= !!!!\n\n"

tail -f cloud_hub.log
