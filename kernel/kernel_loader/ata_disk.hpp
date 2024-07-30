#ifndef INCLUDE_ATA_DISK_H
#define INCLUDE_ATA_DISK_H

#include <cstdint>
#include "ata_disk_asm.hpp"
#include "stream.hpp"
#include "mem.hpp"

#define ATA_SECTOR_SIZE 512

namespace kstd {
    class ata_disk : public istream {
        private: 
            uint32_t pos = 0;
            uint8_t err = 0;
            uint8_t tbuf[ATA_SECTOR_SIZE];
        public:
            // ata_disk() = default;
            void ignore(uint32_t n) override;
            void seekg(uint32_t pos) override;
            uint8_t fail() override;
            void read(char* buf, uint32_t size) override;
    };
}

#endif /* INCLUDE_ATA_DISK_H */
    