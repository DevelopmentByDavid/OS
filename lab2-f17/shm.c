#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id = 0;
    shm_table.shm_pages[i].frame = 0;
    shm_table.shm_pages[i].refcnt = 0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {
    //acquire lock here
    //scan for the id passed in
    int i = 0;
    int empty = -1;
    struct proc *p = myproc();
    acquire(&(shm_table.lock));
    for (; i < 64; i++) {
        if (shm_table.shm_pages[i].id == id) break;
        if (shm_table.shm_pages[i].id == 0 && empty == -1) empty = i;
    }

    if (i == 64 && empty != -1) {               //intialize the page
        //set id to index
        i = empty;
        shm_table.shm_pages[i].id = id;
        //allocate a page to the frame
        shm_table.shm_pages[i].frame = kalloc();
        //set refcount to 1
        shm_table.shm_pages[i].refcnt = 1;

        //do stuff now
        uint va = PGROUNDUP(p->sz);
        mappages(p->pgdir, (void *)va, PGSIZE, V2P(shm_table.shm_pages[id].frame), PTE_W | PTE_U);
        *pointer = (char *) va;
        release(&(shm_table.lock));
        return va;
    } else if (i < 64) {                       //page found
       //declare virtual address
       uint va = PGROUNDUP(p->sz);
       //mappages
       mappages(p->pgdir,
               (void*)va,
               PGSIZE,
               V2P(shm_table.shm_pages[id].frame),
               PTE_W | PTE_U);
       //pointer to pointer
       *pointer = (char *) va;
       release(&(shm_table.lock));
       return va;
    } else {                                   //page not found and no empty pages
        
    }



    release(&(shm_table.lock));
    return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
    //you write this too!
    int i = 0;
    acquire(&(shm_table.lock));
    for (; i < 64; i++) {
        if (shm_table.shm_pages[i].id == id) break;
    }
    
    if (i < 64) {
        int temp = shm_table.shm_pages[i].refcnt--;
        if (temp == 0) {
            shm_table.shm_pages[i].frame = 0;
            shm_table.shm_pages[i].id = 0;
        }
    }
    release(&(shm_table.lock));
    return 0; //added to remove compiler warning -- you should decide what to return
}
