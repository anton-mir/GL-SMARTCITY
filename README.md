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


