#!/bin/bash
echo "Compiling Kernel"
if [[ "$1" == "debug" ]]; then
    make debug
    if [[ $? -eq 0 ]]; then
        echo "Run debug emulator"    
        qemu-system-x86_64 -drive format=raw,file="bin/os.img",index=0 -m 4096M -s -S
    fi
else
    make
    if [[ $? -eq 0 ]]; then
        echo "Run emulator"    
        qemu-system-x86_64 -drive format=raw,file="bin/os.img",index=0, -m 4096M
    fi
fi
