# Kernel
KERNEL_CPP_SOURCES = $(wildcard kernel/*.cpp)
KERNEL_CPP_HEADERS = $(wildcard kernel/*.hpp)
# file extension replacement from cpp to o
KERNEL_CPP_OBJS = ${KERNEL_CPP_SOURCES:.cpp=.o}
# replacement of parent directory 'kernel' to 'bin' 
KERNEL_CPP_OBJS_PATH = $(patsubst kernel/%,bin/kernel/%,$(KERNEL_CPP_OBJS))

KERNEL_ASM_SOURCES = $(wildcard kernel/kernel_asm/*.asm)
KERNEL_ASM_OBJS = ${KERNEL_ASM_SOURCES:.asm=.o}
KERNEL_ASM_OBJS_PATH = $(patsubst kernel/kernel_asm/%,bin/kernel/%,$(KERNEL_ASM_OBJS))

# Kernel Loader
KERNEL_LOADER_CPP_SOURCES = $(wildcard kernel/kernel_loader/*.cpp)
KERNEL_LOADER_CPP_HEADERS = $(wildcard kernel/kernel_loader/*.hpp)
KERNEL_LOADER_CPP_OBJS = ${KERNEL_LOADER_CPP_SOURCES:.cpp=.o}
KERNEL_LOADER_CPP_OBJS_PATH = $(patsubst kernel/kernel_loader/%,bin/kernel_loader/%,$(KERNEL_LOADER_CPP_OBJS))

KERNEL_LOADER_ASM_SOURCES = $(wildcard kernel/kernel_loader/*.asm)
KERNEL_LOADER_ASM_OBJS = ${KERNEL_LOADER_ASM_SOURCES:.asm=.o}
KERNEL_LOADER_ASM_OBJS_PATH = $(patsubst kernel/kernel_loader/%,bin/kernel_loader/%,$(KERNEL_LOADER_ASM_OBJS))

CC = /usr/local/x86_64elfgcc/bin/x86_64-elf-gcc
LD = /usr/local/x86_64elfgcc/bin/x86_64-elf-ld
CXX = /usr/local/x86_64elfgcc/bin/x86_64-elf-g++
GDB = /usr/local/x86_64elfgcc/bin/x86_64-elf-gdb

# -g: debug flag -m32: 32bit object file
CPPFLAGS = -g -fvar-tracking -std=c++26 -B/usr/local/x86_64elfgcc/bin/ -fno-PIE -fpermissive -fno-exceptions -fno-rtti
LDFLAGS = -melf_x86_64

BIN_DIR=./bin

${BIN_DIR}/os.img: ${BIN_DIR}/boot.bin ${BIN_DIR}/kernel_loader.bin ${BIN_DIR}/kernel.bin
# create a zeros initialised disk file of 32MB
	dd if=/dev/zero of=$@ bs=512 count=131072
# partition the disk file
	parted $@ mklabel msdos
	parted $@ mkpart primary fat32 1MiB 32MiB
	parted $@ set 1 boot on
	mformat -i $@@@1M -h 255 -s 63 -F ::
	dd if=$< of=$@ conv=notrunc bs=1M seek=1
	mcopy -i $@@@1M $(word 2,$^) "::loader.bin"
	mcopy -i $@@@1M $(word 3,$^) "::kernel.bin"
# mkfs.fat -F 32 -n "ZI" $@
# override the first 512 bytes with the boot loader
# dd if=$< of=$@ conv=notrunc
# copy the kernel binary into the file format
#mcopy -i $@ "./tools/fat32.cpp" "::kernel.bin"

${BIN_DIR}/kernel_loader.bin: ${BIN_DIR}/kernel_loader/kernel_loader.o ${KERNEL_LOADER_ASM_OBJS_PATH} ${KERNEL_LOADER_CPP_OBJS_PATH}
	${LD} -melf_i386 -o $@ -Ttext 0x8C00 $^ --oformat binary

${BIN_DIR}/kernel.bin: ${BIN_DIR}/kernel/kernel_entry.o ${KERNEL_ASM_OBJS_PATH} ${KERNEL_CPP_OBJS_PATH}
	${LD} ${LDFLAGS} -o $@ -Ttext 0xD100000 $^ --oformat binary

${BIN_DIR}/kernel/%.o: kernel/%.cpp ${HEADERS}
	${CXX} ${CPPFLAGS} -m64 -ffreestanding  -c $< -o $@

${BIN_DIR}/kernel/%.o: kernel/kernel_asm/%.asm
	nasm $< -f elf64 -o $@

${BIN_DIR}/kernel_loader/%.o: kernel/kernel_loader/%.asm
	nasm $< -f elf -o $@

${BIN_DIR}/kernel_loader/%.o: kernel/kernel_loader/%.cpp ${HEADERS}
	${CXX} ${CPPFLAGS} -m32 -ffreestanding  -c $< -o $@

${BIN_DIR}/%.o: boot/%.asm
	nasm $< -f elf -o $@

${BIN_DIR}/%.bin: boot/%.asm
	nasm $< -f bin -o $@

${BIN_DIR}/kernel_loader.elf: ${BIN_DIR}/kernel_loader/kernel_loader.o ${KERNEL_LOADER_ASM_OBJS_PATH} ${KERNEL_LOADER_CPP_OBJS_PATH}
	${LD} -melf_i386 -o $@ -Ttext 0x8C00 $^

${BIN_DIR}/kernel.elf: ${BIN_DIR}/kernel/kernel_entry.o ${KERNEL_ASM_OBJS_PATH} ${KERNEL_CPP_OBJS_PATH}
	${LD} ${LDFLAGS} -o $@ -Ttext 0xD100000 $^ 


debug: ${BIN_DIR}/os.img ${BIN_DIR}/kernel.elf ${BIN_DIR}/kernel_loader.elf


clean:
	rm -rf ${BIN_DIR}/*.o ${BIN_DIR}/*.bin ${BIN_DIR}/*.elf ${BIN_DIR}/*.img
	rm -rf ${BIN_DIR}/kernel/*.o ${BIN_DIR}/*.bin ${BIN_DIR}/*.elf ${BIN_DIR}/*.img
	rm -rf ${BIN_DIR}/kernel_loader/*.o ${BIN_DIR}/*.bin ${BIN_DIR}/*.elf ${BIN_DIR}/*.img

.PHONY: clean all