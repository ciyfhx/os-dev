#include "ata_disk.hpp"

using namespace kstd;

void ata_disk::ignore(uint32_t n) {
    this->seekg(pos + n);
}

void ata_disk::seekg(uint32_t pos){
    this->pos = pos;
}

uint8_t ata_disk::fail(){
    return this->err;
}

void ata_disk::read(char* buf, uint32_t size){
    if(size == 0) {
        err = 1; // size cannot be zero
        return;
    }

    uint32_t startiongPos = pos;
    uint32_t endingPos = startiongPos + size;

    uint32_t startingLba = startiongPos / ATA_SECTOR_SIZE;
    uint32_t endingLba = endingPos / ATA_SECTOR_SIZE;

    uint32_t sectorsToRead = endingLba - startingLba + 1;

    uint32_t startingOffsetBytes = startiongPos % ATA_SECTOR_SIZE;
    uint32_t startingRemainingBytes = ATA_SECTOR_SIZE - startingOffsetBytes;
    uint32_t endingRemainingBytes = endingPos % ATA_SECTOR_SIZE;

    if(startingLba == endingLba){
        ata_lba_read(startingLba, 1, this->tbuf);
        memcpy(buf, &this->tbuf[startingOffsetBytes], size);
        ignore(size);
        return;
    }

    uint32_t curLba = startingLba;
    uint32_t written = 0;
    while(curLba <= endingLba){
        ata_lba_read(curLba, 1, this->tbuf);
        if(curLba == startingLba){
            memcpy(buf, &this->tbuf[startingOffsetBytes], startingRemainingBytes);
            written += startingRemainingBytes;
        }else if(curLba == endingLba){
            memcpy(buf + written, this->tbuf, endingRemainingBytes);
            written += endingRemainingBytes;
        }else{
            memcpy(buf + written, &this->tbuf, ATA_SECTOR_SIZE);
            written += ATA_SECTOR_SIZE;
        }
        curLba++;
    }
    ignore(written);

    // uint32_t remainingBytesToRead = size % ATA_SECTOR_SIZE;
    // if(remainingBytesToRead != 0) sectorsToRead++;
    // uint32_t i = sectorsToRead;
    // uint32_t offset = (sectorsToRead - i);
    // while(i > 0){
    //     ata_lba_read(lba + offset, 1, this->tbuf);
    //     auto dest = buf + (ATA_SECTOR_SIZE * offset);
    //     if(i == 0){
    //         memcpy(dest, this->tbuf, remainingBytesToRead);
    //     }else memcpy(dest + , this->tbuf, ATA_SECTOR_SIZE);
    //     i--;
    // }

}

