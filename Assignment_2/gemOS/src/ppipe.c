#include<ppipe.h>
#include<context.h>
#include<memory.h>
#include<lib.h>
#include<entry.h> 
#include<file.h>


// Per process information for the ppipe.
struct ppipe_info_per_process {

    // TODO:: Add members as per your need...
    int used;
    int pid;
    int read_state;
    int write_state;
    int read_pointer;

};

// Global information for the ppipe.
struct ppipe_info_global {

    char *ppipe_buff;       // Persistent pipe buffer: DO NOT MODIFY THIS.

    // TODO:: Add members as per your need...
    int write_pointer;
    int left_pointer;

};

// Persistent pipe structure.
// NOTE: DO NOT MODIFY THIS STRUCTURE.
struct ppipe_info {

    struct ppipe_info_per_process ppipe_per_proc [MAX_PPIPE_PROC];
    struct ppipe_info_global ppipe_global;

};


// Function to allocate space for the ppipe and initialize its members.
struct ppipe_info* alloc_ppipe_info() {

    // Allocate space for ppipe structure and ppipe buffer.
    struct ppipe_info *ppipe = (struct ppipe_info*)os_page_alloc(OS_DS_REG);
    char* buffer = (char*) os_page_alloc(OS_DS_REG);

    // Assign ppipe buffer.
    ppipe->ppipe_global.ppipe_buff = buffer;

    /**
     *  TODO:: Initializing pipe fields
     *
     *  Initialize per process fields for this ppipe.
     *  Initialize global fields for this ppipe.
     *
     */ 
    ppipe->ppipe_global.write_pointer = 0;
    ppipe->ppipe_global.left_pointer = 0;
    
    int i=0;
    while(i<MAX_PPIPE_PROC){
        ppipe->ppipe_per_proc[i].used = -1;
        ppipe->ppipe_per_proc[i].read_pointer = 0;
        ppipe->ppipe_per_proc[i].read_state = -1;
        ppipe->ppipe_per_proc[i].write_state = -1;
        i++;
    }

    struct exec_context* current = get_current_ctx();

    i=0;
    while(i<MAX_PPIPE_PROC){
        if(ppipe->ppipe_per_proc[i].used == -1){
            ppipe->ppipe_per_proc[i].pid = current->pid;
            ppipe->ppipe_per_proc[i].read_state = 1;
            ppipe->ppipe_per_proc[i].write_state = 1;
            ppipe->ppipe_per_proc[i].used = 1;
            break;
        }
        i++;
    }

    // Return the ppipe.
    return ppipe;

}

// Function to free ppipe buffer and ppipe info object.
// NOTE: DO NOT MODIFY THIS FUNCTION.
void free_ppipe (struct file *filep) {

    os_page_free(OS_DS_REG, filep->ppipe->ppipe_global.ppipe_buff);
    os_page_free(OS_DS_REG, filep->ppipe);

} 

// Fork handler for ppipe.
int do_ppipe_fork (struct exec_context *child, struct file *filep) {
    
    /**
     *  TODO:: Implementation for fork handler
     *
     *  You may need to update some per process or global info for the ppipe.
     *  This handler will be called twice since ppipe has 2 file objects.
     *  Also consider the limit on no of processes a ppipe can have.
     *  Return 0 on success.
     *  Incase of any error return -EOTHERS.
     *
     */

    int i = 0;
    int flag1 = 0;
    int index;
    while(i<MAX_PPIPE_PROC){
        if(filep->ppipe->ppipe_per_proc[i].pid == child->pid && filep->ppipe->ppipe_per_proc[i].used==1){
            index = i;
            flag1 = 1;
            break;
        }
        i++;
    }

    i=0;
    int flag2=0;
    while(i<MAX_PPIPE_PROC){
        if(flag1){
            if(filep->mode == O_READ)
                filep->ppipe->ppipe_per_proc[index].read_state = 1;
                filep->ppipe->ppipe_per_proc[index].read_pointer = 0;
            if(filep->mode == O_WRITE)
                filep->ppipe->ppipe_per_proc[index].write_state = 1;
            filep->ppipe->ppipe_per_proc[index].used = 1;
            flag2 = 1;
            break;
        }
        else if(filep->ppipe->ppipe_per_proc[i].used ==-1){
            filep->ppipe->ppipe_per_proc[i].pid = child->pid;
            if(filep->mode == O_READ)
                filep->ppipe->ppipe_per_proc[i].read_state = 1;
                filep->ppipe->ppipe_per_proc[index].read_pointer = 0;
            if(filep->mode == O_WRITE)
                filep->ppipe->ppipe_per_proc[i].write_state = 1;
            filep->ppipe->ppipe_per_proc[i].used = 1;
            flag2 = 1;
            break;
        }
        i++;
    }
    if(!flag2){
        return -ENOMEM;
    }

    // Return successfully.
    return 0;

}


