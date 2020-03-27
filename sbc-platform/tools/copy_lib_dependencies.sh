#!/bin/bash 
# Author : Serhii Sokolov
# Email : serhii[dot]sokolov[at]globallogic[dot]com
# License : GNU GPLv3
#

function usage()
{
    cat << EOU
Usage: bash $0 <path to the binary> <path to the output folder> 
EOU
exit 1
}

#Validate the inputs
[[ $# < 2 ]] && usage

#Check if the paths are vaild
[[ ! -e $1 ]] && echo "Not a vaild input $1" && exit 1 

if [[ $# == 3 ]]
then
    silent_mode=true
else
    silent_mode=false
fi

CUR_PATH=$(dirname "${BASH_SOURCE[0]}")
OUTPUT_PATH="$2"
SCRIPT_PATH=${CUR_PATH}/$(basename "$0")
parentdir="$(dirname ${OUTPUT_PATH})";

[[ $silent_mode != true ]] && [[ -e "${parentdir}/lib64" ]] && rm -f ${parentdir}/lib64
[[ $silent_mode != true ]] && [[ -e "${parentdir}/x86_64" ]] && rm -f ${parentdir}/x86_64

#Get the library dependencies
[[ $silent_mode != true ]] && echo "Collecting the shared library dependencies for $1..."
deps=$(ldd "$1" | cut -d'>' -f2 | awk '{print $1}')

[[ $silent_mode != true ]] && echo "Copying library dependencies to ${OUTPUT_PATH}"

mkdir -p ${OUTPUT_PATH}

for lib in $deps ; do
    if [ -f "$lib" ] ; then
        libname=${lib##*/}
        if [ -f "${OUTPUT_PATH}/${libname}" ]
        then
            continue
        fi
        cp "$lib" "${OUTPUT_PATH}/" 
        ${SCRIPT_PATH} ${lib} ${OUTPUT_PATH} true
    fi  
done

[[ $silent_mode != true ]] && ln -sf ${OUTPUT_PATH} ${parentdir}/lib64 
[[ $silent_mode != true ]] && ln -sf ${OUTPUT_PATH} ${parentdir}/x86_64

[[ $silent_mode != true ]] && echo && echo "Done!"

