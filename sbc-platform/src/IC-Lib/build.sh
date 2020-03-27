#!/bin/bash

LIBCLUSTER_PATH=$(pwd)
LIBCLUSTER_BUILD_DIR=${LIBCLUSTER_PATH}/build
INSTALL_LIB_CLUSTER=false
CMAKE_OPTS=".."
ARM64CMAKE=${LIBCLUSTER_PATH}/nvidia_arm64.cmake
release_not_specified=true

for i in "$@"
do
        case $i in
        --clean)
                echo 'Lib cluster cleaning ...'
                rm -rf ${LIBCLUSTER_BUILD_DIR}
                exit 0
        ;;
        --install)
                echo 'Install process included'
                echo 'Sudo required'
                INSTALL_LIB_CLUSTER=true
        ;;
        --arm64)
        		echo 'Amr64 native build ...'
		        CMAKE_OPTS="\
        			-DCMAKE_TOOLCHAIN_FILE=nvidia_arm64.cmake \
		             ${CMAKE_OPTS}"
        ;;
        --release)
                echo 'Release build'
                CMAKE_OPTS="\
                        -DCMAKE_BUILD_TYPE=RELEASE \
                        ${CMAKE_OPTS}"
                release_not_specified=false
        ;;
        *)
                echo "Undefined options"

        esac
done

mkdir -p ${LIBCLUSTER_BUILD_DIR}
cd ${LIBCLUSTER_BUILD_DIR}


if [ ${release_not_specified} = true ]
then
        echo "Debug build by default"
        CMAKE_OPTS="\
                -DCMAKE_BUILD_TYPE=Debug \
                ${CMAKE_OPTS}"
fi

echo ${CMAKE_OPTS}

cmake ${CMAKE_OPTS}
make VERBOSE=1

if [ ${INSTALL_LIB_CLUSTER} = true ]
then
        sudo make install
        sudo ldconfig
fi

