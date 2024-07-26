#include <iostream>
#include <fstream>
#include <cstdint>
#include <string.h>
#include <locale>
#include <codecvt>
#include <functional>
#include <stdexcept>

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
    __uint128_t guid;
    uint64_t starting_lba_for_partition_entries;
    uint32_t number_of_partition_entries;
    uint32_t size_of_partition_entry;
    uint32_t crc32_of_partition_entries;
    //Reversed zero till end of sector
} __attribute__((packed)) GPT;

typedef struct {
    __uint128_t partition_type_guid;
    __uint128_t unique_partition_guid;
    uint64_t first_lba;
    uint64_t last_lba; //inclusive
    uint64_t attribute_flags;
    char  partition_name[72];
} __attribute__((packed)) GPTPartitionEntry;

class GPTReader {
private:
    MBR protective_mbr;
    GPT gpt;
    const GPTPartitionEntry* partitionEntries;
    std::ifstream infile;

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
    GPTReader(std::string file): infile(file, std::fstream::in | std::fstream::binary){
        if(infile.fail()){
            std::cerr << "Unable to locate file" << std::endl;
        }
        infile.read((char*)&protective_mbr, sizeof(MBR));
        
        for (size_t i = 0; i < 4; i++)
        {
            MBRPartitionEntry& entry = protective_mbr.partitions[i];
            if(mbrPartitionIsUsed(entry)){
                std::cout << "Partition :" << i << std::endl;
                std::cout << "Partition starting LBA: " << entry.lba_of_first_sector << std::endl;

                //Check for GPT header
                if(entry.partition_type == 0xEE){
                    uint32_t gpt_lba = entry.lba_of_first_sector;
                    infile.seekg(gpt_lba * SECTOR_SIZE);
                    infile.read((char*)&gpt, sizeof(GPT));
                    std::cout << "GPT Partition Entries starting LBA: " << gpt.starting_lba_for_partition_entries << std::endl;

                    // Read partition entries
                    partitionEntries = new GPTPartitionEntry[gpt.number_of_partition_entries];
                    infile.seekg(gpt.starting_lba_for_partition_entries * SECTOR_SIZE);
                    infile.read((char*)partitionEntries, sizeof(GPTPartitionEntry) * gpt.number_of_partition_entries);
                    

                }

            }
        }


        
    }

    ~GPTReader(){
        infile.close();
    }

    inline bool mbrPartitionIsUsed(const MBRPartitionEntry& entry){
        return entry.status != 0 || isValidCHS(entry.first_sector) || entry.partition_type != 0 || isValidCHS(entry.last_sector) || entry.lba_of_first_sector != 0 ||
             entry.no_of_sectors != 0;
    }


    const MBR* getProtectiveMBRHeader(){
        return &protective_mbr;
    }

    void readMBRPartition(const MBRPartitionEntry& entry, std::function<void(uint8_t*, uint32_t)> fn){
        uint8_t* tmpBuffer = new uint8_t[SECTOR_SIZE];
        infile.seekg(entry.lba_of_first_sector * SECTOR_SIZE);
        uint32_t i = 0;
        while(i < entry.no_of_sectors){
            infile.read((char*)tmpBuffer, SECTOR_SIZE);

            if(!infile.fail()){
                fn(tmpBuffer, SECTOR_SIZE);
            }else throw std::runtime_error("Error reading partition!");

            i++;
        }
    }

    const GPTPartitionEntry* getGPTPartitionEntry(uint32_t index) const{
        return &partitionEntries[index];
    }
    const GPT* getGPTHeader() const{
        return &gpt;
    }

    inline bool gptPartitionIsUsed(const GPTPartitionEntry& entry){
        return entry.partition_type_guid != 0 || entry.unique_partition_guid != 0;
    }

    void readGPTPartition(const GPTPartitionEntry& entry, std::function<void(uint8_t*, uint32_t)> fn){
        uint8_t* tmpBuffer = new uint8_t[SECTOR_SIZE];
        infile.seekg(entry.first_lba * SECTOR_SIZE);
        uint32_t currentLba = entry.first_lba;
        while(currentLba <= entry.last_lba){
            infile.read((char*)tmpBuffer, SECTOR_SIZE);

            if(!infile.fail()){
                fn(tmpBuffer, SECTOR_SIZE);
            }else throw std::runtime_error("Error reading partition!");

            currentLba++;
        }
    }

};

#define GPT_FORMAT 0
#define MBR_FORMAT 1

int main(int argc, char* argv[]){
    GPTReader reader("/home/ciyfhx/os/bin/os.img");
#if GPT_FORMAT == 1
    const GPT* gpt = reader.getGPTHeader();
    for (size_t i = 0; i < gpt->number_of_partition_entries; i++)
    {
        const GPTPartitionEntry* gptEntry = reader.getGPTPartitionEntry(i);
        if(reader.gptPartitionIsUsed(*gptEntry)){
            std::cout << "GPT Partition :" << i << std::endl;

            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> conversion;
            std::string partitionName = conversion.to_bytes( (char16_t*)gptEntry->partition_name);

            // std::cout << "Partition Type GUID: " << gptEntry.partition_type_guid << std::endl;
            std::cout << "Partition Name: " << partitionName << std::endl;
            std::cout << "Partition starting LBA: 0x" << std::hex << gptEntry->first_lba << std::endl;
            std::cout << "Partition ending LBA: 0x" << std::hex << gptEntry->last_lba << std::endl;

            if(partitionName == "Main Data Partition"){
                std::cout << "Extracting Data Partition: " << partitionName << std::endl;
                std::fstream outfile("data_partition.bin", std::fstream::out | std::fstream::binary);
                reader.readGPTPartition(*gptEntry, [&](uint8_t* buf, uint32_t size){
                    outfile.write((char*)buf, size);
                });
            }

        }
    }
#elif MBR_FORMAT == 1
    
    auto mbr = reader.getProtectiveMBRHeader();

    for (int i = 0; i < 4; i++)
    {
        auto partition = mbr->partitions[i];
        if(reader.mbrPartitionIsUsed(mbr->partitions[i])){
           std::cout << "MBR Partition :" << i << std::endl;
           std::cout << "Extracting Partition..." << std::endl;
           std::fstream outfile("data_partition.bin", std::fstream::out | std::fstream::binary);
                reader.readMBRPartition(partition, [&](uint8_t* buf, uint32_t size){
                    outfile.write((char*)buf, size);
            });
            break;
        }
    }

#endif
    return 0;
}