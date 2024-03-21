#!/bin/bash

set -e

export DEPS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ ! -f ${DEPS_DIR}/vars.sh ]; then
    echo "vars.sh is missing".
    exit 1
fi
. ${DEPS_DIR}/vars.sh

mkdir -p ${DEPS_SOURCE_DIR}

clone_func() {
    local REPO=$1
    local TAG=$2
    local DEST_DIR=$3
    #echo "REPO: ${REPO}"

    if [ -z "$REPO" ]; then
        echo "Error: No repo specified to clone"
        exit 1
    fi
    if [ -z "$TAG" ]; then
        echo "Error: No tag specified for $REPO"
        exit 1
    fi
    echo "$DEST_DIR"
    if [ -z "$DEST_DIR" ]; then
        echo "Error: No dest dir specified for $REPO"
        exit 1
    fi


    FAIL=1
    for attemp in 1 2; do
        if [ -r "$DEST_DIR" ]; then
            echo "Archive $FILENAME already exist."
            FAIL=0
            break;
        else
            echo "Cloning from $REPO to $DEST_DIR"
            git clone -b $TAG $REPO $DEST_DIR
            if [ "$?"x == "0"x ]; then
                FAIL=0
                echo "Success to clone $REPO"
            else 
                echo "Failed to clone $REPO"
            fi
        fi
    done

    if [ $FAIL -ne 0 ]; then
        echo "Failed to download $REPO"
    fi
    return $FAIL
}

echo "===== Cloning deps archives...."
echo ${DEPS_ARCHIVES_CLONE[*]}
for DEP_ARCH in ${DEPS_ARCHIVES_CLONE[*]}
do
    REPO=$DEP_ARCH"_REPO"
    TAG=$DEP_ARCH"_TAG"
    DEP_DIR=$DEP_ARCH"_SOURCE"
    
    DEST_DIR=${DEPS_SOURCE_DIR}/${!DEP_DIR}
    clone_func ${!REPO} ${!TAG} ${DEST_DIR}
done
echo "===== Cloning deps archives...done"

# check if all deps archives exists
echo "===== Checking all deps archives..."
for DEP_ARCH in ${DEPS_ARCHIVES_CLONE[*]}
do
    NAME=$DEP_ARCH"_NAME"
    if [ ! -r $DEPS_SOURCE_DIR/${!NAME} ]; then
        echo "Failed to fetch ${!NAME}"
        exit 1
    fi
done
echo "===== Checking all deps archives...done"
