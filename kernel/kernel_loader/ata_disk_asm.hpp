#ifndef INCLUDE_ATA_DISK_ASM_H
#define INCLUDE_ATA_DISK_ASM_H

#include <cstdint>    

extern "C" {
    /** ata_lba_read: 
     *  ATA read sectors (LBA)
     *
     *  @param lba LBA of the sector to read
     *  @param n number of sectors to read
     *  @param 
     */
    void ata_lba_read(uint32_t lba, uint8_t n, void* destination);

}

#endif /* INCLUDE_ATA_DISK_ASM_H */
    