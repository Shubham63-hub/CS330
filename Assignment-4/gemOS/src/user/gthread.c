#include <gthread.h>
#include <ulib.h>

static struct process_thread_info tinfo __attribute__((section(".user_data"))) = {};
/*XXX 
      Do not modifiy anything above this line. The global variable tinfo maintains user
      level accounting of threads. Refer gthread.h for definition of related structs.
 */

// my function
void helper_function(){
	// getting the rax value to the implicit exit thread.
	u64 ret;
	asm("mov %%rax, %0" 
		: "=r"(ret) 
		:);

	// finding which thread is exiting
	int pid = getpid();
	int i = 0,flag = 0;
	for(i = 0 ; i < MAX_THREADS; i++){
		if(tinfo.threads[i].pid == pid && tinfo.threads[i].status == TH_STATUS_USED){
			flag = 1;
			break;
		}
	}
	if(!flag){
		exit(0);
	}

	// return it as return address
	tinfo.threads[i].ret_addr = (void *)ret;
	tinfo.threads[i].status = TH_STATUS_ALIVE;
	exit(0);
}


/* Returns 0 on success and -1 on failure */
/* Here you can use helper system call "make_thread_ready" for your implementation */
int gthread_create(int *tid, void *(*fc)(void *), void *arg) {
      
	/* You need to fill in your implementation here*/

	// checking for a unused thread
	int i = 0;
	while(tinfo.threads[i].status == TH_STATUS_USED || tinfo.threads[i].status == TH_STATUS_ALIVE){
		i++;
	}
	if( i >= MAX_THREADS){
		return -1;
	}

	// allocating the stack to the thread
	void *stack_pointer = mmap(NULL, TH_STACK_SIZE, PROT_READ|PROT_WRITE, 0);
	if(!stack_pointer || stack_pointer == MAP_ERR){
        return -1;
 	}
	long thread_pid = clone(fc, ((u64)stack_pointer) + TH_STACK_SIZE - 8, arg); // size if -8 as we push helper_function in stack
	if(thread_pid == -1) {
		return -1;
	}

	// updating thread info
	tinfo.num_threads += 1;
	*tid = i;
	tinfo.threads[i].tid = i;
	tinfo.threads[i].pid = thread_pid;
	tinfo.threads[i].status = TH_STATUS_USED;
	tinfo.threads[i].stack_addr = stack_pointer; // + TH_STACK_SIZE - 8 to point to base of the stack

	// for implicit exit
	u64 *new_addr = (u64 *)((u64)stack_pointer + TH_STACK_SIZE );
	new_addr -= 1; // empty 8 bytes in stack
	*(u64 *)new_addr = (u64)helper_function;

	// ready thread
	make_thread_ready(thread_pid);

	return 0;
}

int gthread_exit(void *retval) {

	/* You need to fill in your implementation here*/

	// which thread is running
	long thread_pid = getpid();
	int i, flag;
	for( i = 0 ; i < MAX_THREADS ; i++){
		if(tinfo.threads[i].pid == thread_pid && tinfo.threads[i].status == TH_STATUS_USED){
			flag = 1;
			break;
		}
	}
	if(!flag){
		return -1;
	}

	// return address = retval and status
	tinfo.threads[i].ret_addr = retval;
	tinfo.threads[i].status = TH_STATUS_ALIVE;

	//call exit
	exit(0);
}

void* gthread_join(int tid) {
  
     /* Here you can use helper system call "wait_for_thread" for your implementation */   
     /* You need to fill in your implementation here*/

	 // which tid thread
	 int i = 0;
	 while(tinfo.threads[i].tid != tid){
		 i++;
	 }
	
	 if(i >= MAX_THREADS){
		 return NULL;
	 }

	 // error condition if thread is already joined
	 if(tinfo.threads[i].status != TH_STATUS_USED && tinfo.threads[i].status != TH_STATUS_ALIVE){
		 return NULL;
	 }
	
	 // wait for thread
	 long pid = tinfo.threads[i].pid;
	 if(tinfo.threads[i].status != TH_STATUS_ALIVE){
		while(tinfo.threads[i].status == TH_STATUS_USED){
			int k =	wait_for_thread(pid);
			if(k < 0){
				break;
			}
		}
	 }

	 // return address
	 void* ret = tinfo.threads[i].ret_addr;

	 // change in the data structure
	 tinfo.num_threads -= 1;
	 tinfo.threads[i].status = TH_STATUS_UNUSED;
	 tinfo.threads[i].ret_addr = NULL;
	 tinfo.threads[i].pid = 0;

	 // unmap the stack
	 munmap(tinfo.threads[i].stack_addr, TH_STACK_SIZE);

	 return ret;
	 
}


/*Only threads will invoke this. No need to check if its a process
 * The allocation size is always < GALLOC_MAX and flags can be one
 * of the alloc flags (GALLOC_*) defined in gthread.h. Need to 
 * invoke mmap using the proper protection flags (for prot param to mmap)
 * and MAP_TH_PRIVATE as the flag param of mmap. The mmap call will be 
 * handled by handle_thread_private_map in the OS.
 * */

