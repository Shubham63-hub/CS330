#include <debug.h>
#include <context.h>
#include <entry.h>
#include <lib.h>
#include <memory.h>
 

/*****************************HELPERS******************************************/
 
/*
 * allocate the struct which contains information about debugger
 *
 */
struct debug_info *alloc_debug_info()
{
	struct debug_info *info = (struct debug_info *) os_alloc(sizeof(struct debug_info));
	if(info)
		bzero((char *)info, sizeof(struct debug_info));
	return info;
}
/*
 * frees a debug_info struct
 */
void free_debug_info(struct debug_info *ptr)
{
	if(ptr)
		os_free((void *)ptr, sizeof(struct debug_info));
}



/*
 * allocates a page to store registers structure
 */
struct registers *alloc_regs()
{
	struct registers *info = (struct registers*) os_alloc(sizeof(struct registers));
	if(info)
		bzero((char *)info, sizeof(struct registers));
	return info;
}

/*
 * frees an allocated registers struct
 */
void free_regs(struct registers *ptr)
{
	if(ptr)
		os_free((void *)ptr, sizeof(struct registers));
}

/*
 * allocate a node for breakpoint list
 * which contains information about breakpoint
 */
struct breakpoint_info *alloc_breakpoint_info()
{
	struct breakpoint_info *info = (struct breakpoint_info *)os_alloc(
		sizeof(struct breakpoint_info));
	if(info)
		bzero((char *)info, sizeof(struct breakpoint_info));
	return info;
}

/*
 * frees a node of breakpoint list
 */
void free_breakpoint_info(struct breakpoint_info *ptr)
{
	if(ptr)
		os_free((void *)ptr, sizeof(struct breakpoint_info));
}

/*
 * Fork handler.
 * The child context doesnt need the debug info
 * Set it to NULL
 * The child must go to sleep( ie move to WAIT state)
 * It will be made ready when the debugger calls wait
 */
void debugger_on_fork(struct exec_context *child_ctx)
{
	// printk("DEBUGGER FORK HANDLER CALLED\n");
	child_ctx->dbg = NULL;
	child_ctx->state = WAITING;
}


/******************************************************************************/


/* This is the int 0x3 handler
 * Hit from the childs context
 */

