#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

typedef struct _mblock_t {
  struct _mblock_t * prev;
  struct _mblock_t * next;
  size_t size;
  int status;
  void * payload;
} mblock_t;

typedef struct _mlist_t {
    mblock_t * head;
} mlist_t;

#define MBLOCK_HEADER_SZ offsetof(mblock_t, payload)
mlist_t mlist;

mblock_t* findLastMemlistBlock();
mblock_t* findFreeBlockOfSize(size_t size);
void splitBlockAtSize(mblock_t* block, size_t newSize);
void coallesceBlockPrev(mblock_t* freedBlock);
mblock_t* growHeapBySize(size_t size);
void* mymalloc(size_t size);
void myfree(void* ptr);

mblock_t* findLastMemlistBlock(){
    mblock_t* currBlock = mlist.head;
    if(currBlock == NULL){
        return NULL;
    }
    while(currBlock->next != NULL){
        currBlock = currBlock->next;
    }
    return currBlock;
}

mblock_t* findFreeBlockOfSize(size_t size){
    mblock_t* currBlock = mlist.head;
    if(currBlock == NULL){
        return NULL;
    }
    while(currBlock != NULL){
        if(currBlock->size >= size && currBlock->status == 0){
            return currBlock;
        }
        currBlock = currBlock->next;
    }
    return currBlock;
}

void splitBlockAtSize(mblock_t* block, size_t newSize){
    if(block->size >= MBLOCK_HEADER_SZ + newSize + 1){
        mblock_t* remainingBlock = (mblock_t*)((char*)block + MBLOCK_HEADER_SZ + newSize);
        remainingBlock->prev = block;
        remainingBlock->next = block->next;
        remainingBlock->size = block->size - MBLOCK_HEADER_SZ - newSize;
        remainingBlock->status = 0;

        if(block->next != NULL){
        block->next->prev = remainingBlock;
        }
        block->next = remainingBlock;
        block->size = newSize;
    }
    block->status = 1;
}

mblock_t* growHeapBySize(size_t size){
    size_t breakIncrement = (size + MBLOCK_HEADER_SZ > 1024) ? size : 1024;
    void* prevBreak = sbrk(breakIncrement);
    if(prevBreak == (void*) - 1){
        return NULL;
    }

    mblock_t* newBlock = (mblock_t*)prevBreak;
    mblock_t* lastBlock = findLastMemlistBlock();
    if(lastBlock == NULL){
        mlist.head = newBlock;
        newBlock->prev = NULL;
    }
    else{
        newBlock->prev = lastBlock;
        lastBlock->next = newBlock;
    }
    newBlock->next = NULL;
    newBlock->size = breakIncrement - MBLOCK_HEADER_SZ;
    newBlock->status = 0;

    return newBlock;
}

void coallesceBlockPrev(mblock_t* freedBlock){
    mblock_t* prevBlock = freedBlock->prev;
    mblock_t* nextBlock = freedBlock->next;
    if(nextBlock != NULL && nextBlock->status == 0){
        freedBlock->next = nextBlock->next;
        if(nextBlock->next != NULL){
            nextBlock->next->prev = freedBlock;
        }
        freedBlock->size += MBLOCK_HEADER_SZ + nextBlock->size;
    }

    if(prevBlock != NULL && prevBlock->status == 0){
        prevBlock->next = freedBlock->next;
        if(freedBlock->next != NULL){
            freedBlock->next->prev = prevBlock;
        }
        prevBlock->size += MBLOCK_HEADER_SZ + freedBlock->size;
    }
}

void* mymalloc(size_t size){
    if(size <= 0){
        return NULL;
    }

    mblock_t* freeBlock = findFreeBlockOfSize(size);
    if(freeBlock == NULL){
        freeBlock = growHeapBySize(size);
        if(freeBlock == NULL){
            return NULL;
        }
    }
    splitBlockAtSize(freeBlock, size);
    return(&(freeBlock->payload));
}

void myfree(void* ptr){
    void* currBreak = sbrk(0);
    if(ptr == NULL || ptr < (void*)mlist.head || ptr >= currBreak){
        return;
    }
    mblock_t* beginningOfMBlock = (mblock_t*)((char*)ptr - MBLOCK_HEADER_SZ);
    beginningOfMBlock->status = 0;
    coallesceBlockPrev(beginningOfMBlock);
}

int main(){
    mlist.head = NULL;
    void * p1 = mymalloc(10);
    void * p2 = mymalloc(100);
    void * p3 = mymalloc(200);
    void * p4 = mymalloc(500);
    myfree(p3); p3 = NULL;
    myfree(p2); p2 = NULL;
    void * p5 = mymalloc(150);
    void * p6 = mymalloc(500);
    myfree(p4); p4 = NULL;
    myfree(p5); p5 = NULL;
    myfree(p6); p6 = NULL;
    myfree(p1); p1 = NULL;
}