# Smart Boyarka City Platform

This is Global Logic Ukraine educational project. 

Copyright

Description

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
libssl-dev libcanberra-gtk* python-gps libjansson-dev libftdi-dev \
fluid libjsoncpp-dev libfltk1.3* fftw3 libgflags-dev
```

#### Install Cmake 3.16+
```
$ cmake --version
```
If you have Cmake version < 3.16:
```
$ sudo apt remove cmake
$ cd ~ && wget https://github.com/Kitware/CMake/releases/download/v3.17.0-rc1/cmake-3.17.0-rc1.tar.gz
$ tar xvzf cmake-3.17.0-rc1.tar.gz && cd cmake-3.17.0-rc1
$ ./bootstrap
$ make -j"$(nproc)"
$ sudo make install && sudo ldconfig -v
```

Alternatvely you can install Cmake with snap
```
$ sudo snap install cmake --channel=3.16/stable --classic
```

### Add your ssh key to github if needed

Use ssh-keygen to generate keys if you don't have them in ~/.ssh/
```
$ ssh-keygen (press enter for all prompts)
$ cat ~/.ssh/id_rsa.pub
```
Copy key from terminal to github.com Settings->SSH ang GPG keys->New SSH key

#### Clone sbc-platform project

Clone *sbc-platform* from github.com to your home directory:
```
$ mkdir -p ~/src && cd ~/src
$ git clone git@github.com:GlobalLogicEdu/GL-SMARTCITY.git
```

Init submodules

```
$ cd ~/src/GL-SMARTCITY/sbc-platform && git submodule update --init
```

#### Install Gtest
```
$ cd ~/src/GL-SMARTCITY/sbc-platform/lib/gtest && mkdir -p build && cd build
$ cmake ..
$ make -j"$(nproc)"
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

Check do you have installed gpsd and it's version:

```
$ apt list --installed | grep gpsd
$ gpsd -V
```

If the version is higher then 2.95 - you need to uninstall it because our 
implementation works only with old API.

How to uninstall:

```
$ sudo apt-get remove gpsd && sudo apt-get autoremove
```

How to install old version of gpsd library from source:
 
```
$ cd ~/src/GL-SMARTCITY/sbc-platform/lib/gpsd
$ ./autogen.sh && ./configure
$ make -j"$(nproc)"
$ sudo make install && sudo ldconfig -v
```
After install - close terminal, then open new terminal window and check gpsd version:

```
$ gpsd -V
```
You should see *gpsd: 2.95*

### Build pits/xlgyro for GL starterkit

The aim of this project is to build demo project and firmware for
STM32F4DISCOVERY development board. It works together with GL
starterkit.

Before continue with this manual, please read file **README.md** in
_~/src/GL-SMARTCITY/sbc-platform/src/pits/xlgyro_ for building and flashing instructions.

### Setup CloudHub

In order to be able to publish data and see the results on the map, you need to setup 
data transfer to the Google cloud account, prepared for you by project staff or optionally
you can setup setup your own cloud. 

If you want to use defaut course account, please edit this file 
~/src/GL-SMARTCITY/sbc-platform/src/CloudHub/conf/config.json and fill there "registry_id"
and "project_id" fields with AirC project default settings (insert your registry id instead 
of "registryId" and your project id instead of "project-id-123456").
You can get this IDs from project course staff. Also staff will give you sequre keys 
that you will need to place into ~/src/GL-SMARTCITY/sbc-platform/src/CloudHub/certs folder. 

For more detailed information (or if you want to setup your personal Google cloud)
please refer to Google Cloud setup manual specially prepared by AirC project staff.

### Build SBC and all components

```
$ cd ~/src/GL-SMARTCITY/sbc-platform 
$ ./build.sh 
```
**build.sh** options: 
* _--pits_ Build for pits on road detection (working with GL Starterkit)
* _--clean_ Remove previous build

**build.sh** build debug version without pits detection by default. 
If you want to process data from STM32F4DISCOVERY use *--pits* option.

# How to RUN ALL  

For running each app use separate terminal tabs. If you don't run pits demo
with STM32F4DISCOVERY, skip step 1 and step 2, start with step 3. 


### 1 Run xlgyro

Build, flash STM32F4DISCOVERY check (**README.md** in 
_~/src/GL-SMARTCITY/sbc-platform/src/pits/xlgyro_) and then:
* Connect GL Starterkit USART3 TX port to your UART/USB converters' RX port
* Connect GL Starterkit USART3 GND pin to your UART/USB converters' GND pin
* Insert your UART/USB converter to USB port of your PC 

### 2 Run xlgyrod

Open new terminal tab and run:

```
$ cd ~/src/GL-SMARTCITY/sbc-platform/build/src/pits/xlgyrod
$ ./xlgyrod
```

Press reset button at STM32F4DISCOVERY board.

You should see incoming signals from STM32F4DISCOVERY.

### 3 Run gpsfake

Open new terminal tab and run:

```
$ gpsfake -c 1 ~/src/GL-SMARTCITY/sbc-platform/car_path_simulation.nmea
``` 

Gpsfake will start car movement simulation according the path saved in 
**car_path_simulation.nmea**. If you want, you can generate another path file 
using this website https://nmeagen.org  

### 4 Run sbc-car

Open new terminal tab and run:

```
$ cd ~/src/GL-SMARTCITY/sbc-platform/build/src/sbc-car/bin && ./obdgpslogger
```

### 5 Run IC-Lib

Open new terminal tab and run:

```
$ cd ~/src/GL-SMARTCITY/sbc-platform/build/src/IC-Lib && ./cluster-app conf/config.json
```

### 6 Run CloudHub

Open new terminal tab and run:

```
$ cd ~/src/GL-SMARTCITY/sbc-platform/build/src/CloudHub && ./cloud_hub conf/config.json
```

## Check result

[Open Google Cloud](https://reflected-flame-250912.firebaseapp.com)

The car should be driving along the path. 

When the pit is detected with GL Starterkit board, an marker should appear at
the map near the place where the car was at this moment.
