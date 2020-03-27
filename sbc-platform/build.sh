#!/bin/bash

BUILD_PATH=$(pwd)
BUILD_DIR=${BUILD_PATH}/build
CMAKE_OPTS=".."
release_not_specified=true

for i in "$@"
do
        case $i in
        --clean)
                echo 'Cleaning ...'
                rm -rf ${BUILD_DIR}
                rm -rf ${BUILD_PATH}/src/sbc-car/bin
                exit 0
        ;;
        --pits)
                echo 'Build for pits on road detection'
                CMAKE_OPTS="\
                        -DCMAKE_XLGYRO_OPTION=XLGYRO_ENABLED \
                        ${CMAKE_OPTS}"
        ;;
        --static)
                echo "Build static library"
                CMAKE_OPTS="\
                        -DBUILD_SHARED_LIBS=OFF \
                        ${CMAKE_OPTS}"
        ;;
        --release)
                echo "Build for release"
                release_not_specified=false
        ;;
        *)
                echo "Undefined options"
        esac
done

mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR} || exit

if [ ${release_not_specified} = true ]
then
        echo "Debug build by default"
        CMAKE_OPTS="\
                -DCMAKE_BUILD_TYPE=Debug \
                ${CMAKE_OPTS}"
fi

echo ${CMAKE_OPTS}

cmake ${CMAKE_OPTS}

make VERBOSE=1 -j"$(nproc)"

cp -r ../src/sbc-car/bin/ ./src/sbc-car/

exit 0