// Function to close the ppipe ends and free the ppipe when necessary.
long ppipe_close (struct file *filep) {

    /**
     *  TODO:: Implementation of Pipe Close
     *
     *  Close the read or write end of the ppipe depending upon the file
     *      object's mode.
     *  You may need to update some per process or global info for the ppipe.
     *  Use free_pipe() function to free ppipe buffer and ppipe object,
     *      whenever applicable.
     *  After successful close, it return 0.
     *  Incase of any error return -EOTHERS.
     *                                                                          
     */

    struct exec_context* current = get_current_ctx();
    if(filep->type!= PPIPE){
        return -EOTHERS;
    }
    int index;
    int flag = 0;
    int k = 0;
    while(k<MAX_PPIPE_PROC){
        if(filep->ppipe->ppipe_per_proc[k].used==1 && current->pid == filep->ppipe->ppipe_per_proc[k].pid){
            index = k;
            flag = 1;
            break;
        }
        k++;
    }
    if(flag){
        if(filep->mode == O_READ){
            filep->ppipe->ppipe_per_proc[index].read_state = -1;
        }
        if(filep->mode == O_WRITE){
            filep->ppipe->ppipe_per_proc[index].write_state = -1;
        }
        if(filep->ppipe->ppipe_per_proc[index].read_state == -1 && filep->ppipe->ppipe_per_proc[index].write_state == -1){
            filep->ppipe->ppipe_per_proc[index].used = -1;
        }
        int flag1 = 0;
        int i = 0;
        while( i<MAX_PPIPE_PROC){
            if(filep->ppipe->ppipe_per_proc[i].used == 1 ){
                flag1 = 1;
                break;
            }
            i++;
        }
        if(!flag1){
            free_ppipe(filep);
        }
    }

    int ret_value;

    // Close the file and return.
    ret_value = file_close (filep);         // DO NOT MODIFY THIS LINE.

    // And return.
    return ret_value; 

}

