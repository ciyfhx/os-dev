#ifndef INCLUDE_GDT_H
#define INCLUDE_GDT_H

#include <cstdint>
#include <functional>
#include "stream.hpp"
#include "mem_allocator.hpp"

using gdt_callback = void (*) (uint8_t*, uint32_t); 

#define SECTOR_SIZE 512

typedef struct {
    uint8_t head;
    uint8_t sector_and_high_cyclinder;
    uint8_t low_cyclinder;
} __attribute__((packed)) CHS;

typedef struct {
    uint8_t status;
    CHS first_sector;
    uint8_t partition_type;
    CHS last_sector;
    uint32_t lba_of_first_sector;
    uint32_t no_of_sectors;
} __attribute__((packed)) MBRPartitionEntry;

typedef struct {
    uint8_t bootcode[446];
    MBRPartitionEntry partitions[4];
    uint16_t boot_signature;
} __attribute__((packed)) MBR;

typedef struct {
    uint64_t signature; // must be  45h 46h 49h 20h 50h 41h 52h 54h
    uint32_t revision_number;
    uint32_t header_size;
    uint32_t crc32_of_header;
    uint32_t reserved; // must be zero
    uint64_t current_header_lba;
    uint64_t backup_header_lba;
    uint64_t first_useable_lba_for_partition;
    uint64_t last_useable_lba_for_partition;
    uint64_t guid_higher;
    uint64_t guid_lower;
    uint64_t starting_lba_for_partition_entries;
    uint32_t number_of_partition_entries;
    uint32_t size_of_partition_entry;
    uint32_t crc32_of_partition_entries;
    //Reversed zero till end of sector
} __attribute__((packed)) GDT;

typedef struct {
    uint64_t partition_type_guid_higher;
    uint64_t partition_type_guid_lower;
    uint64_t unique_guid_higher;
    uint64_t unique_guid_lower;
    uint64_t first_lba;
    uint64_t last_lba; //inclusive
    uint64_t attribute_flags;
    char  partition_name[72];
} __attribute__((packed)) GDTPartitionEntry;

template<typename Stream>
requires std::is_base_of_v<kstd::istream, Stream>
class GDTReader {
private:
    MBR protective_mbr;
    GDT gdt;
    const GDTPartitionEntry* partitionEntries;
    Stream stream;

    uint32_t chsToLba(const CHS& chs){
        uint8_t sector = chs.sector_and_high_cyclinder & 0x3F;
        uint8_t high_cyclinder = chs.sector_and_high_cyclinder & 0xC0;
        uint16_t cyclinder = high_cyclinder << 2 | chs.low_cyclinder;
        return (cyclinder * 254 + chs.head) * 63 + (sector - 1); // for 8gb or larger disk
    }
    inline bool isValidCHS(const CHS& chs){
        return chs.head != 0 || chs.sector_and_high_cyclinder != 0 || chs.low_cyclinder != 0;
    }

public:
    template<typename S1>
    requires std::is_base_of_v<kstd::istream, Stream>
    GDTReader(S1&& stream) : stream(std::forward<S1>(stream)){
        // if(stream.fail() == 1){
        //     // std::cerr << "Unable to locate file" << std::endl;
        // }
        stream.read((uint8_t*)&protective_mbr, sizeof(MBR));
        
        for (uint32_t i = 0; i < 4; i++)
        {
            MBRPartitionEntry& entry = protective_mbr.partitions[i];
            if(mbrPartitionIsUsed(entry)){
                // std::cout << "Partition :" << i << std::endl;
                // std::cout << "Partition starting LBA: " << entry.lba_of_first_sector << std::endl;

                //Check for GDT header
                if(entry.partition_type == 0xEE){
                    uint32_t gdt_lba = entry.lba_of_first_sector;
                    stream.seekg(gdt_lba * SECTOR_SIZE);
                    stream.read((uint8_t*)&gdt, sizeof(GDT));
                    // std::cout << "GDT Partition Entries starting LBA: " << gdt.starting_lba_for_partition_entries << std::endl;

                    // Read partition entries
                    partitionEntries = new GDTPartitionEntry[gdt.number_of_partition_entries];
                    // partitionEntries = (GDTPartitionEntry*) kmalloc(gdt.number_of_partition_entries * sizeof(GDTPartitionEntry));
                    stream.seekg(gdt.starting_lba_for_partition_entries * SECTOR_SIZE);
                    stream.read((uint8_t*)partitionEntries, sizeof(GDTPartitionEntry) * gdt.number_of_partition_entries);
                    

                }

            }
        }
    }

    ~GDTReader(){
        delete[] partitionEntries;
        // kfree(partitionEntries);
    }

    inline bool mbrPartitionIsUsed(const MBRPartitionEntry& entry){
        return entry.status != 0 || isValidCHS(entry.first_sector) || entry.partition_type != 0 || isValidCHS(entry.last_sector) || entry.lba_of_first_sector != 0 ||
        entry.no_of_sectors != 0;
    }
    const MBR* getProtectiveMBRHeader() {
        return &protective_mbr;
    }
    void readMBRPartition(const MBRPartitionEntry& entry, gdt_callback fn){
        uint8_t* tmpBuffer = new uint8_t[SECTOR_SIZE];
        stream.seekg(entry.lba_of_first_sector * SECTOR_SIZE);
        uint32_t i = 0;
        while(i < entry.no_of_sectors){
            stream.read((uint8_t*)tmpBuffer, SECTOR_SIZE);

            if(!stream.fail()){
                fn(tmpBuffer, SECTOR_SIZE);
            } else return;//else throw std::runtime_error("Error reading partition!");

            i++;
        }
        delete tmpBuffer;
    }
    const GDTPartitionEntry* getGDTPartitionEntry(uint32_t index) const {
        return &partitionEntries[index];
    }
    const GDT* getGDTHeader() const {
        return &gdt;
    }
    inline bool gdtPartitionIsUsed(const GDTPartitionEntry& entry) {
        return entry.partition_type_guid_higher != 0 || entry.partition_type_guid_lower != 0 || entry.unique_guid_higher != 0  || entry.unique_guid_lower != 0;
    }
    void readGDTPartition(const GDTPartitionEntry& entry, gdt_callback fn) {
        uint8_t* tmpBuffer = new uint8_t[SECTOR_SIZE];
        stream.seekg(entry.first_lba * SECTOR_SIZE);
        uint32_t currentLba = entry.first_lba;
        while(currentLba <= entry.last_lba){
            stream.read((uint8_t*)tmpBuffer, SECTOR_SIZE);

            if(!stream.fail()){
                fn(tmpBuffer, SECTOR_SIZE);
            }else return;//throw std::runtime_error("Error reading partition!");

            currentLba++;
        }
        delete tmpBuffer;
    }

};


#endif /* INCLUDE_GDT_H */
    