void* gmalloc(u32 size, u8 alloc_flag)
{
   
	/* You need to fill in your implementation here*/

	// finding index of thread with given pid
	int pid = getpid();
	int i = 0 , flag = 0;
	for(i = 0; i < MAX_THREADS; i++){
		if(tinfo.threads[i].pid == pid && tinfo.threads[i].status == TH_STATUS_USED){
			flag = 1;
			break;
		}
	}
	if(!flag){
		return NULL;
	}

	// ownonly 
	if(alloc_flag == GALLOC_OWNONLY){
		// finding which private mapping is free
		int k = 0;
		while(tinfo.threads[i].priv_areas[k].owner != NULL){
			k++;
		}
		if(k >= MAX_GALLOC_AREAS){
			return NULL;
		}
		void *memory_pointer = mmap(NULL, size, PROT_READ|PROT_WRITE|TP_SIBLINGS_NOACCESS, MAP_TH_PRIVATE);
		if(!memory_pointer || memory_pointer == MAP_ERR){
        	return NULL;
 		}
		
		tinfo.threads[i].priv_areas[k].owner = tinfo.threads + i;
		tinfo.threads[i].priv_areas[k].start = (u64)memory_pointer;
		tinfo.threads[i].priv_areas[k].length = size;
		tinfo.threads[i].priv_areas[k].flags = PROT_READ|PROT_WRITE|TP_SIBLINGS_NOACCESS;
		return memory_pointer;
	}

	// read only for others
	else if(alloc_flag == GALLOC_OTRDONLY){
		// finding which private mapping is free
		int k = 0;
		while(tinfo.threads[i].priv_areas[k].owner != NULL){
			k++;
		}
		if(k >= MAX_GALLOC_AREAS){
			return NULL;
		}
		void *memory_pointer = mmap(NULL, size, PROT_READ|PROT_WRITE|TP_SIBLINGS_RDONLY, MAP_TH_PRIVATE);
		if(!memory_pointer || memory_pointer == MAP_ERR){
        	return NULL;
 		}
		
		tinfo.threads[i].priv_areas[k].owner = tinfo.threads + i;
		tinfo.threads[i].priv_areas[k].start = (u64)memory_pointer;
		tinfo.threads[i].priv_areas[k].length = size;
		tinfo.threads[i].priv_areas[k].flags = PROT_READ|PROT_WRITE|TP_SIBLINGS_RDONLY;
		return memory_pointer;
	}

	// read write both permissions for others
	else if(alloc_flag == GALLOC_OTRDWR){
		// finding which private mapping is free
		int k = 0;
		while(tinfo.threads[i].priv_areas[k].owner != NULL){
			k++;
		}
		if(k >= MAX_GALLOC_AREAS){
			return NULL;
		}
		void *memory_pointer = mmap(NULL, size, PROT_READ|PROT_WRITE|TP_SIBLINGS_RDWR, MAP_TH_PRIVATE);
		if(!memory_pointer || memory_pointer == MAP_ERR){
        	return NULL;
 		}
		
		tinfo.threads[i].priv_areas[k].owner = tinfo.threads + i;
		tinfo.threads[i].priv_areas[k].start = (u64)memory_pointer;
		tinfo.threads[i].priv_areas[k].length = size;
		tinfo.threads[i].priv_areas[k].flags = PROT_READ|PROT_WRITE|TP_SIBLINGS_RDWR;
		return memory_pointer;
	}
	else{
		return NULL;
	}
}
/*
   Only threads will invoke this. No need to check if the caller is a process.
*/
int gfree(void *ptr)
{
   
    /* You need to fill in your implementation here*/
	// finding thread using pid
	int pid = getpid();
	int i , flag = 0;
	for( i = 0; i< MAX_THREADS; i++){
		if(tinfo.threads[i].pid == pid && (tinfo.threads[i].status == TH_STATUS_USED || tinfo.threads[i].status == TH_STATUS_ALIVE)){
			flag = 1;
			break;
		}
	}
	if(!flag){
		return -1;
	}

	// finding in which private area it exists
	flag = 0;
	int k;
	for(k = 0; k < MAX_GALLOC_AREAS; k++){
		if((tinfo.threads[i].priv_areas[k].start == (u64)ptr) && (tinfo.threads[i].priv_areas[k].owner == tinfo.threads + i)){
			flag = 1;
			break;
		}
	}
	if(!flag){
		return -1;
	}

	// setting owner again to NULL and length to 0
	tinfo.threads[i].priv_areas[k].owner = NULL;
	tinfo.threads[i].priv_areas[k].length = 0;
	munmap((void *)tinfo.threads[i].priv_areas[k].start, tinfo.threads[i].priv_areas[k].length);

    return 0;
}
