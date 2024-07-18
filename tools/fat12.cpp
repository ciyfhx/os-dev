#include <iostream>
#include <fstream>
#include <cstdint>
#include <string.h>
#include <functional>

typedef struct {
    uint8_t bdb_oem[8];
    uint16_t bdb_bytes_per_sector;
    uint8_t bdb_sectors_per_cluster;
    uint16_t bdb_reserved_sectors;
    uint8_t bdb_fat_count;
    uint16_t bdb_dir_entries_count;
    uint16_t bdb_total_sectors;
    uint8_t bdb_media_descriptor_type;
    uint16_t bdb_sectors_per_fat;
    uint16_t bdb_sectors_per_track;
    uint16_t bdb_heads;
    uint32_t bdb_hidden_sectors;
    uint32_t bdb_large_sector_count;
    uint8_t ebr_drive_number;
    uint8_t reserved;
    uint8_t ebr_signature;
    uint8_t ebr_volume_id[4];
    uint8_t ebr_volume_label[11];
    uint8_t ebr_system_id[8];
} __attribute__((packed)) Fat12Header;

enum Attributes { READ_ONLY=-0x1, HIDDEN=0x2, SYSTEM=0x4, VOLUME_ID=0x08, DIRECTORY=0x10, ARCHIVE=0x20 };

typedef struct {
    uint8_t filename[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_10_ms; // Creation time in hundredths of a second (0-199)
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_accessed_date;
    uint16_t high_16_cluster; // Always zero for fat12
    uint16_t last_modified_time;
    uint16_t last_modified_date;
    uint16_t low_16_cluster;
    uint32_t filesize;
}__attribute__((packed)) DirectoryEntry;

class Fat12Reader {
private:
    Fat12Header header;
    uint8_t* fat;
    uint32_t fatSize;
    const DirectoryEntry* rootDirEntries;
    uint32_t clusterLoc;
    uint32_t clusterSize;
    std::ifstream infile;

    bool readCluster(uint32_t clusterNum, uint8_t* buffer){
        uint32_t readClusterLoc = clusterLoc + clusterSize * (clusterNum - 2);

        infile.seekg(readClusterLoc);
        infile.read((char*)buffer, clusterSize);
        return infile.fail() == 0;
    }

    uint16_t readFat(int16_t lookup){
        uint16_t fatIndex = lookup * 3 / 2;
        if(lookup % 2 == 0)
            return fat[fatIndex] | ((fat[fatIndex + 1] & 0x0F) << 8);
        else
            return fat[fatIndex + 1] << 4 | ((fat[fatIndex] & 0xF0) >> 4);
    }

public:
    Fat12Reader(std::string file): infile(file, std::fstream::in | std::fstream::binary){

        if(infile.fail()){
            std::cerr << "Unable to locate file" << std::endl;
        }

        infile.ignore(3); // skip the reserved bytes
        infile.read((char*)&header, sizeof(Fat12Header));

        infile.seekg(512); // jump to fat
        // read FAT
        uint32_t fatSize = header.bdb_fat_count * header.bdb_sectors_per_fat * header.bdb_bytes_per_sector;
        fat = new uint8_t[fatSize];
        infile.read((char*)fat, fatSize);

        // read root directory
        uint32_t rootDirSize = header.bdb_dir_entries_count * 32;
        rootDirEntries = new DirectoryEntry[header.bdb_dir_entries_count];
        infile.read((char*)rootDirEntries, rootDirSize);

        // cluster info
        clusterSize = header.bdb_bytes_per_sector * header.bdb_sectors_per_cluster;
        clusterLoc = 512 + fatSize + rootDirSize;
    }
    
    ~Fat12Reader(){
        delete[] fat;
        delete[] rootDirEntries;
        infile.close();
    }

    const DirectoryEntry* getRootDirectoryEntry(uint32_t entryNum) const{
        return &this->rootDirEntries[entryNum];
    }

    int32_t getRootDirectoryEntrySize() const {
        return this->header.bdb_dir_entries_count;
    }

    const DirectoryEntry* findKernelBin() {
        uint32_t size = this->getRootDirectoryEntrySize();
        for (size_t i = 0; i < size; i++)
        {
            auto entry = this->getRootDirectoryEntry(i);
            if(strncmp((char*)entry->filename, "KERNEL  BIN", 11) == 0){
                return entry;
            }
        }
        return nullptr;
    }

    void readFileContent(const DirectoryEntry* entry, std::function<void(uint8_t*, uint32_t)> fn){
        uint16_t cluster = entry->low_16_cluster;
        uint8_t* tmpBuffer = new uint8_t[clusterSize];
        uint32_t bytesToRead = entry->filesize;
        do{
            if(this->readCluster(cluster, tmpBuffer)){
                if(bytesToRead < clusterSize)
                    fn(tmpBuffer, bytesToRead);
                else 
                    fn(tmpBuffer, clusterSize);
                bytesToRead -= clusterSize;
            }
            cluster = this->readFat(cluster);
        }while(cluster < 0xFF8);
        delete[] tmpBuffer;
    }
    

};

int main(int argc, char* argv[]){
    Fat12Reader reader("../bin/os.bin");

    const DirectoryEntry* kernelEntry;
    if((kernelEntry = reader.findKernelBin()) != nullptr){
        std::cout << "Found \"KERNEL BIN\"" << std::endl;
        std::fstream outfile("kernel.bin", std::fstream::out | std::fstream::binary);
        // Read kernel
        std::cerr << "Extracting kernel.bin file" << std::endl;
        reader.readFileContent(kernelEntry, [&](uint8_t* buf, uint32_t size){
            outfile.write((char*)buf, size);
        });
        outfile.close();
        return 0;
    }else{
        std::cerr << "Unable to find \"KERNEL BIN\"" << std::endl;
        return 1;
    }

    return 0;
}