long int3_handler(struct exec_context *ctx)
{
	
	//Your code
	if(ctx == NULL) return -1;

	// printk("int3 called\n");
	struct exec_context *parent;
	parent = get_ctx_by_pid(ctx->ppid);

	// int3 called from end of a function
	if(ctx->regs.entry_rip-1 == (u64)parent->dbg->end_handler){

		// orignal return address
		ctx->regs.entry_rsp -= 8;
		*(u64 *)ctx->regs.entry_rsp = parent->dbg->removed_addr[parent->dbg->index-1];

		// push rbp
		ctx->regs.entry_rsp -= 8;
		*((u64 *)ctx->regs.entry_rsp) = (u64)(ctx->regs.rbp);

		// offset
		parent->dbg->rsp_offset = 8;

		// parent return if breakpoint is hit
		parent->regs.rax = ctx->regs.entry_rip-1;

		int flag = 0;
		struct breakpoint_info *temp = parent->dbg->head;
		while(temp != NULL){
			if(temp->addr == parent->dbg->currently_running[parent->dbg->index-1]){
				flag = 1;
				break;
			}
			temp = temp->next;
		}
		temp->inside_int -= 1;
		parent->dbg->currently_running[parent->dbg->index-1] = 0;
		parent->dbg->index = parent->dbg->index - 1;

		// backtrace
		int i = 0;
		while(i<MAX_BACKTRACE){
			parent->dbg->info_backtrace[i] = 0;
			i++;
		}
		i=0;
		//rbp
		u64 rbp = ctx->regs.entry_rsp;
		//first return 
		u64 fir_ret = *(u64 *)(rbp + 8);

		int j = parent->dbg->index - 1;

		// main return address is END_ADDR
		while( fir_ret != END_ADDR ){
			if( i >= MAX_BACKTRACE) break;
			if(fir_ret == (u64)parent->dbg->end_handler){
				parent->dbg->info_backtrace[i++] = parent->dbg->removed_addr[j--];
				rbp	 = *((u64 *) rbp);
				fir_ret = *(u64 *)(rbp + 8);
			}
			else{
				parent->dbg->info_backtrace[i++] = fir_ret;
				rbp	 = *((u64 *) rbp);
				fir_ret = *(u64 *)(rbp + 8);
			}
		}
		
		ctx->state = WAITING;
		parent->state = READY;
		schedule(parent);
	
		return 0;
	}

	// int3 called from starting of a function
	int flag = 0;
	struct breakpoint_info *temp = parent->dbg->head;
	while(temp != NULL){
		if(temp->addr == ctx->regs.entry_rip-1){
			flag = 1;
			break;
		}
		temp = temp->next;
	}
	
	if(!flag) return -1;

	flag = 0;
	if(temp->end_breakpoint_enable == 1){
		// store the address which is going to be overwritted
		flag = 1;
		parent->dbg->removed_addr[parent->dbg->index] = *(u64 *)ctx->regs.entry_rsp;

		// returns to the do_end_handler
		*((u64 *)ctx->regs.entry_rsp) = (u64)(parent->dbg->end_handler);

		temp->inside_int += 1;
		parent->dbg->currently_running[parent->dbg->index] = ctx->regs.entry_rip-1;
		parent->dbg->index = parent->dbg->index + 1;
	}
	
	// push rbp
	ctx->regs.entry_rsp -= 8;
	*((u64 *)ctx->regs.entry_rsp) = (u64)(ctx->regs.rbp);

	// offset
	parent->dbg->rsp_offset = 8;

	//parent return if breakpoint is hit
	parent->regs.rax = ctx->regs.entry_rip - 1;

	// backtrace
	int i = 0;
	while(i<MAX_BACKTRACE){
		parent->dbg->info_backtrace[i] = 0;
		i++;
	}
	i=0;
	//rbp
	u64 rbp = ctx->regs.entry_rsp;
	//first return 
	u64 fir_ret = *(u64 *)(rbp + 8);

	// first will be the function address
	parent->dbg->info_backtrace[i++] = ctx->regs.entry_rip - 1;

	int j = parent->dbg->index - 1;

	// main return address is END_ADDR
	while( fir_ret != END_ADDR ){
		if( i >= MAX_BACKTRACE) break;
		if(fir_ret == (u64)parent->dbg->end_handler){
			parent->dbg->info_backtrace[i++] = parent->dbg->removed_addr[j--];
			rbp	 = *((u64 *) rbp);
			fir_ret = *(u64 *)(rbp + 8);
		}
		else{
			parent->dbg->info_backtrace[i++] = fir_ret;
			rbp	 = *((u64 *) rbp);
			fir_ret = *(u64 *)(rbp + 8);
		}
	}

	ctx->state = WAITING;
	parent->state = READY;
	schedule(parent);

	return 0;
}

/*
 * Exit handler.
 * Deallocate the debug_info struct if its a debugger.
 * Wake up the debugger if its a child
 */
void debugger_on_exit(struct exec_context *ctx)
{
	// Your code
	if(ctx == NULL) return;

	//child exiting
	if(ctx->dbg == NULL){
		int parentpid = ctx->ppid;
		struct exec_context *parent = get_ctx_by_pid(parentpid);
		parent->state = READY;

		// telling parent child exited
		parent->regs.rax = CHILD_EXIT;
	}

	// debugger exiting
	else{
		struct breakpoint_info *temp = ctx->dbg->head;
		struct breakpoint_info *temp1 = ctx->dbg->head;
		
		while(temp != NULL){
			temp1 = temp;
			temp = temp->next;
			free_breakpoint_info(temp1);
		}
		free_debug_info(ctx->dbg);
	}
}


/*
 * called from debuggers context
 * initializes debugger state
 */
int do_become_debugger(struct exec_context *ctx, void *addr)
{
	// Your code
	// initialise all variables for debug info
	if(ctx == NULL) return -1;
	
	ctx->dbg = alloc_debug_info();
	if(ctx->dbg == NULL) return -1;

	ctx->dbg->breakpoint_count = 0;
	ctx->dbg->head = NULL;

	// do end handler address such that its first instruction is calling int3 handler
	*(u8 *)addr = INT3_OPCODE;
	ctx->dbg->end_handler = addr;

	for(int i = 0; i < MAX_BACKTRACE; i++){
		ctx->dbg->info_backtrace[i] = 0;
	}

	ctx->dbg->number_count = 1;
	ctx->dbg->index = 0;
	ctx->dbg->rsp_offset = 0;
	
	return 0;
}

