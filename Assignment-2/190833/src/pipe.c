#include<pipe.h>
#include<context.h>
#include<memory.h>
#include<lib.h>
#include<entry.h>
#include<file.h>


// Per process info for the pipe.
struct pipe_info_per_process {

    // TODO:: Add members as per your need...
    int used;           //1 is used, -1 is not used 
    unsigned int pid;   //pid of the process
    int read_state;     //1 if can read else -1 
    int write_state;    //1 if can write else -1
};

// Global information for the pipe.
struct pipe_info_global {

    char *pipe_buff;    // Pipe buffer: DO NOT MODIFY THIS.

    // TODO:: Add members as per your need...
    int read_pointer;   //points to the point till which read is done(excluding the point)
    int write_pointer;  //points to the point till which write is done(excluding the point)
    int buffempty;      //how much buff is empty so that we can properly read and write

};

// Pipe information structure.
// NOTE: DO NOT MODIFY THIS STRUCTURE.
struct pipe_info {

    struct pipe_info_per_process pipe_per_proc [MAX_PIPE_PROC];
    struct pipe_info_global pipe_global;

};


// Function to allocate space for the pipe and initialize its members.
struct pipe_info* alloc_pipe_info () {
	
    // Allocate space for pipe structure and pipe buffer.
    struct pipe_info *pipe = (struct pipe_info*)os_page_alloc(OS_DS_REG);
    char* buffer = (char*) os_page_alloc(OS_DS_REG);

    /**
     *  TODO:: Initializing pipe fields
     *  
     *  Initialize per process fields for this pipe.
     *  Initialize global fields for this pipe.
     *
     */
    
    // Assign pipe buffer.
    pipe->pipe_global.pipe_buff = buffer;

    //assign pointers
    pipe->pipe_global.read_pointer = 0; 
    pipe->pipe_global.write_pointer = 0; //didn't wrote anything at the current point
    pipe->pipe_global.buffempty = MAX_PIPE_SIZE; // whole buff is empty

    //initialising all the pipe_info_per_process such that they are free (-1) (not used)
    int i = 0;
    while( i<MAX_PIPE_PROC){
        pipe->pipe_per_proc[i].used = -1;
        pipe->pipe_per_proc[i].read_state = -1;
        pipe->pipe_per_proc[i].write_state = -1;
        i++;
    }

    // first process which is connected to pipe
    struct exec_context* current = get_current_ctx();

    int flag = 0;
    i = 0;
    while(i< MAX_PIPE_PROC){
        if(pipe->pipe_per_proc[i].used == -1){
            flag = 1;
            pipe->pipe_per_proc[i].pid = current->pid ;
            pipe->pipe_per_proc->read_state = 1;
            pipe->pipe_per_proc->write_state = 1;
            pipe->pipe_per_proc[i].used = 1;
            break;
        }
        i++;
    }

    // Return the pipe.
    return pipe;

}

// Function to free pipe buffer and pipe info object.
// NOTE: DO NOT MODIFY THIS FUNCTION.
void free_pipe (struct file *filep) {

    os_page_free(OS_DS_REG, filep->pipe->pipe_global.pipe_buff);
    os_page_free(OS_DS_REG, filep->pipe);

}

// Fork handler for the pipe.
int do_pipe_fork (struct exec_context *child, struct file *filep) {

    /**
     *  TODO:: Implementation for fork handler
     *
     *  You may need to update some per process or global info for the pipe.
     *  This handler will be called twice since pipe has 2 file objects.
     *  Also consider the limit on no of processes a pipe can have.
     *  Return 0 on success.
     *  Incase of any error return -EOTHERS.
     *
     */

    if(filep == NULL) return -EOTHERS;

    int i = 0;
    int flag1 = 0;
    int index;
    while(i<MAX_PIPE_PROC){
        if(filep->pipe->pipe_per_proc[i].pid == child->pid && filep->pipe->pipe_per_proc[i].used==1){
            index = i;
            flag1 = 1;
            break;
        }
        i++;
    }

    i=0;
    int flag2=0;
    while(i<MAX_PIPE_PROC){
        if(flag1){
            if(filep->mode == O_READ)
                filep->pipe->pipe_per_proc[index].read_state = 1;
            if(filep->mode == O_WRITE)
                filep->pipe->pipe_per_proc[index].write_state = 1;
            filep->pipe->pipe_per_proc[index].used = 1;
            flag2 = 1;
            break;
        }
        else if(filep->pipe->pipe_per_proc[i].used ==-1){
            filep->pipe->pipe_per_proc[i].pid = child->pid;
            if(filep->mode == O_READ)
                filep->pipe->pipe_per_proc[i].read_state = 1;
            if(filep->mode == O_WRITE)
                filep->pipe->pipe_per_proc[i].write_state = 1;
            filep->pipe->pipe_per_proc[i].used = 1;
            flag2 = 1;
            break;
        }
        i++;
    }
    if(!flag2){
        return -EOTHERS;
    }

    // Return successfully.
    return 0;

}

