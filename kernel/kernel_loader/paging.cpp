#include "paging.hpp"

// Must be multiple of PAGE_SIZE
#define KERNEL_LOC 0x00000000C0100000

Paging* paging;

extern "C" void setup_paging(){
    uint64_t physicalRam = 0x80000000; // 2GB
    uint64_t virtualRam = 0x100000000; // 4GB

    paging = new Paging(physicalRam, virtualRam);
    //Reserved mapped
    //first frame 2000 frames is reserved
    for (std::size_t i = 0; i < 1025; i++)
    {
        paging->getFrameAllocator()->setFrame(i * PAGE_SIZE);
    }
    paging->initPage();

    //Identity map from 0MB-1MB
    for(std::size_t i = 0; i < 512; i++){
        //identity mapping
        paging->mapVirtualPageToPhysicalFrame(i, i);
    }
    // //Identity map from the 2MB onwards
    // for(std::size_t i = 1024; i < paging->getFrameAllocator()->getFrameInfo()->nframes; i++){
    //     //identity mapping
    //     paging->mapVirtualPageToPhysicalFrame(i, i);
    // }
    //Map the kernel at KERNEL_LOC to the 1MB-2MB
    auto offset = KERNEL_LOC / PAGE_SIZE;
    for(std::size_t i = 512; i < 1024; i++){
        paging->mapVirtualPageToPhysicalFrame(i + offset - 512, i);
    }
    //Identity map from the 2MB-7MB
    auto start = 1024;
    auto end = 0xD00000 / PAGE_SIZE;
    for(std::size_t i = start; i < end; i++){
        //identity mapping
        paging->mapVirtualPageToPhysicalFrame(i, i);
    }

    paging->setCR3Register();
    
    // delete frameAllocator;

}