/*
 * called from debuggers context
 */
int do_set_breakpoint(struct exec_context *ctx, void *addr, int flag)
{
	// Your code
	if(ctx == NULL) return -1;

	struct breakpoint_info *temp = ctx->dbg->head;

	if(MAX_BREAKPOINTS <= 0){
		return -1;
	}

	// no breakpoint present
	if(temp == NULL){
		struct breakpoint_info *new = alloc_breakpoint_info();
		if(new == NULL) return -1;

		new->num = ctx->dbg->number_count;
		ctx->dbg->number_count = ctx->dbg->number_count + 1;

		// instruction is of 1(8 bits) byte (objdump)
		*((u8 *)addr) = INT3_OPCODE;
		new->addr = (u64)addr;

		new->end_breakpoint_enable = flag;
		new->inside_int = 0;

		temp = new;
		temp->next = NULL;

		ctx->dbg->breakpoint_count = ctx->dbg->breakpoint_count + 1;
		ctx->dbg->head = temp;
		return 0;
	}

	//if breakpoint is already present
	while(temp->next != NULL){
		if(temp->addr == (u64)addr){
			if(temp->end_breakpoint_enable == 0){
				temp->end_breakpoint_enable = flag;
				return 0;
			}
			if(flag == 1 && temp->end_breakpoint_enable == 1) return 0;
			else if(temp->inside_int <= 0){
				temp->end_breakpoint_enable = flag;
				return 0;
			}
			else if(temp->inside_int > 0) return -1;
		}
		temp = temp->next;
	}

	//last node remaining
	if(temp->addr == (u64)addr){
		if(temp->end_breakpoint_enable == 0){
			temp->end_breakpoint_enable = flag;
			return 0;
		}
		if(flag == 1 && temp->end_breakpoint_enable == 1) return 0;
		else if(temp->inside_int <= 0){
			temp->end_breakpoint_enable = flag;
			return 0;
		}
		else if(temp->inside_int > 0) return -1;
	}

	// return -1
	if(ctx->dbg->breakpoint_count >= MAX_BREAKPOINTS){
		return -1;
	}

	//new breakpoint
	struct breakpoint_info *new = alloc_breakpoint_info();
	if(new == NULL) return -1;

	new->num = ctx->dbg->number_count;
	ctx->dbg->number_count = ctx->dbg->number_count + 1;

	// instruction is of 1(8 bits) byte (objdump)
	*((u8 *)addr) = INT3_OPCODE;
	new->addr = (u64)addr;

	new->end_breakpoint_enable = flag;
	new->inside_int = 0;
	temp->next = new;
	temp->next->next = NULL;

	ctx->dbg->breakpoint_count = ctx->dbg->breakpoint_count + 1;
	
	return 0;
}

/*
 * called from debuggers context
 */
int do_remove_breakpoint(struct exec_context *ctx, void *addr)
{
	//Your code
	if(ctx == NULL) return -1;

	struct breakpoint_info *temp = ctx->dbg->head;
	
	// if we need to remove head
	if(temp->addr == (u64)addr){
		if(temp->end_breakpoint_enable == 0){
			// int3 doesnt get called after removing breakpoint
			*((u8 *)addr) = PUSHRBP_OPCODE;

			ctx->dbg->head = temp->next;
			free_breakpoint_info(temp);
			ctx->dbg->breakpoint_count = ctx->dbg->breakpoint_count - 1;
			return 0;
		}
		if(temp->inside_int > 0) return -1;

		// int3 doesnt get called
		*((u8 *)addr) = PUSHRBP_OPCODE;

		ctx->dbg->head = temp->next;
		free_breakpoint_info(temp);
		ctx->dbg->breakpoint_count = ctx->dbg->breakpoint_count - 1;
		return 0;
	}

	// remove a middle element
	struct breakpoint_info *prev = temp;
	temp = temp->next;

	int flag = 0;
	while(temp != NULL){
		if(temp->addr == (u64)addr){
			flag = 1;
			break;
		}
		prev = prev->next;
		temp = temp->next;
	}
	// addr breakpoint is not present
	if(!flag){
		return -1;
	}
	if(temp->end_breakpoint_enable == 0){
		// change first instruction to default again
		*((u8 *)addr) = PUSHRBP_OPCODE;

		// present 
		prev->next = temp->next;
		free_breakpoint_info(temp);
		ctx->dbg->breakpoint_count = ctx->dbg->breakpoint_count - 1;

		return 0;
	}
	if(temp->inside_int > 0){
		return -1;
	}

	// change first instruction to default again
	*((u8 *)addr) = PUSHRBP_OPCODE;

	// present 
	prev->next = temp->next;
	free_breakpoint_info(temp);
	ctx->dbg->breakpoint_count = ctx->dbg->breakpoint_count - 1;

	return 0;
}


