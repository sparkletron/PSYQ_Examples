#!/usr/bin/env bash

echo "Creating ISO image for PlayStation Emulator"

echo "Copying files to tmp directory"

mkdir /tmp/data

cp -r ../MAIN.exe /tmp/data
cp -r ../TIM	  /tmp/data
cp -r ../CDROM    /tmp/data
cp -r ~/.wine/drive_c/psyq/cdgen/LCNSFILE /tmp

echo "Generating unlicensed image file"

echo -e 'D:\r cd "tmp/data/CDROM/"\r BUILDCD.EXE -l -iTEXTURE.IMG TEXTURE.CTI\r exitemu\r' | dosemu -dumb

echo "Generating licensed ISO file"

stripisowin.exe s 2352 /tmp/data/CDROM/TEXTURE.IMG /tmp/data/CDROM/TEXTURE.ISO

PSXLICENSE.exe /us /i /tmp/data/CDROM/TEXTURE.ISO

echo "Moving ISO to bin_psx directory"

mkdir ../IMAGE

cp /tmp/data/CDROM/TEXTURE.ISO ../IMAGE

echo "Cleaning Up files"

rm -rf /tmp/data
rm -rf /tmp/LCNSFILE

