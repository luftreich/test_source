#!/bin/bash
#CROSS_TOOLCHAINS_DIR=/usr/local/bin/hndtools-arm-linux-2.6.36-uclibc-4.5.3
export ROOT=`pwd`
CROSS_TOOLCHAINS_DIR=/home/vincent/Vanilla_Latte/Board/trunk/mining_amlogic/amlogic_toolchain/arm-linux-amlogic-gnueabihf
PATH=$PATH:/opt/arm-linux-amlogic-gnueabihf/usr/bin/

TOOLCHAIN_ROOT='/opt'
export STAGING_DIR=$TOOLCHAIN_ROOT/staging_dir/target-arm_cortex-a5+neon_eglibc-2.19_eabi
CROSS_TOOLCHAINS_DIR=$TOOLCHAIN_ROOT/staging_dir/toolchain-arm_cortex-a5+neon_gcc-4.8-linaro_eglibc-2.19_eabi
export PATH=$PATH:${CROSS_TOOLCHAINS_DIR}/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CROSS_TOOLCHAINS_DIR/lib:$ROOT/build/lib
export NAND_FLASH_SUPPORT=1
