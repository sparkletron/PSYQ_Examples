#!/usr/bin/env bash

echo "Creating ISO image for PlayStation Emulator"

echo "Copying files to tmp directory"

mkdir /tmp/data

cp -r ../MAIN.exe /tmp/data
cp -r ../TIM	  /tmp/data
cp -r ../cdrom    /tmp/data
cp -r ~/.wine/drive_c/psyq/cdgen/LCNSFILE /tmp

echo "Generating unlicensed image file"

echo -e 'D:\r cd "tmp/data/cdrom/"\r BUILDCD.EXE -l -iTEXTURE.IMG TEXTURE.CTI\r exitemu\r' | dosemu -dumb

echo "Generating licensed ISO file"

stripisowin.exe s 2352 /tmp/data/cdrom/TEXTURE.IMG /tmp/data/cdrom/TEXTURE.ISO

PSXLICENSE.exe /eu /i /tmp/data/cdrom/TEXTURE.ISO

echo "Moving ISO to bin_psx directory"

cp /tmp/data/cdrom/TEXTURE.ISO ../

echo "Cleaning Up files"

rm -rf /tmp/data
rm -rf /tmp/LCNSFILE

