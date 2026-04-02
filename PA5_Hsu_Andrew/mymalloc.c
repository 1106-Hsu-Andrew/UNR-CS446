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

#define MBLOCK_HEADER_SZ offsetof(mblock_t, payload)
mlist_t mlist;
mlist.head = NULL;

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

mblock_t* findLastMemlistBlock();
mblock_t* findFreeBlockOfSize(size_t size);
void splitAndCoallesceBlockAtSize(mblock_t* block, size_t newSize);
mblock_t* growHeapBySize(size_t size);
void* mymalloc(size_t size);
void myfree(void* ptr);

mblock_t* findLastMemlistBlock(){
    mblock_t* currBlock = mlist.head;
    if(currBlock == NULL){
        printf("List is empty.\n");
        return;
    }
    while(currBlock != NULL){
        currBlock = currBlock->next;
    }
    return currBlock;
}

mblock_t* findFreeBlockOfSize(size_t size){
    mblock_t* currBlock = mlist.head;
    if(currBlock == NULL){
        printf("List is empty.\n");
        return;
    }
    while(currBlock != NULL && currBlock.size < size){
        currBlock = currBlock->next;
    }
    return currBlock;
}

void splitAndCoallesceBlockAtSize(mblock_t* block, size_t newSize){
    mblock_t* remainingBlock = &block + MBLOCK_HEADER_SZ + newSize;

    freedBlock->prev = &block;
    freedBlock->next = block->next;
    freedBlock->status = 0;
    remainingBlock->size = MBLOCK_HEADER_SZ + newSize;

    block->next->prev = &remainingBlock;
    block->next = &remainingBlock;
    block->status = 1;
}

mblock_t* growHeapBySize(size_t size){
    breakIncrement = (size > 1024) ? size : 1024;
    prevBreak = sbrk(breakIncrement);
    if(prevBreak == (void*) - 1){
        return NULL;
    }

    mblock_t* newBlock = &prevBreak + MBLOCK_HEADER_SZ + size;
    newBlock->prev = &prevBreak;
    newBlock->next = NULL;
    newBlock->status = 0;
    newBlock->size = size;

    lastBlock = findLastMemBlock();
    lastBlock->next = &newBlock;
}

void* mymalloc(size_t size){
    mblock_t* freeBlock = findFreeBlockOfSize(size);
    if(freeBlock != NULL){
        splitAndCoallesceBlockAtSize(freeBlock, size);
    }
    else{
        growHeapBySize(size);
    }


}

int main(){
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