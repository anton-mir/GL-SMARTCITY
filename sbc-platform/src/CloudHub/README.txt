SBC Cloud hub
---------------------------------------------------------------------------------------------------
1. Configuration parameters
name:	mqtt_msg_parallel_instances
descr:	number of maximimum store mqtt data child processes(maximim parallel store car count)

name:	mqtt_msg_operation_timeout
descr:	mqtt communication operation timeout (if child process not store data into this time window MqttMsgManager kill child process to free slot for a next car object)

name:	data_processos_group_intervel
descr:	interval for group data contain 2 field: sec - seconds and msec milliseconds

name:	data_processor_store_every_pkt
descr:	cloud hub can dprop some packets from group if packets incomming too often. If You wish store every packet, set this parameter to 1.
	If you wish to drop some packets, set wich every packet will be append to group.
	Exmaple: data_processor_store_every_pkt=3, only every 3 packet will be stored to cloud

name:	port
descr:	udp listening port

name:	mqtt_group_param
descr: this section contain mqtt depended parameters

name:	mqtt_group_param->registry_id
descr:	cloud registry id

name:	mqtt_group_param->project_id
descr: cloud project id

name:	mqtt_group_param->root_cert
descr:	cloud root certificate

name:	mqtt_group_param->region
descr:	cloud region

name:	mqtt_group_param->objects
descr:	obects, that hub will be recognize. For each object you should set per object parameters.


Per object parameters

name:	mqtt_group_param->objects->mqtt_id
descr:	MQTT object id, that registered into cloud

name:	mqtt_group_param->objects->sbc_id
descr:	SBC object id (this id should correspont to Emulation Server id and/or SmartCityIC id)

name:	mqtt_group_param->objects->key_path
descr:	every mqtt object have own private key(public key registered into cloud)

name:	mqtt_group_param->objects->key_algorytm
descr:	mqtt object key generateded algorythm


---------------------------------------------------------------------------------------------------
2. Application architecture

New packet -> UdpServer -> ClusterDataProcessor(groupping) -> MqttMsgManager(run store data instance) -> MqttMsgInstance (store group data) -> Packet Group into cloud -> END

---------------------------------------------------------------------------------------------------
3. Required library
paho.mqtt.c
jansson
jwt
openssl

---------------------------------------------------------------------------------------------------
Build, configure and enjoy :)