// Function to perform flush operation on ppipe.
int do_flush_ppipe (struct file *filep) {

    /**
     *  TODO:: Implementation of Flush system call
     *
     *  Reclaim the region of the persistent pipe which has been read by 
     *      all the processes.
     *  Return no of reclaimed bytes.
     *  In case of any error return -EOTHERS.
     *
     */
    int reclaimed_bytes = 0;
    int minpointer = 4097, maxpointer = 0;
    int i = 0;
    int flag = 1;
    while(i<MAX_PPIPE_PROC){
        if(filep->ppipe->ppipe_per_proc[i].used == 1 && filep->ppipe->ppipe_per_proc[i].read_state == 1){
            if(minpointer < filep->ppipe->ppipe_per_proc[i].read_pointer){}
            else minpointer = filep->ppipe->ppipe_per_proc[i].read_pointer;

            if(maxpointer > filep->ppipe->ppipe_per_proc[i].read_pointer){}
            else maxpointer = filep->ppipe->ppipe_per_proc[i].read_pointer;
            flag = 0;
        }
        i++;
    }
    if(flag){
        return 0;
    }
    
    if(filep->ppipe->ppipe_global.left_pointer<maxpointer && filep->ppipe->ppipe_global.left_pointer>=minpointer){
        int i = filep->ppipe->ppipe_global.left_pointer;
        int firstread;
        while(i<MAX_PPIPE_PROC){
            if(filep->ppipe->ppipe_per_proc[i].used == 1 && filep->ppipe->ppipe_per_proc[i].read_state == 1){
                firstread = i;
                break;
            }
            i++;
        }
        reclaimed_bytes = filep->ppipe->ppipe_per_proc[firstread].read_pointer - filep->ppipe->ppipe_global.left_pointer;
        filep->ppipe->ppipe_global.left_pointer = filep->ppipe->ppipe_per_proc[firstread].read_pointer;
    }
    else if(filep->ppipe->ppipe_global.left_pointer>=maxpointer && filep->ppipe->ppipe_global.left_pointer>minpointer){
        reclaimed_bytes = 4096 - filep->ppipe->ppipe_global.left_pointer + minpointer;
        filep->ppipe->ppipe_global.left_pointer = minpointer;
    } 
    else if(filep->ppipe->ppipe_global.left_pointer<maxpointer && filep->ppipe->ppipe_global.left_pointer<minpointer ){
        reclaimed_bytes = minpointer - filep->ppipe->ppipe_global.left_pointer;
        filep->ppipe->ppipe_global.left_pointer = minpointer;
    }
    else if(filep->ppipe->ppipe_global.left_pointer==maxpointer && filep->ppipe->ppipe_global.left_pointer==minpointer){
        reclaimed_bytes = 0;
        filep->ppipe->ppipe_global.left_pointer = minpointer;
    } 

    // Return reclaimed bytes.
    return reclaimed_bytes;

}

// Read handler for the ppipe.
int ppipe_read (struct file *filep, char *buff, u32 count) {
    
    /**
     *  TODO:: Implementation of PPipe Read
     *
     *  Read the data from ppipe buffer and write to the provided buffer.
     *  If count is greater than the present data size in the ppipe then just read
     *      that much data.
     *  Validate file object's access right.
     *  On successful read, return no of bytes read.
     *  Incase of Error return valid error code.
     *      -EACCES: In case access is not valid.
     *      -EINVAL: If read end is already closed.
     *      -EOTHERS: For any other errors.
     *
     */

    if(filep->type != PPIPE){
        return -EOTHERS;
    }
    if(filep->mode != O_READ){
        return -EACCES;
    }

    struct exec_context* current = get_current_ctx();
    int index;
    int flag1 = 0;
    int i = 0;
    while( i<MAX_PPIPE_PROC){
        if(filep->ppipe->ppipe_per_proc[i].used == 1 && current->pid == filep->ppipe->ppipe_per_proc[i].pid && filep->ppipe->ppipe_per_proc[i].read_state == 1){
            index = i;
            flag1 = 1;
            break;
        }
        i++;
    }
    if(!flag1){
        return -EINVAL;
    }

    int bytes_read = 0;
    i = 0;
    while( i<count ){
        if(filep->ppipe->ppipe_per_proc[index].read_pointer == filep->ppipe->ppipe_global.write_pointer){
            break;
        }

        if(filep->ppipe->ppipe_per_proc[index].read_pointer + 1 >= MAX_PPIPE_SIZE){
            filep->ppipe->ppipe_per_proc[index].read_pointer = 0;
            if(filep->ppipe->ppipe_per_proc[index].read_pointer == filep->ppipe->ppipe_global.write_pointer){
                break;
            }
        }
            
        

        buff[i] = filep->ppipe->ppipe_global.ppipe_buff[filep->ppipe->ppipe_per_proc[index].read_pointer];
        filep->ppipe->ppipe_per_proc[index].read_pointer += 1;
        
        bytes_read += 1;
        i++;
    }


    // Return no of bytes read.
    return bytes_read;
	
}

