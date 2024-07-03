#!/bin/bash

set -e

export DEPS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ ! -f ${DEPS_DIR}/vars.sh ]; then
    echo "vars.sh is missing".
    exit 1
fi
. ${DEPS_DIR}/vars.sh

mkdir -p ${DEPS_SOURCE_DIR}

sha256sum_func() {
    local FILENAME=$1
    local DESC_DIR=$2
    local SHA256SUM=$3

    sha256=`sha256sum "${DESC_DIR}/${FILENAME}"`
    if [ "$sha256" != "${SHA256SUM}  ${DESC_DIR}/${FILENAME}" ]; then
        echo "${DESC_DIR}/${FILENAME} sha256sum check failed!"
        echo -e "expect-sha256 ${SHA256SUM} \nactual-sha256 $sha256"
        return 1
    fi


    return 0
}


download_func() {
    local FILENAME=$1
    local DOWNLOAD_URL=$2
    local DESC_DIR=$3
    local SHA256SUM=$4

    if [ -z "$FILENAME" ]; then
        echo "Error: No file name specified to download"
        exit 1
    fi
    if [ -z "$DOWNLOAD_URL" ]; then
        echo "Error: No download url specified for $FILENAME $DOWNLOAD_URL"
        exit 1
    fi
    if [ -z "$DESC_DIR" ]; then
        echo "Error: No dest dir specified for $FILENAME"
        exit 1
    fi


    FAIL=1
    for attemp in 1 2; do
        if [ -r "$DESC_DIR/$FILENAME" ]; then
            if sha256sum_func $FILENAME $DESC_DIR $SHA256SUM; then
                echo "Archive $FILENAME already exist."
                FAIL=0
                break;
            fi
            echo "Archive $FILENAME will be removed and download again."
            rm -f "$DESC_DIR/$FILENAME"
        else
            echo "Downloading $FILENAME from $DOWNLOAD_URL to $DESC_DIR"
            wget --no-check-certificate $DOWNLOAD_URL -O $DESC_DIR/$FILENAME
            if [ "$?"x == "0"x ]; then
                if sha256sum_func $FILENAME $DESC_DIR $SHA256SUM; then
                    FAIL=0
                    echo "Success to download $FILENAME"
                    break;
                fi
                echo "Archive $FILENAME will be removed and download again."
                rm -f "$DESC_DIR/$FILENAME"
            else
                echo "Failed to download $FILENAME. attemp: $attemp"
            fi
        fi
    done

    if [ $FAIL -ne 0 ]; then
        echo "Failed to download $FILENAME"
    fi
    return $FAIL
}



########
######## Download All Deps
########

# download deps archives
echo "===== Downloading dep archives..."
echo ${DEPS_ARCHIVES_DOWNLOAD}
for DEP_ARCH in ${DEPS_ARCHIVES_DOWNLOAD[*]}
do
    NAME=$DEP_ARCH"_NAME"
    SHA256SUM=$DEP_ARCH"_SHA256"
    if test "x$REPOSITORY_URL" = x; then
        URL=$DEP_ARCH"_DOWNLOAD"
        download_func ${!NAME} ${!URL} $DEPS_SOURCE_DIR ${!SHA256SUM}
        if [ "$?"x != "0"x ]; then
            echo "Failed to download ${!NAME}"
            exit 1
        fi
    else
        URL="${REPOSITORY_URL}/${!NAME}"
        download_func ${!NAME} ${URL} $DEPS_SOURCE_DIR ${!SHA256SUM}
        if [ "$?x" != "0x" ]; then
            #try to download from home
            URL=$DEP_ARCH"_DOWNLOAD"
            download_func ${!NAME} ${!URL} $DEPS_SOURCE_DIR ${!SHA256SUM}
            if [ "$?x" != "0x" ]; then
                echo "Failed to download ${!NAME}"
                exit 1 # download failed again exit.
            fi
        fi
    fi
done
echo "===== Downloading deps archives...done"