// Function to close the pipe ends and free the pipe when necessary.
long pipe_close (struct file *filep) {

    /**
     *  TODO:: Implementation of Pipe Close
     *
     *  Close the read or write end of the pipe depending upon the file
     *      object's mode.
     *  You may need to update some per process or global info for the pipe.
     *  Use free_pipe() function to free pipe buffer and pipe object,
     *      whenever applicable.
     *  After successful close, it return 0.
     *  Incase of any error return -EOTHERS.
     *
     */
    if(filep == NULL) return -EOTHERS;

    struct exec_context* current = get_current_ctx();
    if(filep->type != PIPE){
        return -EOTHERS;
    }
    int index ;
    int flag = 0;
    int k = 0;
    while(k<MAX_PIPE_PROC){
        if(filep->pipe->pipe_per_proc[k].used==1 && current->pid == filep->pipe->pipe_per_proc[k].pid){
            index = k;
            flag = 1;
            break;
        }
        k++;
    }
    if(flag){
        if(filep->mode == O_READ){
            if(filep->pipe->pipe_per_proc[index].read_state == -1){
                return -EOTHERS;
            }
            filep->pipe->pipe_per_proc[index].read_state = -1;
        }
        if(filep->mode == O_WRITE){
            if(filep->pipe->pipe_per_proc[index].write_state == -1){
                return -EOTHERS;
            }
            filep->pipe->pipe_per_proc[index].write_state = -1;
        }
        if(filep->pipe->pipe_per_proc[index].read_state == -1 && filep->pipe->pipe_per_proc[index].write_state == -1){
            filep->pipe->pipe_per_proc[index].used = -1;
        }
        int flag1 = 0;
        int i = 0;
        while( i<MAX_PIPE_PROC){
            if(filep->pipe->pipe_per_proc[i].read_state == -1 && filep->pipe->pipe_per_proc[i].write_state == -1){
                filep->pipe->pipe_per_proc[i].used = -1;
            }
            if(filep->pipe->pipe_per_proc[i].used == 1 ){
                flag1 = 1;
                break;
            }
            i++;
        }
        if(!flag1){
            free_pipe(filep);
        }
    }
    else {
        return -EOTHERS;
    }

    int ret_value;

    // Close the file and return.
    ret_value = file_close (filep);         // DO NOT MODIFY THIS LINE.

    // And return.
    return ret_value; 

}

