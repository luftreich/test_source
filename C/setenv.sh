#!/bin/bash
#CROSS_TOOLCHAINS_DIR=/usr/local/bin/hndtools-arm-linux-2.6.36-uclibc-4.5.3
export ROOT=`pwd`
CROSS_TOOLCHAINS_DIR=/home/vincent/Vanilla_Latte/Board/trunk/mining_amlogic/amlogic_toolchain/arm-linux-amlogic-gnueabihf
PATH=$PATH:~/samba/Vanilla_Latte/Board/trunk/mining_amlogic/amlogic_toolchain/arm-linux-amlogic-gnueabihf/usr/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CROSS_TOOLCHAINS_DIR/lib:$ROOT/build/lib
export NAND_FLASH_SUPPORT=1
