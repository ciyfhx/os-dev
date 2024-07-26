CPP_SOURCES = $(wildcard kernel/*.cpp)
HEADERS = $(wildcard kernel/*.hpp)
# file extension replacement from cpp to o
OBJS = ${CPP_SOURCES:.cpp=.o}
# replacement of parent directory 'kernel' to 'bin' 
OBJS_PATH = $(patsubst kernel/%,bin/%,$(OBJS))

ASM_SOURCES = $(wildcard kernel/kernel_asm/*.asm)
ASM_OBJS = ${ASM_SOURCES:.asm=.o}
ASM_OBJS_PATH = $(patsubst kernel/kernel_asm/%,bin/%,$(ASM_OBJS))

CC = /usr/local/i386elfgcc/bin/i386-elf-gcc
LD = /usr/local/i386elfgcc/bin/i386-elf-ld
CXX = /usr/local/i386elfgcc/bin/i386-elf-g++
GDB = /usr/local/i386elfgcc/bin/i386-elf-gdb

# -g: debug flag -m32: 32bit object file
CPPFLAGS = -g -m32 -fvar-tracking -std=c++26 -B/usr/local/i386elfgcc/bin/
LDFLAGS = -melf_i386 

BIN_DIR=./bin

${BIN_DIR}/os.img: ${BIN_DIR}/boot.bin ${BIN_DIR}/full_kernel.o
# create a zeros initialised disk file of 32MB
	dd if=/dev/zero of=$@ bs=512 count=131072
# partition the disk file
	parted $@ mklabel msdos
	parted $@ mkpart primary fat32 1MiB 32MiB
	parted $@ set 1 boot on
	mformat -i $@@@1M -h 255 -s 63 -F ::
	dd if=$< of=$@ conv=notrunc bs=1M seek=1
	mcopy -i $@@@1M $(word 2,$^) "::kernel.bin"
# mkfs.fat -F 32 -n "ZI" $@
# override the first 512 bytes with the boot loader
# dd if=$< of=$@ conv=notrunc
# copy the kernel binary into the file format
#mcopy -i $@ "./tools/fat32.cpp" "::kernel.bin"

${BIN_DIR}/full_kernel.o: ${BIN_DIR}/kernel_entry.o ${ASM_OBJS_PATH} ${OBJS_PATH} 
	${LD} ${LDFLAGS} -o $@ -Ttext 0x8C00 $^ --oformat binary

${BIN_DIR}/%.o: kernel/%.cpp ${HEADERS}
	${CXX} ${CPPFLAGS} -ffreestanding  -c $< -o $@

${BIN_DIR}/%.o: kernel/kernel_asm/%.asm
	nasm $< -f elf -o $@

${BIN_DIR}/%.o: boot/%.asm
	nasm $< -f elf -o $@

${BIN_DIR}/%.bin: boot/%.asm
	nasm $< -f bin -o $@

${BIN_DIR}/full_kernel.elf: ${BIN_DIR}/kernel_entry.o ${ASM_OBJS_PATH} ${OBJS_PATH}
	${LD} ${LDFLAGS} -o $@ -Ttext 0x8C00 $^ 


debug: ${BIN_DIR}/os.img ${BIN_DIR}/full_kernel.elf


clean:
	rm -rf ${BIN_DIR}/*.o ${BIN_DIR}/*.bin ${BIN_DIR}/*.elf

.PHONY: clean all