# Smart Boyarka City Platform

This project is the part of Global Logic educational proof of concept projects (POCs). 

# Copyright

GlobalLogic Ukraine 2021

GlobalLogic Sweden 2024

# Short Description

Smart Boyarka City Platform consists of a few modules providing the possibility of data collection from the smart devices, processing data gathered, and transferring the data to the Cloud, where it might be stored and displayed.

In the scope of this project, the AirC device was developed and created. The AirC device measures air quality and thus plays the role of the data source device in the Smart City infrastructure.

You can find more info in [docs folder](/docs/readme.md)

## Dependencies

Prefferable OS version is Ubuntu 20.04.6 LTS 

# Software installation

If you want to install all software manually, follow next steps.
It is possible to use Virtual Box for this project. 

#### Install software dependencies
```
$ sudo apt-get install autoconf automake autotools-dev build-essential doxygen \
freeglut3 freeglut3-dev gcc git libagg-dev libcairo2-dev libfreetype6-dev \
libmarisa-dev libpangocairo-1.0-0 libpango1.0-dev libprotobuf-dev \
libqt5svg5-dev libtool libxml2-dev make pkg-config protobuf-compiler \
subversion libgoogle-glog-dev qtbase5-dev libqt5gui5 qttools5-dev-tools \
qttools5-dev qtdeclarative5-dev qtlocation5-dev qtpositioning5-dev \
qt5-default qml-module-qtquick-* qml-module-qtgraphicaleffects \
qml-module-qtpositioning libboost-all-dev libcurl4-openssl-dev \
libssl-dev libcanberra-gtk* gpsd-clients libjansson-dev libftdi-dev \
fluid libjsoncpp-dev libfltk1.3* fftw3 libgflags-dev libncurses5-dev libtinfo-dev \
scons python-gi python-gi-cairo g++ python-gobject libgtk-3-dev pps-tools
```

#### Install Cmake 3.16+
```
$ cmake --version
```
If you have Cmake version < 3.16:
```
$ sudo snap install cmake --channel=3.16/stable --classic
```
Alternatvely you can install Cmake manually
```
$ sudo apt remove cmake
$ cd ~ && wget https://github.com/Kitware/CMake/releases/download/v3.17.0-rc1/cmake-3.17.0-rc1.tar.gz
$ tar xvzf cmake-3.17.0-rc1.tar.gz && cd cmake-3.17.0-rc1
$ ./bootstrap
$ make -j"$(nproc)"
$ sudo make install && sudo ldconfig -v
```

### Add your ssh key to github if needed

Use ssh-keygen to generate keys if you don't have them in ~/.ssh/
```
$ ssh-keygen -t ecdsa (press enter for all prompts)
$ cat ~/.ssh/id_ecdsa.pub
```
Copy key from terminal to github.com Settings->"SSH ang GPG keys"->"New SSH key"

#### Clone sbc-platform project

Clone *sbc-platform* from github.com to your home directory:
```
$ mkdir -p ~/src && cd ~/src
$ git clone git@github.com:anton-mir/GL-SMARTCITY.git
```

Init submodules

```
$ cd ~/src/GL-SMARTCITY/sbc-platform && git submodule update --init
```

#### Install Gtest
```
$ cd ~/src/GL-SMARTCITY/sbc-platform/lib/gtest && mkdir -p build && cd build
$ cmake .. && make -j"$(nproc)"
$ sudo make install && sudo ldconfig -v
```

### Build and install lib jansson, jwt, paho.mqtt.c

```
$ cd ~/src/GL-SMARTCITY/sbc-platform/lib/paho.mqtt.c/ && mkdir -p build && cd build
$ cmake -DBUILD_SHARED_LIBS=ON -DJANSSON_BUILD_SHARED_LIBS=ON \
-DPAHO_WITH_SSL=TRUE -DPAHO_BUILD_DOCUMENTATION=FALSE \
-DPAHO_BUILD_SAMPLES=FALSE ..
$ make -j"$(nproc)"
$ sudo make install && sudo ldconfig -v
```

### Build and install gpsd

Check do you have installed gpsd version 3.20:

```
$ gpsd -V
```

If you have other version, please uninstall it and install V3.20 instead:

```
$ sudo apt-get remove gpsd && sudo apt-get autoremove
```

Install gpsd V3.20 from source:
 
