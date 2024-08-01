#include "linkedlist_allocator.hpp"
#include "ata_disk.hpp"
#include "gdt.hpp"
#include "fat32.hpp"
#include "paging.hpp"

kstd::ata_disk getAtaDisk(){
    return kstd::ata_disk{};
}

kstd::istream_offset<kstd::ata_disk> getAtaDiskWithOffset(std::size_t offset){
    return kstd::istream_offset<kstd::ata_disk>{getAtaDisk(), offset};
}

extern "C" int kernel_loader_main(){

    init_allocator();

    // Read header info
    GDTReader<kstd::ata_disk> reader(getAtaDisk());
    
    auto* mbr = reader.getProtectiveMBRHeader();

    for (int i = 0; i < 4; i++)
    {
        auto* partition = &mbr->partitions[i];
        if(reader.mbrPartitionIsUsed(mbr->partitions[i])){
            Fat32Reader<kstd::istream_offset<kstd::ata_disk>> fat32(getAtaDiskWithOffset(partition->lba_of_first_sector * ATA_SECTOR_SIZE));

            auto* kernelBin = fat32.findKernelBin();
            
            fat32.readFileContent(kernelBin, +[](uint8_t* buf, uint32_t offset, uint32_t size){
                //Load into physical memory location at 0x200000 but will page to 0xC0100000
                void* kernelLoc = (void*) 0x200000;
                memcpy(kernelLoc + offset, buf, size);
            });

            break;
        }
    }

    setup_paging();


    return 0;
}