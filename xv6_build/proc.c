#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

#define QUEUE_SIZE 100

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

//!MODIFIED
struct queue {
    struct el *head;
    struct el *tail;
};

static struct queue QUEUE[QUEUE_SIZE] = {{0,0}};

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}



void queue_push(struct proc *proc, int index) {
    //if queue is empty set head and tail to proc
    //acquire(&ptable.lock);
    
    if((QUEUE[index].head == 0) && (QUEUE[index].tail == 0)){
        proc->el.prevEl = proc->el.nextEl = 0;
        proc->el.proc = proc;
        QUEUE[index].head = QUEUE[index].tail = &(proc->el);
        //release(&ptable.lock);
        return;
    }
    //change current tail to proc arg
    QUEUE[index].tail->nextEl = &(proc->el);
    //push proc arg onto queue
    proc->el.proc = proc;
    proc->el.nextEl = 0;
    proc->el.prevEl = QUEUE[index].tail;
    QUEUE[index].tail = &(proc->el);
    //release(&ptable.lock);
 
}

//scheduler needs to push process back on if it is still runnable
struct proc *queue_pop() {
   int i = 0;
   for(;i < QUEUE_SIZE; i++){
       if (QUEUE[i].head == 0) {
           continue;
       } else {
            break;
       }
   }

    //unfinished processes are popped back after it switches back to scheduler
    //QUEUE[i].tail->nextEl = QUEUE[i].head;
    //change tail to current head
    //QUEUE[i].tail = QUEUE[i].head;
    //change queue head to oldhead->nextEl
    //QUEUE[i].head = QUEUE[i].head->nextEl;
    //change oldHead->nextEl to null
    //QUEUE[i].tail->nextEl = 0;
    //change oldTail->nextEl to oldHead
    //return oldHead->myProc;
    
    if(QUEUE[i].head == 0) return 0;
    struct proc* p = QUEUE[i].head->proc;
    if (QUEUE[i].head->nextEl != 0) {
        QUEUE[i].head = QUEUE[i].head->nextEl;
        QUEUE[i].head->prevEl = 0;
    }
    return p;
}

void queue_promote_all() {
    //promote all to first queue
    //for(int i = 1; i < 100; i++) {
        //not doing rn
    //}
}

struct el *getPrev(struct el *el) {
    return el->prevEl;
}
struct el *getNext(struct el *el) {
    return el->nextEl;
}

void queue_search_and_destroy (struct proc *proc, int index) {
    //don't care about efficiency atm, just getting to work
    //plus efficiency here won't make the biggest deal in the world
    //just looks nooby
    int found = 0;
    struct el *newTail;
    struct el *currEl;
    struct el *newHead = newTail = currEl = 0;
    if (QUEUE[index].head->proc == proc) {
        newHead = QUEUE[index].head->nextEl;
        QUEUE[index].head->nextEl = 0;
        QUEUE[index].head->prevEl = 0;
        QUEUE[index].head->proc = 0;
        QUEUE[index].head = newHead;
        found = 1;
        return;
    } else if (QUEUE[index].tail->proc == proc) {
       newTail = QUEUE[index].tail->prevEl;
       QUEUE[index].tail->prevEl = 0;
       QUEUE[index].tail->nextEl = 0;
       QUEUE[index].tail->proc = 0;
       QUEUE[index].tail = newTail;
       found = 1;
       return;
    }
    currEl = getNext(QUEUE[index].head);
    while(!found) {
        if (currEl->proc == proc) {
            //great found it
            currEl->prevEl->nextEl = currEl->nextEl;
            currEl->nextEl->prevEl = currEl->prevEl;
            currEl->prevEl = 0;
            currEl->nextEl = 0;
            currEl->proc = 0;
            found = 1;
            break;
        }
        if (currEl == QUEUE[index].tail) {
            panic("Process does not exist in queue");
            break;
        }
    }
}


//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;
  p->priority = 1; //!MODIFIED
  p->el.nextEl = 0;
  p->el.prevEl = 0;
  p->el.proc = p;
  //queue_push(p, p->priority);
  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;
  queue_push(p, p->priority);

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}
//Sets the current process's priority
int
setpriority(int priority)
{
  struct proc *p = myproc();
  acquire(&ptable.lock);
  queue_search_and_destroy(p, p->priority); 
  p->priority = priority;
  queue_push(p, p->priority);
   
  release(&ptable.lock);
  return 0; 
}
// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;
  //!MODIFIED
  //setpriority
  np->priority = curproc->priority;
  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;
  queue_push(np, np->priority);

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait(status) to find out it exited.
void
exit(int status)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  curproc->exitStatus = status;
  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait(status)
  wakeup1(curproc->parent);
  
  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(int* status)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.

        // !MODIFIED
        //queue_search_and_destroy(p, p->priority);

        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);

        //Save process exit status if the status variable is not NULL !MODIFIED
        if(status) 
          *status = p->exitStatus;

        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

int
waitpid(int pid, int* status, int options)
{
  struct proc *p;
  int cur_pid;
  int exists = 0;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->pid == pid){
        exists = 1;
        if(p->state == ZOMBIE && p->pid == pid/* !MODIFIED added pid check*/){
          // Found one.
          cur_pid = p->pid;
          kfree(p->kstack);
          p->kstack = 0;
          freevm(p->pgdir);
          p->pid = 0;
          p->parent = 0;
          p->name[0] = 0;
          p->killed = 0;
          p->state = UNUSED;
          release(&ptable.lock);

          //Save process exit status if the status variable is not NULL !MODIFIED
          if(status){ 
            *status = p->exitStatus;
	  //!MODIFIED
	        if(*status != 0)
	            return -1; 
	      } 
          return cur_pid;
        }
      }
    }
	if(!exists) return -1;
        

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}




//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    // !MODIFIED changed the loop to look for highest priority and runs it out of the loop
    acquire(&ptable.lock);
    
    p = queue_pop(); // assign schedp to p to reduce modification to original code.
    // Switch to chosen process.  It is the process's job
    // to release ptable.lock and then reacquire it
    // before jumping back to us.
    //!MODIFIED
    if (p != 0 && p->state == RUNNABLE) {
        c->proc = p;
        switchuvm(p);
        p->state = RUNNING;

        swtch(&(c->scheduler), p->context);
        switchkvm();

        if(p->state == RUNNABLE) queue_push(p, p->priority);
    
        // Process is done running for now.
        // It should have changed its p->state before coming back.
        c->proc = 0;
    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  queue_push(myproc(), myproc()->priority);
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();


  //maybe not needed
  //push proc back on queue after sleep if not woken by wakeup() i.e. after the sleep(int) syscall.
  //if(p->state == SLEEPING){
  //  p->state = RUNNABLE;
  //  queue_push(p, p->priority);
  //}
  

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan){
      p->state = RUNNABLE;
      queue_push(p, p->priority);
    }
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING){
        p->state = RUNNABLE;
        queue_push(p, p->priority);
      }
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

