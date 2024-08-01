#ifndef INCLUDE_PAGING_H
#define INCLUDE_PAGING_H

#include <cstdint>
#include <type_traits>
#include "frame_allocator.hpp"
#include "mem_allocator.hpp"
#include "basic_io.hpp"

#define PAGE_SIZE 0x1000
#define PAGE_MAX_ENTRIES 512
#define PAGE_ENTRY_SIZE 8


typedef struct _PML4E
{
    union
    {
        struct
        {
            uint64_t Present : 1;              // Must be 1, region invalid if 0.
            uint64_t ReadWrite : 1;            // If 0, writes not allowed.
            uint64_t UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
            uint64_t PageWriteThrough : 1;     // Determines the memory type used to access PDPT.
            uint64_t PageCacheDisable : 1;     // Determines the memory type used to access PDPT.
            uint64_t Accessed : 1;             // If 0, this entry has not been used for translation.
            uint64_t Ignored1 : 1;
            uint64_t PageSize : 1;             // Must be 0 for PML4E.
            uint64_t Ignored2 : 4;
            uint64_t PageFrameNumber : 36;     // The page frame number of the PDPT of this PML4E.
            uint64_t Reserved : 4;
            uint64_t Ignored3 : 11;
            uint64_t ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
        };
        uint64_t Value;
    };
} PML4E, *PPML4E;

typedef struct _PDPTE
{
    union
    {
        struct
        {
            uint64_t Present : 1;              // Must be 1, region invalid if 0.
            uint64_t ReadWrite : 1;            // If 0, writes not allowed.
            uint64_t UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
            uint64_t PageWriteThrough : 1;     // Determines the memory type used to access PD.
            uint64_t PageCacheDisable : 1;     // Determines the memory type used to access PD.
            uint64_t Accessed : 1;             // If 0, this entry has not been used for translation.
            uint64_t Ignored1 : 1;
            uint64_t PageSize : 1;             // If 1, this entry maps a 1GB page.
            uint64_t Ignored2 : 4;
            uint64_t PageFrameNumber : 36;     // The page frame number of the PD of this PDPTE.
            uint64_t Reserved : 4;
            uint64_t Ignored3 : 11;
            uint64_t ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
        };
        uint64_t Value;
    };
} PDPTE, *PPDPTE;

typedef struct _PDE
{
    union
    {
        struct
        {
            uint64_t Present : 1;              // Must be 1, region invalid if 0.
            uint64_t ReadWrite : 1;            // If 0, writes not allowed.
            uint64_t UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
            uint64_t PageWriteThrough : 1;     // Determines the memory type used to access PT.
            uint64_t PageCacheDisable : 1;     // Determines the memory type used to access PT.
            uint64_t Accessed : 1;             // If 0, this entry has not been used for translation.
            uint64_t Ignored1 : 1;
            uint64_t PageSize : 1;             // If 1, this entry maps a 2MB page.
            uint64_t Ignored2 : 4;
            uint64_t PageFrameNumber : 36;     // The page frame number of the PT of this PDE.
            uint64_t Reserved : 4;
            uint64_t Ignored3 : 11;
            uint64_t ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
        };
        uint64_t Value;
    };
} PDE, *PPDE;

typedef struct _PTE
{
    union
    {
        struct
        {
            uint64_t Present : 1;              // Must be 1, region invalid if 0.
            uint64_t ReadWrite : 1;            // If 0, writes not allowed.
            uint64_t UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
            uint64_t PageWriteThrough : 1;     // Determines the memory type used to access the memory.
            uint64_t PageCacheDisable : 1;     // Determines the memory type used to access the memory.
            uint64_t Accessed : 1;             // If 0, this entry has not been used for translation.
            uint64_t Dirty : 1;                // If 0, the memory backing this page has not been written to.
            uint64_t PageAccessType : 1;       // Determines the memory type used to access the memory.
            uint64_t Global: 1;                // If 1 and the PGE bit of CR4 is set, translations are global.
            uint64_t Ignored2 : 3;
            uint64_t PageFrameNumber : 36;     // The page frame number of the backing physical page.
            uint64_t Reserved : 4;
            uint64_t Ignored3 : 7;
            uint64_t ProtectionKey: 4;         // If the PKE bit of CR4 is set, determines the protection key.
            uint64_t ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
        };
        uint64_t Value;
    };
} PTE, *PPTE;

extern "C" void setup_paging();


typedef struct PagingInfo {
    uint64_t virtualRamSize;
    uint64_t physicalRamSize;
    PPML4E rootTablePhysicalAddress;
    PPML4E rootTableVirtualAddress;
    bool pagingEnabled = false;
};

template <typename Page>
concept IsPage = requires (Page page) {
    std::bool_constant<std::is_same_v<Page, PTE> || std::is_same_v<Page, PPDE> || std::is_same_v<Page, PPDPTE> || std::is_same_v<Page, PPML4E>>::value;
};

class Paging {
private: 

    PagingInfo* pagingInfo;
    FrameAllocator* frameAllocator;

