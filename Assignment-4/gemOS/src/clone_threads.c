#include<clone_threads.h>
#include<entry.h>
#include<context.h>
#include<memory.h>
#include<lib.h>
#include<mmap.h>
 
/*
  system call handler for clone, create thread like 
  execution contexts. Returns pid of the new context to the caller. 
  The new context starts execution from the 'th_func' and 
  use 'user_stack' for its stack
*/
long do_clone(void *th_func, void *user_stack, void *user_arg) 
{

  struct exec_context *new_ctx = get_new_ctx();
  struct exec_context *ctx = get_current_ctx();

  u32 pid = new_ctx->pid;
  
  if(!ctx->ctx_threads){  // This is the first thread
          ctx->ctx_threads = os_alloc(sizeof(struct ctx_thread_info));
          bzero((char *)ctx->ctx_threads, sizeof(struct ctx_thread_info));
          ctx->ctx_threads->pid = ctx->pid;
  }
     
 /* XXX Do not change anything above. Your implementation goes here*/
  
  // allocate page for os stack in kernel part of process's VAS
  // The following two lines should be there. The order can be 
  // decided depending on your logic.
  // new_ctx is not assigned

   setup_child_context(new_ctx);
   new_ctx->type = EXEC_CTX_USER_TH;    // Make sure the context type is thread

   // pid is set already
   new_ctx->ppid = ctx->pid;
   new_ctx->state = WAITING;
   new_ctx->used_mem = ctx->used_mem;
   new_ctx->pgd = ctx->pgd;
   // os stack and os rsp are initialised in setup_child_context.
   int i = 0;
   while(i < MAX_MM_SEGS){
     new_ctx->mms[i] = ctx->mms[i];
     i++;
   }
   i=0;
   new_ctx->vm_area = ctx->vm_area;
   while(i < CNAME_MAX){
     new_ctx->name[i] = ctx->name[i];
     i++;
   }
   i=0;

   // change regs accordingly 
   new_ctx->regs = ctx->regs;
   new_ctx->regs.rbp = (u64)user_stack;
   new_ctx->regs.rdi = (u64)user_arg;
   new_ctx->regs.entry_rip = (u64)th_func;
   new_ctx->regs.entry_rsp = (u64)user_stack;
   
   new_ctx->pending_signal_bitmap = ctx->pending_signal_bitmap;
   while(i<MAX_SIGNALS){
     new_ctx->sighandlers[i] = ctx->sighandlers[i];
     i++;
   }
   i=0;

   new_ctx->ticks_to_sleep = ctx->ticks_to_sleep;
   new_ctx->alarm_config_time = ctx->alarm_config_time;
   new_ctx->ticks_to_alarm = ctx->ticks_to_alarm;
   while(i<MAX_OPEN_FILES){
     new_ctx->files[i] = ctx->files[i];
     i++;
   }

   // find the thread index which is not used.
   i=0;
   while(ctx->ctx_threads->threads[i].status == TH_USED && i<MAX_THREADS){
     i++;
   }
   if( i >= MAX_THREADS){
     return -1;
   }

   // update the variables
   ctx->ctx_threads->threads[i].pid = new_ctx->pid;
   ctx->ctx_threads->threads[i].status = TH_USED;
   ctx->ctx_threads->threads[i].parent_ctx = ctx;
   i=0;
   
   long ret = new_ctx->pid;
	 return ret;
}

/*This is the page fault handler for thread private memory area (allocated using 
 * gmalloc from user space). This should fix the fault as per the rules. If the the 
 * access is legal, the fault handler should fix it and return 1. Otherwise it should
 * invoke segfault_exit and return -1*/

