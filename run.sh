#!/bin/bash

echo "Compiling Kernel"
make

if [ $? -eq 0 ]; then
    echo "Run emulator"
    qemu-system-x86_64 -drive format=raw,file="bin/os.bin",index=0,if=floppy,  -m 128M
fi