    template <typename PageParentTable, typename PageChildTable, typename Index>
    requires IsPage<PageParentTable> && IsPage<PageChildTable> && std::is_integral_v<Index>
    auto* getPageTable(PageParentTable* table, Index index){    
        auto* entry = &table[index];
        //If not exist we create one
        if(!entry->Present){
            entry->Present = true;
            entry->ReadWrite = true;
            auto frameNo = frameAllocator->findFrame();
            entry->PageFrameNumber = frameNo;
            auto frameAddress = frameNo * PAGE_SIZE;
            memclr((uint64_t*)frameAddress, PAGE_SIZE);
            frameAllocator->setFrame(frameAddress);
            return (PageChildTable*)frameAddress;
        }else{
            auto frameNo = entry->PageFrameNumber;
            auto frameAddress = frameNo * PAGE_SIZE;
            return (PageChildTable*)frameAddress;
        }

    }

    void memclr(uint64_t* dest, uint64_t size){
        for (uint64_t i = 0; i < size; i++)
        {
            dest[i] = 0;
        }
    }


    PPML4E getRootTable() {return (PPML4E) (!pagingInfo->pagingEnabled ? pagingInfo->rootTablePhysicalAddress : pagingInfo->rootTableVirtualAddress);}

public:
    Paging(uint64_t physicalRam, uint64_t virtualRam){
        pagingInfo = new PagingInfo;
        pagingInfo->physicalRamSize = physicalRam;
        pagingInfo->virtualRamSize = virtualRam;

        frameAllocator = new FrameAllocator(physicalRam);
    }
    ~Paging(){
        delete pagingInfo;
        delete frameAllocator;
    }

    FrameAllocator* getFrameAllocator() {
        return frameAllocator;
    }

    void initPage(){
        auto frameNo = frameAllocator->findFrame();
        auto frameAddress = frameNo * PAGE_SIZE;
        frameAllocator->setFrame(frameAddress);
        pagingInfo->rootTablePhysicalAddress = frameAddress;
        //Setup PML4T
        memclr((uint64_t*)pagingInfo->rootTablePhysicalAddress, PAGE_SIZE);
    }

    void mapVirtualPageToPhysicalFrame(uint64_t vPage, uint64_t pFrame){
        uint32_t pageTableEntry = vPage % PAGE_MAX_ENTRIES;
        uint32_t pageDirectoryTableEntry = vPage / PAGE_MAX_ENTRIES % PAGE_MAX_ENTRIES;
        uint32_t pageDirectoryPointerTableEntry = vPage / (PAGE_MAX_ENTRIES * PAGE_MAX_ENTRIES) % PAGE_MAX_ENTRIES;
        uint32_t pml4PageTableEntry = vPage / (PAGE_MAX_ENTRIES * PAGE_MAX_ENTRIES * PAGE_ENTRY_SIZE) % PAGE_MAX_ENTRIES;

        PPDPTE pageDirectoryPointerTable = getPageTable<PML4E, PDPTE>(getRootTable(), pml4PageTableEntry);
        PPDE pageDirectoryTable = getPageTable<PDPTE, PDE>(pageDirectoryPointerTable, pageDirectoryPointerTableEntry);
        PPTE pageTable = getPageTable<PDE, PTE>(pageDirectoryTable, pageDirectoryTableEntry);

        if(pageTable[pageTableEntry].Present){
            panic("Page already been mapped");
        }
        pageTable[pageTableEntry].Present = true;
        pageTable[pageTableEntry].ReadWrite = true;
        pageTable[pageTableEntry].PageFrameNumber = pFrame;
    }

    void unmapVirtualToPhysicalFrame(std::size_t vPage){
        uint64_t pageTableEntry = vPage % PAGE_MAX_ENTRIES;
        uint64_t pageDirectoryTableEntry = vPage / PAGE_MAX_ENTRIES;
        uint64_t pageDirectoryPointerTableEntry = vPage / (PAGE_MAX_ENTRIES * PAGE_MAX_ENTRIES);
        uint64_t pml4PageTableEntry = vPage / (PAGE_MAX_ENTRIES * PAGE_MAX_ENTRIES * PAGE_ENTRY_SIZE);

        PPDPTE pageDirectoryPointerTable = getPageTable<PML4E, PDPTE>(getRootTable(), pml4PageTableEntry);
        PPDE pageDirectoryTable = getPageTable<PDPTE, PDE>(pageDirectoryPointerTable, pageDirectoryPointerTableEntry);
        PPTE pageTable = getPageTable<PDE, PTE>(pageDirectoryTable, pageDirectoryTableEntry);

        pageTable[pageTableEntry].Present = 0;
        frameAllocator->clearFrame(pageTable[pageTableEntry].PageFrameNumber * PAGE_SIZE);
    }

    void setCR3Register(){
        asm volatile("mov %0, %%cr3":: "r"(pagingInfo->rootTablePhysicalAddress));
        // u32int cr0;
        // asm volatile("mov %%cr0, %0": "=r"(cr0));
        // cr0 |= 0x80000000; // Enable paging!
        // asm volatile("mov %0, %%cr0":: "r"(cr0));
    }

};

#endif /* INCLUDE_PAGING_H */
    