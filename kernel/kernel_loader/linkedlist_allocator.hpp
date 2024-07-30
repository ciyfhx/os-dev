#ifndef INCLUDE_LIST_H
#define INCLUDE_LIST_H

#include <type_traits>
#include <cstdint>

struct block_node{
    block_node* prev;
    block_node* next;
    uint32_t blockSize; // size of block excluding the sizeof(block_node)
};

//Linked list style heap memory allocator
class linkedlist_allocator {
private:
    block_node avail_block_start;
    block_node alloc_block_start;
    block_node avail_block_end;
    block_node alloc_block_end;
    std::size_t* heapBaseAddress;
    std::size_t size;

    /**
     * Insert a connected block before
     *
     * Before:
     * Block_Prev <--> startingBlock <--> Block_Next
     *
     * After:
     * Block_Prev <--> blockToInsert <--> startingBlock <--> Block_Next
     *
     * @param blockToInsert
     * @param startingBlock
     */
    static inline void insertBlockBefore(block_node* blockToInsert, block_node* startingBlock){
        linkBlocks(startingBlock->prev, blockToInsert);
        linkBlocks(blockToInsert, startingBlock);
    }

    /**
     * Replace a connected block with another block
     *
     * Before:
     * Block_Prev <--> block1 <--> Block_Next
     *
     * After:
     * Block_Prev <--> block2 <--> Block_Next
     *
     * @param block1
     * @param block2
     */
    static inline void replaceBlock(block_node* block1, block_node* block2){
        linkBlocks(block1->prev, block2);
        linkBlocks(block2, block1->next);
    }

    /**
     * Remove a block
     *
     * Before:
     * Block_Prev <--> block <--> Block_Next
     *
     * After:
     * Block_Prev <--> Block_Next
     *
     * @param block
     */
    static inline void removeBlock(block_node* block){
        linkBlocks(block->prev, block->next);
    }

    /**
     * Link Two blocks together
     * block1 <--> noblock2
     *
     * block1 forward connected --> block2
     * block2 backward connected --> block1
     * @param block1
     * @param block2
     */
    static inline void linkBlocks(block_node* block1, block_node* block2){
        block1->next = block2;
        block2->prev = block1;
    }

    static inline bool isAdjacent(block_node* node1, block_node* node2){
        return ((std::size_t)node1 + sizeof(block_node) + node1->blockSize) == (std::size_t)node2 ||
                ((std::size_t)node2 + sizeof(block_node) + node2->blockSize) == (std::size_t)node1;
    }

    static inline void mergeBlocks(block_node* node1, block_node* node2){
        node1->blockSize += node2->blockSize + sizeof(block_node);
    }

    static inline void createBlock(block_node* node, std::size_t blockSize){
        node->blockSize = blockSize;
        node->next = nullptr;
        node->prev = nullptr;
    }

public:
    // linkedlist_allocator(std::size_t* heapBaseAddress, uint32_t size) : heapBaseAddress(heapBaseAddress), size(size) {
    //     auto* avail_block = (block_node*)heapBaseAddress;
    //     createBlock(avail_block, size - sizeof(block_node));
    //     linkBlocks(&avail_block_start, avail_block);
    //     linkBlocks(avail_block, &avail_block_end);
    //     avail_block_start.blockSize = 0;
    //     avail_block_end.blockSize = 0;

    //     linkBlocks(&alloc_block_start, &alloc_block_end);
    //     alloc_block_start.blockSize = 0;
    //     alloc_block_end.blockSize = 0;

    // }

    void init(std::size_t* heapBaseAddress, uint32_t size){
        this->heapBaseAddress = heapBaseAddress;
        this->size = size;
        auto* avail_block = (block_node*)heapBaseAddress;
        createBlock(avail_block, size - sizeof(block_node));
        linkBlocks(&avail_block_start, avail_block);
        linkBlocks(avail_block, &avail_block_end);
        avail_block_start.blockSize = 0;
        avail_block_end.blockSize = 0;

        linkBlocks(&alloc_block_start, &alloc_block_end);
        alloc_block_start.blockSize = 0;
        alloc_block_end.blockSize = 0;

    }

    std::size_t* malloc(uint32_t size){
        block_node* curr_avail_block = avail_block_start.next;
        //find available block that can fit the requested size
        while(curr_avail_block != nullptr) {
            std::size_t remainingSizeInBlock = curr_avail_block->blockSize - size;
            if(remainingSizeInBlock > 0){
                if(remainingSizeInBlock > sizeof(block_node)){
                    //Create a new available block
                    curr_avail_block->blockSize = size;
                    auto* new_avail_block = (block_node*)((std::size_t) curr_avail_block + sizeof(block_node) + size);
                    new_avail_block->blockSize = remainingSizeInBlock - sizeof(block_node);

                    //replace the current block with the new block
                    replaceBlock(curr_avail_block, new_avail_block);
                }else{
                    //remove the current block
                    removeBlock(curr_avail_block);
                }

                // add the new current block to the allocated block
                insertBlockBefore(curr_avail_block, alloc_block_start.next);

                return (std::size_t*)((std::size_t)curr_avail_block + sizeof(block_node));
            }
            curr_avail_block = curr_avail_block->next; 
        }
        //OOM
        return nullptr;
    }

    void free(std::size_t* address){
        auto* cur_alloc_block = (block_node*) ((std::size_t)address - sizeof(block_node));

        //Remove block from allocated
        removeBlock(cur_alloc_block);

        //Insert the freed block into the available block but in ascending order
        block_node* precedingAvailBlock = &avail_block_start;
        while(precedingAvailBlock->next != nullptr){
            // find preceding available block
            if(precedingAvailBlock->next > cur_alloc_block && precedingAvailBlock->blockSize != 0)break;
            precedingAvailBlock = precedingAvailBlock->next;
        }
        insertBlockBefore(cur_alloc_block, precedingAvailBlock);

        if(isAdjacent(precedingAvailBlock, cur_alloc_block)){
            //Merge the previous block with this one
            mergeBlocks(precedingAvailBlock, cur_alloc_block);

            removeBlock(cur_alloc_block);
            cur_alloc_block = precedingAvailBlock;
        }
        if(isAdjacent(cur_alloc_block, cur_alloc_block->next)){
            //Merge the next block with this one
            mergeBlocks(cur_alloc_block, cur_alloc_block->next);
            removeBlock(cur_alloc_block->next);
        }
    }
};

#endif /* INCLUDE_LIST_H */