// Write handler for ppipe.
int ppipe_write (struct file *filep, char *buff, u32 count) {

    /**
     *  TODO:: Implementation of PPipe Write
     *
     *  Write the data from the provided buffer to the ppipe buffer.
     *  If count is greater than available space in the ppipe then just write
     *      data that fits in that space.
     *  Validate file object's access right.
     *  On successful write, return no of written bytes.
     *  Incase of Error return valid error code.
     *      -EACCES: In case access is not valid.
     *      -EINVAL: If write end is already closed.
     *      -EOTHERS: For any other errors.
     *
     */

    if(filep->mode != O_WRITE){
        return -EACCES;
        // return -1;
    }

    struct exec_context* current = get_current_ctx();
    int index ;
    int flag1 = 0;
    int i = 0;
    while( i<MAX_PPIPE_PROC){
        if(filep->ppipe->ppipe_per_proc[i].used == 1 && current->pid == filep->ppipe->ppipe_per_proc[i].pid && filep->ppipe->ppipe_per_proc[i].write_state == 1){
            index = i;
            flag1 = 1;
            break;
        }
        i++;
    }
    if(!flag1){
        return -EINVAL;
        // return -2;
    }

    // if mode is not write and filep is not of type pipe
    if(filep->type != PPIPE){
        return -EOTHERS;
        // return -3;
    }
    
    //writing the data
    int bytes_written = 0;
    i = 0;
    while( i<count ){
        if((filep->ppipe->ppipe_global.write_pointer+1)%4097 == filep->ppipe->ppipe_global.left_pointer){
            break;
        }   

        if(filep->ppipe->ppipe_global.write_pointer >= MAX_PPIPE_SIZE) {
            filep->ppipe->ppipe_global.write_pointer = 0;
        }

        filep->ppipe->ppipe_global.ppipe_buff[filep->ppipe->ppipe_global.write_pointer] = buff[i];
        filep->ppipe->ppipe_global.write_pointer += 1;
    
        bytes_written += 1;
        i++;
    }

    // Return no of bytes written.
    return bytes_written;

}

// Function to create persistent pipe.
int create_persistent_pipe (struct exec_context *current, int *fd) {

    /**
     *  TODO:: Implementation of PPipe Create
     *
     *  Find two free file descriptors.
     *  Create two file objects for both ends by invoking the alloc_file() function.
     *  Create ppipe_info object by invoking the alloc_ppipe_info() function and
     *      fill per process and global info fields.
     *  Fill the fields for those file objects like type, fops, etc.
     *  Fill the valid file descriptor in *fd param.
     *  On success, return 0.
     *  Incase of Error return valid Error code.
     *      -ENOMEM: If memory is not enough.
     *      -EOTHERS: Some other errors.
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
        return -ENOMEM;
    }

    //making file object and pipe info object
    struct file* writeend = alloc_file();
    struct file* readend = alloc_file();
    struct ppipe_info* ppipeinfo = alloc_ppipe_info();

    //pipeinfo
    writeend->ppipe = ppipeinfo;
    readend->ppipe = ppipeinfo;

    //write file object
    writeend->type = PPIPE;
    writeend->mode = O_WRITE;
    writeend->fops->write = ppipe_write;
    writeend->fops->close = ppipe_close;
    writeend->fops->read = ppipe_read;

    //read file object
    readend->type = PPIPE;
    readend->mode = O_READ;
    readend->fops->read = ppipe_read;
    readend->fops->close = ppipe_close;
    readend->fops->write = ppipe_write;
    
    //indexes and assign file objects
    fd[0] = fd1;
    current->files[fd1] = readend;

    fd[1] = fd2;
    current->files[fd2] = writeend;

    // Simple return.
    return 0;

}
