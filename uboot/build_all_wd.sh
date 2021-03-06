#!/bin/bash
SCRIPTDIR=$PWD
TARGETDIR=$PWD/out
DVRBOOTEXE=$SCRIPTDIR/examples/flash_writer_u/dvrboot.exe.bin
DVRBOOTDIR=$SCRIPTDIR/examples/flash_writer_u


if [ $# == 0 ]
then
    echo "Error, missing argument"
    echo "Usage: $0 [pelican|monarch|all]"
    exit
fi

if [ -d $PWM/out ]
then
    rm -rf $PWM/out
fi

#
mkdir -p $PWD/out

# Monarch
#
if [ ! "$1" = "pelican" ] ;then
    make distclean;make wd_monarch_config;make clean;make
    cp $DVRBOOTEXE      $TARGETDIR/wd_monarch.uboot32.fb.dvrboot.exe.bin
    echo "making factory checksum image..."
    pushd examples/flash_writer_u/
    cat Bind/hwsetting_header.bin Bind/hwsetting.bin Bind/uboot.bin Bind/fsbl.bin Bind/bl31.bin Bind/uboot64.bin > $TARGETDIR/monarch_uboot_checksum.bin
    popd
fi
# Pelican
#

if [ ! "$1" = "monarch" ]; then
    make distclean;make wd_pelican_config;make clean;make
    cp $DVRBOOTEXE      $TARGETDIR/wd_pelican.uboot32.fb.dvrboot.exe.bin
    echo "making factory checksum image..."
    pushd examples/flash_writer_u/
    ./pelican_uboot_checksum.sh
    cp bootcode_checksum.bin $TARGETDIR/pelican_bootcode_checksum.bin
    cp uboot64_checksum.bin $TARGETDIR/pelican_uboot64_checksum.bin
    popd
fi

ls -l $TARGETDIR/129?_spi_*.bin $TARGETDIR/129?_emmc_*.bin $TARGETDIR/wd_*.bin

pushd examples/flash_writer_u/
if [ ! "$1" = "pelican" ] ;then
./boottar_gen_monarch.sh
cp ./uboot_bins_monarch/uboot.tar $TARGETDIR/wd_monarch_uboot.tar
fi

if [ ! "$1" = "monarch" ] ;then
./boottar_gen_pelican.sh
cp ./uboot_bins_pelican/uboot.tar $TARGETDIR/wd_pelican_uboot.tar
fi

popd

