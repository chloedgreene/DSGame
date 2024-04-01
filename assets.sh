#!/bin/bash

OBJ2DL=/opt/wonderful/thirdparty/blocksds/external/nitro-engine/tools/obj2dl/obj2dl.py

#Remove old Assets
rm -f -- ntrfs/blahaj_model.bin
rm -f -- ntrfs/maps/*.obj


#Convert Assets Into DS assets Manually
python3 $OBJ2DL \
    --input assets/exports/blahaj.obj \
    --output ntrfs/blahaj_model.bin \
    --texture 256 256

python3 $OBJ2DL \
    --input maps/*.obj \
    --output ntrfs/maps/*.bin \
    --texture 256 256
