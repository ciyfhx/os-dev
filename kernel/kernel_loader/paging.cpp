#include "paging.hpp"

// 2GB
#define VM_SIZE 0x7FFFFFFF

#define PAGE_SIZE 0x1000
#define PAGE_MAX_ENTRIES 512
#define PAGE_ENTRY_SIZE 8

// Must be multiple of PAGE_SIZE


#define PAGE_LOC 0x1A0000

inline uint32_t min(uint32_t num1, uint32_t num2) {
    return num1 > num2 ? num2 : num1;
}

void memclr(uint32_t* dest, uint32_t size){
    for (uint32_t i = 0; i < size; i++)
    {
        dest[i] = 0;
    }
}

extern "C" void setup_paging(){
    uint32_t maxRam = 0x80000000; // 2GB
    uint32_t framesOrTotalPte = maxRam / PAGE_SIZE;
    uint32_t totalPtOrPdtEntries = framesOrTotalPte / PAGE_MAX_ENTRIES;
    uint32_t totalPdtOrPdptEntries = totalPtOrPdtEntries / PAGE_MAX_ENTRIES;
    uint32_t totalPdptOrPml4tEntries = totalPdtOrPdptEntries / PAGE_MAX_ENTRIES;
    uint32_t totalPml4t = totalPdptOrPml4tEntries / PAGE_MAX_ENTRIES;

    if(totalPdptOrPml4tEntries == 0)totalPdptOrPml4tEntries = 1;
    if(totalPml4t == 0)totalPml4t = 1;

    uint32_t PML4_LOC = PAGE_LOC;
    uint32_t PDPT_LOC = PML4_LOC + PAGE_SIZE * totalPml4t;
    uint32_t PDT_LOC = PDPT_LOC + PAGE_SIZE * totalPdptOrPml4tEntries;
    uint32_t PT_LOC = PDT_LOC + PAGE_SIZE * totalPdtOrPdptEntries;

    memclr((uint32_t*)PML4_LOC, PAGE_SIZE * (totalPtOrPdtEntries + totalPdtOrPdptEntries + totalPdptOrPml4tEntries + totalPml4t));

    //Setup PML4T
    PPML4E pml4tableEntry = (PPML4E) (PML4_LOC);
    for (uint32_t i = 0; i < totalPdptOrPml4tEntries; i++)
    {
        PPDPTE pTable = (PPDPTE) ((PDPT_LOC) + (i * PAGE_SIZE));
        pml4tableEntry->Present = true;
        pml4tableEntry->ReadWrite = true;
        pml4tableEntry->PageFrameNumber = ((uint32_t) pTable / PAGE_SIZE) & 0xFFFFFFFFF;
        pml4tableEntry++;
    }

    //Setup PDPT
    PPDPTE pdptableEntry = (PPDPTE) (PDPT_LOC);
    for (uint32_t i = 0; i < totalPdtOrPdptEntries; i++)
    {
        PPDE pTable = (PPDE) ((PDT_LOC) + (i * PAGE_SIZE));
        pdptableEntry->Present = true;
        pdptableEntry->ReadWrite = true;
        pdptableEntry->PageFrameNumber = ((uint32_t) pTable / PAGE_SIZE) & 0xFFFFFFFFF;
        pdptableEntry++;
    }

    //Setup PDT
    PPDE pdtableEntry = (PPDE) (PDT_LOC);
    for (uint32_t i = 0; i < totalPtOrPdtEntries; i++)
    {
        PPTE pTable = (PPTE) ((PT_LOC) + (i * PAGE_SIZE));
        pdtableEntry->Present = true;
        pdtableEntry->ReadWrite = true;
        pdtableEntry->PageFrameNumber = ((uint32_t) pTable / PAGE_SIZE) & 0xFFFFFFFFF;
        pdtableEntry++;
    }
    
    //Setup PT
    uint32_t id = 0;
    PPTE ptableEntry = (PPTE) (PT_LOC);
    for (uint32_t i = 0; i < framesOrTotalPte; i++)
    {
        ptableEntry->Present = true;
        ptableEntry->ReadWrite = true;
        ptableEntry->PageFrameNumber = id++; // identity paging
        ptableEntry++;
    }
}