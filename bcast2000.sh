#!/bin/sh

BCASTDIR=/usr/local/bcast

if [ -x /usr/bin/aoss ]; then
OSS_EMU=aoss
fi
if [ -x /usr/bin/padsp ]; then
OSS_EMU=padsp
fi

V4L_EMU=`find /usr -name v4l1compat.so`

#echo $OSS_EMU
#echo $V4L_EMU

export LD_LIBRARY_PATH=$BCASTDIR:$LD_LIBRARY_PATH
export PATH=$BCASTDIR:$PATH
LD_PRELOAD=$V4L_EMU $OSS_EMU $BCASTDIR/bcast2000 $@