/*
 * called from debuggers context
 */

int do_info_breakpoints(struct exec_context *ctx, struct breakpoint *ubp)
{
	
	// Your code
	if(ctx == NULL) return -1;

	struct breakpoint_info* temp = ctx->dbg->head;
	int i = 0;

	// store values
	while(temp != NULL){
		ubp[i].num = temp->num;
		ubp[i].end_breakpoint_enable = temp->end_breakpoint_enable;
		ubp[i].addr = temp->addr;
		temp = temp->next;
		i++;
	}

	// sort values 
	for(int j = 0; j<i; j++){
		int min = j;
		for(int k = j+1; k<i; k++){
			if(ubp[k].num < ubp[min].num){
				min = k;
			}
		}
		if(min == j) continue;
		struct breakpoint temp1;
		temp1 = ubp[j];
		ubp[j] = ubp[min];
		ubp[min] = temp1;
	}
	return i;
}


/*
 * called from debuggers context
 */
int do_info_registers(struct exec_context *ctx, struct registers *regs)
{
	// Your code
	if(ctx == NULL) return -1;

	// child
	struct exec_context* child;
	int i = 1;
	int flag = 0;
	
	while(i <= MAX_PROCESSES){
		struct exec_context *temp = get_ctx_by_pid(i);
		if(ctx->pid == temp->ppid){
			flag = 1;
			break;
		}
		i++;
	}
	if(!flag) return -1;

	// assign
	child = get_ctx_by_pid(i);
	regs->entry_rip = child->regs.entry_rip - 1;
	regs->entry_rsp = child->regs.entry_rsp + ctx->dbg->rsp_offset;
	regs->rbp = child->regs.rbp;
	regs->rax = child->regs.rax;
	regs->rdi = child->regs.rdi;
	regs->rsi = child->regs.rsi;
	regs->rdx = child->regs.rdx;
	regs->rcx = child->regs.rcx;
	regs->r8 = child->regs.r8;
	regs->r9 = child->regs.r9;
	return 0;
}

/*
 * Called from debuggers context
 */
int do_backtrace(struct exec_context *ctx, u64 bt_buf)
{

	// Your code
	if(ctx == NULL) return -1;

	// array pointer
	u64 *return_array = (u64 *)bt_buf;

	// passing backtrace values
	int i = 0;
	while(ctx->dbg->info_backtrace[i] != 0){
		return_array[i] = ctx->dbg->info_backtrace[i];
		i++;
	}
	return i;
}

/*
 * When the debugger calls wait
 * it must move to WAITING state
 * and its child must move to READY state
 */

s64 do_wait_and_continue(struct exec_context *ctx)
{
	// Your Code
	if(ctx == NULL) return -1;

	// child process
	struct exec_context *child;
	int i = 1;
	int flag = 0;

	// find child process
	while( i <= MAX_PROCESSES){
		struct exec_context *temp = get_ctx_by_pid(i);
		if(temp->ppid == ctx->pid){
			flag = 1;
			break;
		}
		i++;
	}
	if(!flag) return -1;

	// set states
	child = get_ctx_by_pid(i);
	if(child == NULL) return -1;
	child->state = READY;
	ctx->state = WAITING;
	schedule(child);

	// if schedule fails
	return -1;
}