// Check whether passed buffer is valid memory location for read or write.
int is_valid_mem_range (unsigned long buff, u32 count, int access_bit) {

    /**
     *  TODO:: Implementation for buffer memory range checking
     *
     *  Check whether passed memory range is suitable for read or write.
     *  If access_bit == 1, then it is asking to check read permission.
     *  If access_bit == 2, then it is asking to check write permission.
     *  If range is valid then return 1.
     *  Incase range is not valid or have some permission issue return -EBADMEM.
     *
     */

    //current process
    struct exec_context *current = get_current_ctx();

    //check
    if(access_bit == 1){
        int i = 0;
        while(i<MAX_MM_SEGS-1){
            if(current->mms[i].start<=buff && current->mms[i].next_free > buff+count-1 && (current->mms[i].access_flags)%2 == 1){
                return 1;
            }
            i++;
        }
        if(current->mms[MAX_MM_SEGS-1].start<=buff && current->mms[MAX_MM_SEGS-1].end>=buff+count-1 && (current->mms[MAX_MM_SEGS-1].access_flags)%2 == 1){
            return 1;
        }
        while(current->vm_area != NULL){
            if(current->vm_area->vm_start<=buff && current->vm_area->vm_end>=buff+count-1 && (current->vm_area->access_flags)%2 ==1){
                return 1;
            }
            current->vm_area = current->vm_area->vm_next;
        }
    }

    else if(access_bit == 2){
        int i = 0;
        while(i<MAX_MM_SEGS-1){
            if(current->mms[i].start<=buff && current->mms[i].next_free > buff+count-1 && (current->mms[i].access_flags)%4 - (current->mms[i].access_flags)%2 == 2){
                return 1;
            }
            i++;
        }
        if(current->mms[MAX_MM_SEGS-1].start<=buff && current->mms[MAX_MM_SEGS-1].end>=buff+count-1 && (current->mms[MAX_MM_SEGS-1].access_flags)%4 - (current->mms[MAX_MM_SEGS-1].access_flags)%2 == 2){
            return 1;
        }
        while(current->vm_area != NULL){
            if(current->vm_area->vm_start<=buff && current->vm_area->vm_end>=buff+count-1 && ((current->vm_area->access_flags)%4 - (current->vm_area->access_flags)%2 )== 2){
                return 1;
            }
            current->vm_area = current->vm_area->vm_next;
        }
    }
    
    int ret_value = -EBADMEM;
    
    // Return the finding.
    return ret_value;

}

// Function to read given no of bytes from the pipe.
int pipe_read (struct file *filep, char *buff, u32 count) {

    /**
     *  TODO:: Implementation of Pipe Read
     *
     *  Read the data from pipe buffer and write to the provided buffer.
     *  If count is greater than the present data size in the pipe then just read
     *       that much data.
     *  Validate file object's access right.
     *  On successful read, return no of bytes read.
     *  Incase of Error return valid error code.
     *       -EACCES: In case access is not valid.
     *       -EINVAL: If read end is already closed.
     *       -EOTHERS: For any other errors.
     *
     */
    if(filep == NULL) return -EOTHERS;

    // if mode is not read and filep is not of type pipe
    if(filep->mode != O_READ){
        return -EOTHERS;
    }
    if(filep->type != PIPE){
        return -EOTHERS;
    }

    struct exec_context* current = get_current_ctx();
    int index;
    int flag1= 0;
    unsigned int i = 0;
    while( i<MAX_PIPE_PROC){
        if(filep->pipe->pipe_per_proc[i].used == 1 && current->pid == filep->pipe->pipe_per_proc[i].pid && filep->pipe->pipe_per_proc[i].read_state == 1){
            index = i;
            flag1= 1;
            break;
        }
        i++;
    }
    if(!flag1){
        return -EINVAL;
    }

    unsigned long start = (unsigned long)buff;
    if(is_valid_mem_range(start, count, 2) != 1){
        return -EACCES;
    }

    // reading the data
    unsigned int bytes_read = 0;
    i = 0;
    
    while( i<count ){
        filep->pipe->pipe_global.buffempty += 1;
        if(filep->pipe->pipe_global.buffempty > MAX_PIPE_SIZE){
            filep->pipe->pipe_global.buffempty = MAX_PIPE_SIZE;
            break;
        }

        if(filep->pipe->pipe_global.read_pointer >= MAX_PIPE_SIZE) {
            filep->pipe->pipe_global.read_pointer = 0;
        }  
        
        buff[i] = filep->pipe->pipe_global.pipe_buff[filep->pipe->pipe_global.read_pointer];
        filep->pipe->pipe_global.read_pointer = (filep->pipe->pipe_global.read_pointer+1);

        bytes_read += 1;
        i++;
    }
    
    // Return no of bytes read.
    return bytes_read;
}