# check if all deps archives exists
echo "===== Checking all deps archives..."
for DEP_ARCH in ${DEPS_ARCHIVES_DOWNLOAD[*]}
do
    NAME=$DEP_ARCH"_NAME"
    if [ ! -r $DEPS_SOURCE_DIR/${!NAME} ]; then
        echo "Failed to fetch ${!NAME}"
        exit 1
    fi
done
echo "===== Checking all deps archives...done"


# unpacking thirdpart archives
echo "===== Unpacking all deps archives..."
TAR_CMD="tar"
UNZIP_CMD="unzip"
SUFFIX_TGZ="\.(tar\.gz|tgz)$"
SUFFIX_XZ="\.tar\.xz$"
SUFFIX_ZIP="\.zip$"
SUFFIX_BZ2="\.bz2$"
# temporary directory for unpacking
# package is unpacked in tmp_dir and then renamed.
mkdir -p $DEPS_SOURCE_DIR/tmp_dir

for DEP_ARCH in ${DEPS_ARCHIVES_DOWNLOAD[*]}
do
    NAME=$DEP_ARCH"_NAME"
    SOURCE=$DEP_ARCH"_SOURCE"

    if [ -z "${!SOURCE}" ]; then
        continue
    fi

    if [ ! -d $DEPS_SOURCE_DIR/${!SOURCE} ]; then
        if [[ "${!NAME}" =~ $SUFFIX_TGZ  ]]; then
            echo "$DEPS_SOURCE_DIR/${!NAME}"
            echo "$DEPS_SOURCE_DIR/${!SOURCE}"
            if ! $TAR_CMD xzf "$DEPS_SOURCE_DIR/${!NAME}" -C $DEPS_SOURCE_DIR/tmp_dir; then
                echo "Failed to untar ${!NAME}"
                exit 1
            fi
        elif [[ "${!NAME}" =~ $SUFFIX_XZ ]]; then
            echo "$DEPS_SOURCE_DIR/${!NAME}"
            echo "$DEPS_SOURCE_DIR/${!SOURCE}"
            if ! $TAR_CMD xJf "$DEPS_SOURCE_DIR/${!NAME}" -C $DEPS_SOURCE_DIR/tmp_dir; then
                echo "Failed to untar ${!NAME}"
                exit 1
            fi
        elif [[ "${!NAME}" =~ $SUFFIX_ZIP ]]; then
            if ! $UNZIP_CMD "$DEPS_SOURCE_DIR/${!NAME}" -d $DEPS_SOURCE_DIR/tmp_dir; then
                echo "Failed to unzip ${!NAME}"
                exit 1
            fi
        elif [[ "${!NAME}" =~ $SUFFIX_BZ2 ]]; then
            echo "$DEPS_SOURCE_DIR/${!NAME}"
            echo "$DEPS_SOURCE_DIR/${!SOURCE}"
            if ! $TAR_CMD jxvf "$DEPS_SOURCE_DIR/${!NAME}" -C $DEPS_SOURCE_DIR/tmp_dir; then
                echo "Failed to untar ${!NAME}"
                exit 1
            fi
        else
            echo "nothing has been done with ${!NAME}"
            continue
        fi
        mv $DEPS_SOURCE_DIR/tmp_dir/* $DEPS_SOURCE_DIR/${!SOURCE}
    else
        echo "${!SOURCE} already unpacked."
    fi
done
rm -r ${DEPS_SOURCE_DIR}/tmp_dir
echo "===== Unpacking all deps archives...done"




echo "===== Patching deps..."

PATCHED_MARK="patched_mark"

# folly patch
cd ${DEPS_SOURCE_DIR}/${FOLLY_SOURCE}
if [ ! -f $PATCHED_MARK ] && [ $FOLLY_SOURCE == "folly-2023.02.20.00" ]; then
    patch -p1 < ${DEPS_PATCH_DIR}/folly-2023.02.20.00-deps.patch
    touch $PATCHED_MARK
fi
cd -
echo "Finished patching ${FOLLY_SOURCE}"

echo "===== Patching deps...done"
