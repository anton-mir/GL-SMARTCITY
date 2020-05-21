# What's new?

Two json sets of data were created: For vehicle and for stable device. 
 - Json for vehicle now includes 4 more fields in substructure "state" for air quality data.
 - In json for stable box were excluded all fields that are related to vehicle. Information about location was excluded from "state" substructure and included to father's structure, so that now information about location updates only once, not every second. 
 - Following files were changed:
```
/sbc-platform/src/sbc-car/src/logger/main.c - call of "generate_json" function
/sbc-platform/src/sbc-car/src/logger/networking.c - call of "generate_json" function
/sbc-platform/src/sbc-car/src/logger/sender.c - definition of "generate_json" function
/sbc-platform/src/sbc-car/src/logger/sender.h - declaration of "generate_json" function
/sbc-platform/src/IC-Lib/libcluster/cluster.cpp - definition of "state_parse" function and cluster_state struct
```

The user can choose a car or a box object will be tracked. By default there is used <car> without the air quality test, however you can add argument ```box``` or ```car``` and you will see also the quality of the air when you run sbc-car module.

Example:
```
cd ~/src/GL-SMARTCITY/sbc-platform/build/src/sbc-car/bin && ./obdgpslogger
```
or

```
cd ~/src/GL-SMARTCITY/sbc-platform/build/src/sbc-car/bin && ./obdgpslogger -d box
```
or

```
cd ~/src/GL-SMARTCITY/sbc-platform/build/src/sbc-car/bin && ./obdgpslogger -d car
```
The next file was changed:
```
/sbc-platform/src/sbc-car/src/logger/main.c
```



