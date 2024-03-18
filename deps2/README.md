Third party dependencies
===
* `patches/`: patch files
* `build_deps.sh`: build third party script, execute this script to build.
* `download_deps.sh`: script for downloading third party archives.
* `vars.sh`: environment variables and third party library name and download link management.


How to build
===
Highly recommend execute this build script on ubuntu:22.04 docker container.

* Step1: Build Requirements(Or use the [Dockerfile](../Dockerfile)):
```
# os libs
apt install -y software-properties-common linux-tools-common linux-tools-generic

# common tools
apt install -y net-tools vim git build-essential cmake wget ninja-build

# clang-14
apt update && apt install -y clang-14 clang-format-14 clang-tidy-14
update-alternatives --install /usr/bin/clang clang /usr/bin/clang-14 100
update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-14 100


# java
apt install -y openjdk-19-jdk maven

# common libs
apt install -y zlib1g-dev libssl-dev
```

* Step2: Execute build script
```
./build_deps.sh
```
By default, thrid party libraries will be installed on `/opt/install`.

Notice: You'll need to have root priviledge.


Pre-built Releass
===
To reduce the time it takes to compile third-party software packages, we have placed pre-compiled third-party packages at the following link:
https://github.com/Husky-GDB/cpp3rdlib/releases
Please note that the pre-compiled software above can only work on **Ubuntu:22.04 amd64 architecture**.




