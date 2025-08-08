#!/bin/sh

# exit if any command fails
set -e

if [ ! "$1" = "/dev/sda" ] ; then

    CARD_DEV=$1

    if [ ! -b "$CARD_DEV" ] ; then
        echo $CARD_DEV is not a block device
        exit 1
    fi

    if [ -b "$CARD_DEV" ] ; then
        dd if=/dev/zero of=$CARD_DEV bs=1024 count=1024
        SIZE=`fdisk -l $CARD_DEV | grep Disk | awk '{print $5}'`

        echo DISK SIZE - $SIZE bytes

        CYLINDERS=$(($SIZE/255/63/512))

        echo CYLINDERS - $CYLINDERS

        {
            echo 63,$((($SIZE/512)-63)),0x0C,*,,,-
        } | sfdisk $CARD_DEV

       mkfs.vfat -F 32 -n "BOOT" ${CARD_DEV}1
# No ext2 rootfs
#       mke2fs -j -L "rootfs" ${CARD_DEV}2
    fi

# Copy-pasted from google
#    # so hopefully this will abort e.g. if we're about to blow away the root filesystem
#    for mounted in $(findmnt -o source | grep "^$CARD_DEV") ; do
#        umount "$mounted"
#    done

#    echo "*************** Formatting SD card... ***************"
#    wipefs --all $CARD_DEV
#    {
#        echo 63,144585,0x0C,* echo 144585,,,-
#    } | sfdisk $CARD_DEV
#
#    mkfs.vfat -F 32 -n "boot" ${CARD_DEV}1
#    mke2fs -j -L "filesystem" ${CARD_DEV}2
fi