// Function to write given no of bytes to the pipe.
int pipe_write (struct file *filep, char *buff, u32 count) {
 
    /**
     *  TODO:: Implementation of Pipe Write
     *
     *  Write the data from the provided buffer to the pipe buffer.
     *  If count is greater than available space in the pipe then just write data
     *       that fits in that space.
     *  Validate file object's access right.
     *  On successful write, return no of written bytes.
     *  Incase of Error return valid error code.
     *       -EACCES: In case access is not valid.
     *       -EINVAL: If write end is already closed.
     *       -EOTHERS: For any other errors.
     *
     */
    if(filep == NULL) return -EOTHERS;

    // if mode is not write and filep is not of type pipe
    if(filep->type != PIPE){
        return -EOTHERS;
    }
    if(filep->mode != O_WRITE){
        return -EOTHERS;
    }

    struct exec_context* current = get_current_ctx();
    int index ;
    int flag1 = 0;
    unsigned long i = 0;
    while( i<MAX_PIPE_PROC){
        if(filep->pipe->pipe_per_proc[i].used == 1 && current->pid == filep->pipe->pipe_per_proc[i].pid && filep->pipe->pipe_per_proc[i].write_state == 1){
            index = i;
            flag1 = 1;
            break;
        }
        i++;
    }
    if(!flag1){
        return -EINVAL;
    }

    
    
    unsigned long start = (unsigned long)buff;
    if(is_valid_mem_range(start, count, 1) != 1){
        return -EACCES;
    }
    
    //writing the data
    unsigned int bytes_written = 0;
    i = 0;

    while( i<count ){        
        filep->pipe->pipe_global.buffempty -= 1;
        if(filep->pipe->pipe_global.buffempty < 0){
            filep->pipe->pipe_global.buffempty = 0;
            break;
        }

        if(filep->pipe->pipe_global.write_pointer >= MAX_PIPE_SIZE) {
            filep->pipe->pipe_global.write_pointer = 0;
        }

        filep->pipe->pipe_global.pipe_buff[filep->pipe->pipe_global.write_pointer] = buff[i];
        filep->pipe->pipe_global.write_pointer = (filep->pipe->pipe_global.write_pointer+1);
    
        bytes_written += 1;
        i++;
    }

    // Return no of bytes written.
    return bytes_written;

}

// Function to create pipe.
int create_pipe (struct exec_context *current, int *fd) {

    /**
     *  TODO:: Implementation of Pipe Create
     *
     *  Find two free file descriptors.
     *  Create two file objects for both ends by invoking the alloc_file() function. 
     *  Create pipe_info object by invoking the alloc_pipe_info() function and
     *       fill per process and global info fields.
     *  Fill the fields for those file objects like type, fops, etc.
     *  Fill the valid file descriptor in *fd param.
     *  On success, return 0.
     *  Incase of Error return valid Error code.
     *       -ENOMEM: If memory is not enough.
     *       -EOTHERS: Some other errors.
     *
     */

    //2 file descriptors
    int count = 0;
    int fd1;
    int fd2;
    int i = 0;
    while(i<MAX_OPEN_FILES){
        if(current->files[i] == NULL && count<2){
            if(count==0) fd1 = i;
            else if(count==1) fd2 = i;
            count++;  
        }   
        if(count == 2) break;
        i++;
    }
    if(count<2){
        return -EOTHERS;
    }

    //making file object and pipe info object
    struct file* writeend = alloc_file();
    if(writeend == NULL) return -ENOMEM;
    struct file* readend = alloc_file();
    if(readend == NULL) return -ENOMEM;
    struct pipe_info* pipeinfo = alloc_pipe_info();
    if(pipeinfo == NULL) return -ENOMEM;

    //pipeinfo
    writeend->pipe = pipeinfo;
    readend->pipe = pipeinfo;

    //write file object
    writeend->type = PIPE;
    writeend->mode = O_WRITE;
    writeend->fops->write = pipe_write;
    writeend->fops->close = pipe_close;
    writeend->fops->read = pipe_read;

    //read file object
    readend->type = PIPE;
    readend->mode = O_READ;
    readend->fops->read = pipe_read;
    readend->fops->close = pipe_close;
    readend->fops->write = pipe_write;
    
    //indexes and assign file objects
    fd[0] = fd1;
    current->files[fd1] = readend;

    fd[1] = fd2;
    current->files[fd2] = writeend;

    // Simple return.
    return 0;

}