```
$ cd ~/src/GL-SMARTCITY/sbc-platform/lib/gpsd
$ git checkout release-3.20 
$ scons && scons check && sudo scons udev-install
```

After install - close terminal, then open new terminal window and check gpsd version:

```
$ gpsd -V
```
You should see *gpsd: 3.20 (revision 3.20)*

# Software variants

In the project use following hardware: STM32F4DISCOVERY + GlobalLogic Starter Kit board.
There are few configurations supported. Please build software needed and flash it according to the following instructions in corresponding submodules, and after that continue to build and run the whole project.

***1. Pits detection with xlgyro and GL Starter Kit***

The aim of this project is to build demo project and firmware for project aimed on detecting the pits on the roads and marking them on the Cloud Map. Needs to be build like that:
```
./build.sh --pits (with CMAKE_XLGYRO_OPTION=XLGYRO_ENABLED)
```
To get more information about this project, for building and flashing instructions, please read [info from xlgyro submodule](/src/pits/xlgyro/README.md).

***2. AirC project for detecting an air quality***

To build firmware for AirC device which also use STM32F4DISCOVERY + GlobalLogic Starter Kit board please check documentation and building instructions in [airc-device submodule](src/airc-device/docs). The schematics of the AirC device sensor board is available in [src/airc-device/pcb](src/airc-device/pcb) folder.

AirC device sensor board has ESP-01 WiFi module integrated which is used for updating AirC sensor board configuration (ESP-01 has default AT-commands based firmware). More info about ESP-01 setup and how to use it available in documentation here [src/airc-device/docs](src/airc-device/docs).

# Setup Google Cloud

In order to be able to publish data to the cloud and observe the results on the map, you need to setup 
data transfer to the Google cloud account, prepared beforehand for you by project staff or optionally
you can setup setup your own cloud. For setting cloud part manually check documentation in [/docs](/docs) folder. 

You need to edit this file ~/src/GL-SMARTCITY/sbc-platform/src/CloudHub/conf/config.json 
and fill there "registry_id" and "project_id" fields with your project default settings 
(insert your registry id instead of "registryId" and your project id instead of "project-id-123456"). 

# Build SBC project and all related components

```
$ cd ~/src/GL-SMARTCITY/sbc-platform 
$ ./build.sh 
```
**build.sh** options: 
* _--pits_ Build for pits on road detection (see above)
* _--clean_ Remove previous build

**build.sh** by default builds project configured for AirC.

Please check the AirC dataflow diagram here [/docs/AirC schema.odt](/docs)  

# How to run procject configured for AirC

You can run all apps one-by-one or use script to run all automatically (see one section below).

### 1 Run gpsfake

Don't run fakeGPS if the real GPS sensor is connected to the Linux node.


Note: This step is optional, because AirC may work even without real/fake GPS.


Open new terminal tab and run:

```
$ gpsfake -c 1 ~/src/GL-SMARTCITY/sbc-platform/car_path_simulation.nmea
``` 

Gpsfake will start car movement simulation according the path saved in 
**car_path_simulation.nmea**. If you want, you can generate another path file 
using this website https://nmeagen.org  

### 2 Run sbc-car (obdgpslogger)

Open new terminal tab and run (change IP to yours airc-box device IP):

```
cd ~/src/GL-SMARTCITY/sbc-platform/build/src/sbc-car/bin && ./obdgpslogger --airc-device box --airc-box-ip 192.168.88.116
```

### 3 Run IC-Lib

Open new terminal tab and run:

```
$ cd ~/src/GL-SMARTCITY/sbc-platform/build/src/IC-Lib && ./cluster-app conf/config.json
```

### 4 Run CloudHub

Open new terminal tab and run:

```
$ cd ~/src/GL-SMARTCITY/sbc-platform/build/src/CloudHub && ./cloud_hub conf/config.json
```

## Run all together

Optionally, if you don't want to open many windows, you can run apps as daemons in 
foreground and monitor only CloudHub logs:

```
$ cd ~/src/GL-SMARTCITY/sbc-platform
$ ./run_airc.sh
```

To stop use *Ctrl-C* or *stop_aitc.sh* script.

Log files located in *~/src/GL-SMARTCITY/sbc-platform/logs* folder.

## Check result

Open corresponding Google Cloud app at using your set up beforehand account.

You should see marker with AirC Device air parameters data appearing along the path. 

