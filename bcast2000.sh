#!/bin/sh

BCASTDIR=/usr/local/bcast

export LD_LIBRARY_PATH=$BCASTDIR:$LD_LIBRARY_PATH
export PATH=$BCASTDIR:$PATH
exec aoss $BCASTDIR/bcast2000 $@
