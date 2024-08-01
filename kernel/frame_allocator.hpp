#ifndef INCLUDE_FRAME_ALLOCATOR_H
#define INCLUDE_FRAME_ALLOCATOR_H

#include <cstdint>
#define PAGE_SIZE 0x1000

#define INDEX_FROM_BIT(a) (a/(8*8))
#define OFFSET_FROM_BIT(a) (a%(8*8))

typedef struct FrameInfo {
    uint64_t *frames;
    uint64_t nframes;
};

class FrameAllocator {
private:
    FrameInfo* frameInfo;

public:
    FrameAllocator(FrameInfo* info): frameInfo(info)  {}

    void setFrame(uint64_t frameAddress){
        uint64_t frame = frameAddress / PAGE_SIZE;
        uint64_t id = INDEX_FROM_BIT(frame);
        uint64_t offset = OFFSET_FROM_BIT(frame);
        frameInfo->frames[id] |= (0x1 << offset);
    }

    void clearFrame(uint64_t frameAddress){
        uint64_t frame = frameAddress / PAGE_SIZE;
        uint64_t id = INDEX_FROM_BIT(frame);
        uint64_t offset = OFFSET_FROM_BIT(frame);
        frameInfo->frames[id] &= ~(0x1 << offset);
    }

    bool testFrame(uint64_t frameAddress){
        uint64_t frame = frameAddress / PAGE_SIZE;
        uint64_t id = INDEX_FROM_BIT(frame);
        uint64_t offset = OFFSET_FROM_BIT(frame);
        return (frameInfo->frames[id] & (0x1 << offset));
    }

    uint64_t findFrame(){
        uint64_t i, j;
        for (i = 0; i < INDEX_FROM_BIT(frameInfo->nframes); i++)
        {
            if (frameInfo->frames[i] != 0xFFFFFFFFFFFFFFFF) // nothing free, exit early.
            {
                // at least one bit is free here.
                for (j = 0; j < 64; j++)
                {
                    uint64_t toTest = 0x1 << j;
                    if ( !(frameInfo->frames[i]&toTest) )
                    {
                        return i*8*8+j;
                    }
                }
            }
        }
    }

};


#endif /** INCLUDE_FRAME_ALLOCATOR_H */