#!/bin/bash
echo "Compiling Kernel"
if [[ $1 -eq "debug" ]]; then
    make debug
    if [[ $? -eq 0 ]]; then
        echo "Run debug emulator"    
        # exec qemu-system-x86_64 -drive format=raw,file="bin/os.bin",index=0,if=floppy, -m 128M -s -S
        qemu-system-i386 -drive format=raw,file="bin/os.bin",index=0,if=floppy, -m 128M -s -S
    fi
else
    make
    if [[ $? -eq 0 ]]; then
        echo "Run emulator"    
        # qemu-system-x86_64 -drive format=raw,file="bin/os.bin",index=0,if=floppy,  -m 128M
        qemu-system-i386 -drive format=raw,file="bin/os.bin",index=0,if=floppy, -m 128M
    fi
fi