int handle_thread_private_fault(struct exec_context *current, u64 addr, int error_code)
{
  
   /* your implementation goes here*/
    // initial error condition
    int present_bit = error_code%2;
    int write_bit = (error_code>>1)%2;
    int privilage_bit = (error_code>>2)%2;

    // always give segfault
    if(present_bit == 1 || privilage_bit == 0){
      segfault_exit(current->pid, current->regs.entry_rip, addr);
      return -1;
    }
    // if user mode can access
    else{

      // finding if it is parent or child
      int flag = 0;
      struct exec_context* ctx;
      if(current->type == EXEC_CTX_USER_TH){
        ctx = get_ctx_by_pid(current->ppid);
        flag = 0;
      }
      else{
        ctx = current;
        flag = 1;
      }

      struct thread req;
      int private;

      // parent process
      if(flag == 1){
        // find all offset
        int pgd_offset = (addr << 16) >> 55;
        int pud_offset = (addr << 25) >> 55;
        int pmd_offset = (addr << 34) >> 55;
        int pte_offset = (addr << 43) >> 55;

        // find all addresses using offset and alloc them if they are not present
        u64* pgd_addr = (u64 *)osmap(ctx->pgd) + pgd_offset;
        if((u64 *)(*(u64 *)pgd_addr) == NULL){
          *(u64 *)pgd_addr = (u64)osmap(os_pfn_alloc(OS_PT_REG));
          *(u64 *)pgd_addr = (u64)(((*(u64 *)pgd_addr) >> 12) << 12) | 0x7;
        }
        u64* pud_addr = (u64 *)(((*(u64 *)pgd_addr) >> 12) << 12) + pud_offset;
        if((u64 *)(*(u64 *)pud_addr) == NULL){
          *(u64 *)pud_addr = (u64)osmap(os_pfn_alloc(OS_PT_REG));
          *(u64 *)pud_addr = (u64)(((*(u64 *)pud_addr) >> 12) << 12) | 0x7;
        }
        u64* pmd_addr = (u64 *)(((*(u64 *)pud_addr) >> 12) << 12) + pmd_offset;
        if((u64 *)(*(u64 *)pmd_addr) == NULL){
          *(u64 *)pmd_addr = (u64)osmap(os_pfn_alloc(OS_PT_REG));
          *(u64 *)pmd_addr = (u64)(((*(u64 *)pmd_addr) >> 12) << 12) | 0x7;
        }
        u64* pte_addr = (u64 *)(((*(u64 *)pmd_addr) >> 12) << 12) + pte_offset;
        // allocating physical area
        if( (u64 *)(*(u64 *)pte_addr) == NULL ){
          u64 new_pfn = os_pfn_alloc(USER_REG);
          void *page = osmap(new_pfn);
          *(u64 *)pte_addr = (u64)page;
          *(u64 *)pte_addr = (u64)(((*(u64 *)pte_addr) >> 12) << 12) | 0x7;
        }
        return 1;
      }

      // thread is the process
      else if(flag == 0){

        // find this address belongs to which threads private mapping
        int thread_pid;
        int flag1 = 0;
        for(int i = 0; i < MAX_THREADS; i++){
          for(int j = 0; j < MAX_PRIVATE_AREAS; j++){
            if(addr < (ctx->ctx_threads->threads[i].private_mappings[j].start_addr + ctx->ctx_threads->threads[i].private_mappings[j].length )  &&  addr >= ctx->ctx_threads->threads[i].private_mappings[j].start_addr && ctx->ctx_threads->threads[i].private_mappings[j].owner != NULL){
              req = ctx->ctx_threads->threads[i];
              private = j;
              thread_pid = ctx->ctx_threads->threads[i].pid;
              flag1 = 1;
              break;
            }
          }
        }
        // belongs to no one
        if(!flag1){
          segfault_exit(current->pid, current->regs.entry_rip, addr);
          return -1;
        }

        // if current thread is the owner thread
        if(thread_pid == current->pid){
          // find all offset
          int pgd_offset = (addr << 16) >> 55;
          int pud_offset = (addr << 25) >> 55;
          int pmd_offset = (addr << 34) >> 55;
          int pte_offset = (addr << 43) >> 55;

          // find all addresses using offset and alloc them if they are not present
          u64* pgd_addr = (u64 *)osmap(ctx->pgd) + pgd_offset;
          if((u64 *)(*(u64 *)pgd_addr) == NULL){
            *(u64 *)pgd_addr = (u64)osmap(os_pfn_alloc(OS_PT_REG));
            *(u64 *)pgd_addr = (u64)(((*(u64 *)pgd_addr) >> 12) << 12) | 0x7;
          }
          u64* pud_addr = (u64 *)(((*(u64 *)pgd_addr) >> 12) << 12) + pud_offset;
          if((u64 *)(*(u64 *)pud_addr) == NULL){
            *(u64 *)pud_addr = (u64)osmap(os_pfn_alloc(OS_PT_REG));
            *(u64 *)pud_addr = (u64)(((*(u64 *)pud_addr) >> 12) << 12) | 0x7;
          }
          u64* pmd_addr = (u64 *)(((*(u64 *)pud_addr) >> 12) << 12) + pmd_offset;
          if((u64 *)(*(u64 *)pmd_addr) == NULL){
            *(u64 *)pmd_addr = (u64)osmap(os_pfn_alloc(OS_PT_REG));
            *(u64 *)pmd_addr = (u64)(((*(u64 *)pmd_addr) >> 12) << 12) | 0x7;
          }
          u64* pte_addr = (u64 *)(((*(u64 *)pmd_addr) >> 12) << 12) + pte_offset;
          // allocating physical area
          if( (u64 *)(*(u64 *)pte_addr) == NULL ){
            u64 new_pfn = os_pfn_alloc(USER_REG);
            void *page = osmap(new_pfn);
            *(u64 *)pte_addr = (u64 )page;
            *(u64 *)pte_addr = (u64)(((*(u64 *)pte_addr) >> 12 ) << 12 ) | 0x7;
          }
          return 1;
        }

        // if current thread is not the owner thread
        else{
          // if noaccess then segfault
          if(req.private_mappings[private].flags == 0x13){
            segfault_exit(current->pid, current->regs.entry_rip, addr);
            return -1;
          }

          // if error is due to read and we dont have read permission
          else if(error_code == 0x4 && req.private_mappings[private].flags != 0x23 && req.private_mappings[private].flags != 0x43){
            segfault_exit(current->pid, current->regs.entry_rip, addr);
            return -1;
          }

          // if read only access and error is due to write then segfault
          if(error_code == 0x6 && req.private_mappings[private].flags == 0x23){
            segfault_exit(current->pid, current->regs.entry_rip, addr);
            return -1;
          }

          // if permission is read only
          else if(req.private_mappings[private].flags == 0x23){
            // find all offset
            int pgd_offset = (addr << 16) >> 55;
            int pud_offset = (addr << 25) >> 55;
            int pmd_offset = (addr << 34) >> 55;
            int pte_offset = (addr << 43) >> 55;

            // find all addresses using offset and alloc them if they are not present
            u64* pgd_addr = (u64 *)osmap(ctx->pgd) + pgd_offset;
            if((u64 *)(*(u64 *)pgd_addr) == NULL){
              *(u64 *)pgd_addr = (u64)osmap(os_pfn_alloc(OS_PT_REG));
              *(u64 *)pgd_addr = (u64)(((*(u64 *)pgd_addr) >> 12) << 12) | 0x7;
            }
            u64* pud_addr = (u64 *)(((*(u64 *)pgd_addr) >> 12) << 12) + pud_offset;
            if((u64 *)(*(u64 *)pud_addr) == NULL){
              *(u64 *)pud_addr = (u64)osmap(os_pfn_alloc(OS_PT_REG));
              *(u64 *)pud_addr = (u64)(((*(u64 *)pud_addr) >> 12) << 12) | 0x7;
            }
            u64* pmd_addr = (u64 *)(((*(u64 *)pud_addr) >> 12) << 12) + pmd_offset;
            if((u64 *)(*(u64 *)pmd_addr) == NULL){
              *(u64 *)pmd_addr = (u64)osmap(os_pfn_alloc(OS_PT_REG));
              *(u64 *)pmd_addr = (u64)(((*(u64 *)pmd_addr) >> 12) << 12) | 0x7;
            }
            u64* pte_addr = (u64 *)(((*(u64 *)pmd_addr) >> 12) << 12) + pte_offset;
            // allocating physical area
            if( (u64 *)(*(u64 *)pte_addr) == NULL ){
              u64 new_pfn = os_pfn_alloc(USER_REG);
              void *page = osmap(new_pfn);
              *(u64 *)pte_addr = (u64)page;
              *(u64 *)pte_addr = (u64)(((*(u64 *)pte_addr) >> 12) << 12) | 0x5; // as only read permission so 101
            }
            return 1;
          }

          // if permission is both read/write
          else if(req.private_mappings[private].flags == 0x43){
            // find all offset
            int pgd_offset = (addr << 16) >> 55;
            int pud_offset = (addr << 25) >> 55;
            int pmd_offset = (addr << 34) >> 55;
            int pte_offset = (addr << 43) >> 55;

            // find all addresses using offset and alloc them if they are not present
            u64* pgd_addr = (u64 *)osmap(ctx->pgd) + pgd_offset;
            if((u64 *)(*(u64 *)pgd_addr) == NULL){
              *(u64 *)pgd_addr = (u64)osmap(os_pfn_alloc(OS_PT_REG));
              *(u64 *)pgd_addr = (u64)(((*(u64 *)pgd_addr) >> 12) << 12) | 0x7;
            }
            u64* pud_addr = (u64 *)(((*(u64 *)pgd_addr) >> 12) << 12) + pud_offset;
            if((u64 *)(*(u64 *)pud_addr) == NULL){
              *(u64 *)pud_addr = (u64)osmap(os_pfn_alloc(OS_PT_REG));
              *(u64 *)pud_addr = (u64)(((*(u64 *)pud_addr) >> 12) << 12) | 0x7;
            }
            u64* pmd_addr = (u64 *)(((*(u64 *)pud_addr) >> 12) << 12) + pmd_offset;
            if((u64 *)(*(u64 *)pmd_addr) == NULL){
              *(u64 *)pmd_addr = (u64)osmap(os_pfn_alloc(OS_PT_REG));
              *(u64 *)pmd_addr = (u64)(((*(u64 *)pmd_addr) >> 12) << 12) | 0x7;
            }
            u64* pte_addr = (u64 *)(((*(u64 *)pmd_addr) >> 12) << 12) + pte_offset;
            // allocating physical area
            if( (u64 *)(*(u64 *)pte_addr) == NULL ){
              u64 new_pfn = os_pfn_alloc(USER_REG);
              void *page = osmap(new_pfn);
              *(u64 *)pte_addr = (u64)page;
              *(u64 *)pte_addr = (u64)(((*(u64 *)pte_addr) >> 12) << 12) | 0x7; // as permission is both read/write so 111
            }
            return 1;
          }
        }
      }
      return -1;
    }
}

