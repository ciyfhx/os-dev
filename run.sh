#!/bin/bash
echo "Compiling Kernel"
if [[ "$1" == "debug" ]]; then
    make debug
    if [[ $? -eq 0 ]]; then
        echo "Run debug emulator"    
        qemu-system-i386 -drive format=raw,file="bin/os.bin",index=0,if=floppy, -m 128M -s -S
    fi
else
    make
    if [[ $? -eq 0 ]]; then
        echo "Run emulator"    
        qemu-system-i386 -drive format=raw,file="bin/os.bin",index=0,if=floppy, -m 128M
    fi
fi