/*This is a handler called from scheduler. The 'current' refers to the outgoing context and the 'next' 
 * is the incoming context. Both of them can be either the parent process or one of the threads, but only
 * one of them can be the process (as we are having a system with a single user process). This handler
 * should apply the mapping rules passed in the gmalloc calls. */

int handle_private_ctxswitch(struct exec_context *current, struct exec_context *next)
{
  
   /* your implementation goes here*/
   // if one is null
   if(current == NULL || next == NULL){
     return -1;
   }

   // if current is parent and next is parent
   // this will return 0 at the end

   // if current is parent, next is thread
   if(current->type == EXEC_CTX_USER && next->type == EXEC_CTX_USER_TH ){
     struct exec_context* parent = get_ctx_by_pid(next->ppid);

     // flush all the tlb
     for(int i = 0; i < MAX_THREADS; i++){
       for(int j = 0; j<MAX_PRIVATE_AREAS; j++){
         if(parent->ctx_threads->threads[i].private_mappings[j].owner != NULL){
            long size = parent->ctx_threads->threads[i].private_mappings[j].length;
            int times = 0;
            while( size > 0 ){
              u64 clr = parent->ctx_threads->threads[i].private_mappings[j].start_addr + times*(1<<12);
              times += 1;
              size = size - (1<<12);

              asm volatile("invlpg (%0)" ::"r" (clr) : "memory");
            }
         }
       }
     }

     // which thread is next
     int thread_index;
     for(int i = 0; i<MAX_THREADS; i++){
       if(parent->ctx_threads->threads[i].pid == next->pid ){
         thread_index = i;
         break;
       }
     }

     // change permissions of all threads accoring to next
      for(int i = 0; i<MAX_THREADS; i++){
        for(int j=0; j<MAX_PRIVATE_AREAS; j++){
          if(parent->ctx_threads->threads[i].private_mappings[j].owner != NULL){
            long size = parent->ctx_threads->threads[i].private_mappings[j].length;
            int times = 0;
            while( size > 0 ){
              u64 addr = parent->ctx_threads->threads[i].private_mappings[j].start_addr + times*(1<<12);
              times += 1;
              size = size - (1<<12);

              int pgd_offset = (addr << 16) >> 55;
              int pud_offset = (addr << 25) >> 55;
              int pmd_offset = (addr << 34) >> 55;
              int pte_offset = (addr << 43) >> 55;

              u64* pgd_addr = (u64 *)osmap(current->pgd) + pgd_offset;
              if((u64 *)(*(u64 *)pgd_addr) == NULL){
                continue;
              }
              u64* pud_addr = (u64 *)(((*(u64 *)pgd_addr) >> 12) << 12) + pud_offset;
              if((u64 *)(*(u64 *)pud_addr) == NULL){
                continue;
              }
              u64* pmd_addr = (u64 *)(((*(u64 *)pud_addr) >> 12) << 12) + pmd_offset;
              if((u64 *)(*(u64 *)pmd_addr) == NULL){
                continue;
              }
              u64* pte_addr = (u64 *)(((*(u64 *)pmd_addr) >> 12) << 12) + pte_offset;
              if((u64 *)(*(u64 *)pte_addr) == NULL){
                continue;
              }
              if(parent->ctx_threads->threads[i].private_mappings[j].flags == 0x23){
                *(u64 *)pte_addr = (u64)(((*(u64 *)pte_addr) >> 12) << 12) | 0x5;
              }
              else if(parent->ctx_threads->threads[i].private_mappings[j].flags == 0x43){
                *(u64 *)pte_addr = (u64)(((*(u64 *)pte_addr) >> 12) << 12) | 0x7;
              }
              else if(parent->ctx_threads->threads[i].private_mappings[j].flags == 0x13){
                *(u64 *)pte_addr = (u64)(((*(u64 *)pte_addr) >> 12) << 12) | 0x1;
              }
            } 
          }
        }
      }

     // change permissions of next as it is going to be current
     for(int i = 0; i<MAX_PRIVATE_AREAS; i++){
       if(parent->ctx_threads->threads[thread_index].private_mappings[i].owner != NULL){
          long size = parent->ctx_threads->threads[thread_index].private_mappings[i].length;
          int times = 0;
          while( size > 0 ){
            u64 addr = parent->ctx_threads->threads[thread_index].private_mappings[i].start_addr + times*(1<<12);
            times += 1;
            size = size - (1<<12);

            int pgd_offset = (addr << 16) >> 55;
            int pud_offset = (addr << 25) >> 55;
            int pmd_offset = (addr << 34) >> 55;
            int pte_offset = (addr << 43) >> 55;

            u64* pgd_addr = (u64 *)osmap(next->pgd) + pgd_offset;
            if((u64 *)(*(u64 *)pgd_addr) == NULL){
              continue;
            }
            u64* pud_addr = (u64 *)(((*(u64 *)pgd_addr) >> 12) << 12) + pud_offset;
            if((u64 *)(*(u64 *)pud_addr) == NULL){
              continue;
            }
            u64* pmd_addr = (u64 *)(((*(u64 *)pud_addr) >> 12) << 12) + pmd_offset;
            if((u64 *)(*(u64 *)pmd_addr) == NULL){
              continue;
            }
            u64* pte_addr = (u64 *)(((*(u64 *)pmd_addr) >> 12) << 12) + pte_offset;
            if((u64 *)(*(u64 *)pte_addr) == NULL){
              continue;
            }
            *(u64 *)pte_addr = (u64)(((*(u64 *)pte_addr) >> 12) << 12) | 0x7;  
          }       
        }
      }
   }





   // if current is thread, next is parent
   if(current->type == EXEC_CTX_USER_TH && next->type == EXEC_CTX_USER ){
     struct exec_context* parent = get_ctx_by_pid(current->ppid);

     // flush all the tlb
     for(int i = 0; i < MAX_THREADS; i++){
       for(int j = 0; j<MAX_PRIVATE_AREAS; j++){
          if(parent->ctx_threads->threads[i].private_mappings[j].owner != NULL){
            long size = parent->ctx_threads->threads[i].private_mappings[j].length;
            int times = 0;
            while( size > 0 ){
              u64 clr = parent->ctx_threads->threads[i].private_mappings[j].start_addr + times*(1<<12);
              times += 1;
              size = size - (1<<12);

              asm volatile("invlpg (%0)" ::"r" (clr) : "memory");
            }
          }
       }
     }

     // set all the flags accordingly
      for(int i = 0; i<MAX_THREADS; i++){
        for(int j=0; j<MAX_PRIVATE_AREAS; j++){
          if(parent->ctx_threads->threads[i].private_mappings[j].owner != NULL){
            long size = parent->ctx_threads->threads[i].private_mappings[j].length;
            int times = 0;
            while( size > 0 ){
              u64 addr = parent->ctx_threads->threads[i].private_mappings[j].start_addr + times*(1<<12);
              times += 1;
              size = size - (1<<12);

              int pgd_offset = (addr << 16) >> 55;
              int pud_offset = (addr << 25) >> 55;
              int pmd_offset = (addr << 34) >> 55;
              int pte_offset = (addr << 43) >> 55;

              u64* pgd_addr = (u64 *)osmap(current->pgd) + pgd_offset;
              if((u64 *)(*(u64 *)pgd_addr) == NULL){
                continue;
              }
              u64* pud_addr = (u64 *)(((*(u64 *)pgd_addr) >> 12) << 12) + pud_offset;
              if((u64 *)(*(u64 *)pud_addr) == NULL){
                continue;
              }
              u64* pmd_addr = (u64 *)(((*(u64 *)pud_addr) >> 12) << 12) + pmd_offset;
              if((u64 *)(*(u64 *)pmd_addr) == NULL){
                continue;
              }
              u64* pte_addr = (u64 *)(((*(u64 *)pmd_addr) >> 12) << 12) + pte_offset;
              if((u64 *)(*(u64 *)pte_addr) == NULL){
                continue;
              }
            
              *(u64 *)pte_addr = (u64)(((*(u64 *)pte_addr) >> 12) << 12) | 0x7;
            }
          }
        }
      }
   }




   // if current is thread, next is thread 
   if(current->type == EXEC_CTX_USER_TH && next->type == EXEC_CTX_USER_TH ){
     struct exec_context *parent = get_ctx_by_pid(current->ppid);
     int currentindex, nextindex;
     for(int i=0; i<MAX_THREADS; i++){
       if(parent->ctx_threads->threads[i].pid == current->pid){
         currentindex = i;
       }
       if(parent->ctx_threads->threads[i].pid == next->pid){
         nextindex = i;
       }
     }

     // flush the tlb for both the threads
     for(int i= 0; i<MAX_PRIVATE_AREAS; i++){
        if(parent->ctx_threads->threads[currentindex].private_mappings[i].owner != NULL){
          long size = parent->ctx_threads->threads[currentindex].private_mappings[i].length;
          int times = 0;
          while( size > 0 ){
            u64 clr = parent->ctx_threads->threads[currentindex].private_mappings[i].start_addr + times*(1<<12);
            times += 1;
            size = size - (1<<12);

            asm volatile("invlpg (%0)" ::"r" (clr) : "memory");
          }
        }
      }
      for(int i = 0; i<MAX_PRIVATE_AREAS; i++){
        if(parent->ctx_threads->threads[nextindex].private_mappings[i].owner != NULL){
          long size = parent->ctx_threads->threads[nextindex].private_mappings[i].length;
          int times = 0;
          while( size > 0 ){
            u64 clr = parent->ctx_threads->threads[nextindex].private_mappings[i].start_addr + times*(1<<12);
            times += 1;
            size = size - (1<<12);

            asm volatile("invlpg (%0)" ::"r" (clr) : "memory");
          }
        }
      }
     
     // change the permissions of current according to next
     for(int i = 0; i<MAX_PRIVATE_AREAS; i++){
       if(parent->ctx_threads->threads[currentindex].private_mappings[i].owner != NULL){
          long size = parent->ctx_threads->threads[currentindex].private_mappings[i].length;
          int times = 0;
          while( size > 0 ){
            u64 addr = parent->ctx_threads->threads[currentindex].private_mappings[i].start_addr + times*(1<<12);
            times += 1;
            size = size - (1<<12);

            int pgd_offset = (addr << 16) >> 55;
            int pud_offset = (addr << 25) >> 55;
            int pmd_offset = (addr << 34) >> 55;
            int pte_offset = (addr << 43) >> 55;

            u64* pgd_addr = (u64 *)osmap(current->pgd) + pgd_offset;
            if((u64 *)(*(u64 *)pgd_addr) == NULL){
              continue;
            }
            u64* pud_addr = (u64 *)(((*(u64 *)pgd_addr) >> 12) << 12) + pud_offset;
            if((u64 *)(*(u64 *)pud_addr) == NULL){
              continue;
            }
            u64* pmd_addr = (u64 *)(((*(u64 *)pud_addr) >> 12) << 12) + pmd_offset;
            if((u64 *)(*(u64 *)pmd_addr) == NULL){
              continue;
            }
            u64* pte_addr = (u64 *)(((*(u64 *)pmd_addr) >> 12) << 12) + pte_offset;
            if((u64 *)(*(u64 *)pte_addr) == NULL){
              continue;
            }
            if(parent->ctx_threads->threads[currentindex].private_mappings[i].flags == 0x23){
              *(u64 *)pte_addr = (u64)(((*(u64 *)pte_addr) >> 12) << 12) | 0x5;
            }
            else if(parent->ctx_threads->threads[currentindex].private_mappings[i].flags == 0x43){
              *(u64 *)pte_addr = (u64)(((*(u64 *)pte_addr) >> 12) << 12) | 0x7;
            }
            else if(parent->ctx_threads->threads[currentindex].private_mappings[i].flags == 0x13){
              *(u64 *)pte_addr = (u64)(((*(u64 *)pte_addr) >> 12) << 12) | 0x1;
            }
          }
        }
      }


     // change permissions of next as it is going to become current
     for(int i = 0; i<MAX_PRIVATE_AREAS; i++){
       if(parent->ctx_threads->threads[nextindex].private_mappings[i].owner != NULL){
          long size = parent->ctx_threads->threads[nextindex].private_mappings[i].length;
          int times = 0;
          while( size > 0 ){
            u64 addr = parent->ctx_threads->threads[nextindex].private_mappings[i].start_addr + times*(1<<12);
            times += 1;
            size = size - (1<<12);

            int pgd_offset = (addr << 16) >> 55;
            int pud_offset = (addr << 25) >> 55;
            int pmd_offset = (addr << 34) >> 55;
            int pte_offset = (addr << 43) >> 55;

            u64* pgd_addr = (u64 *)osmap(next->pgd) + pgd_offset;
            if((u64 *)(*(u64 *)pgd_addr) == NULL){
              continue;
            }
            u64* pud_addr = (u64 *)(((*(u64 *)pgd_addr) >> 12) << 12) + pud_offset;
            if((u64 *)(*(u64 *)pud_addr) == NULL){
              continue;
            }
            u64* pmd_addr = (u64 *)(((*(u64 *)pud_addr) >> 12) << 12) + pmd_offset;
            if((u64 *)(*(u64 *)pmd_addr) == NULL){
              continue;
            }
            u64* pte_addr = (u64 *)(((*(u64 *)pmd_addr) >> 12) << 12) + pte_offset;
            if((u64 *)(*(u64 *)pte_addr) == NULL){
              continue;
            }
            *(u64 *)pte_addr = (u64)(((*(u64 *)pte_addr) >> 12) << 12) | 0x7;   
          }      
        }
      }
   }
   
   return 0;	

}

