#include<ulib.h>


/**
 *  Functions tested in this test case:
 *  1. pipe creation
 *  2. pipe read
 *  3. pipe write
 *  4. fork handler
 */

int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {
	
    // Array to store both read and write fds of the pipe.
	int fd[2];
	
	// Create pipe.
	int ret_code = pipe(fd);
	
	// Check for any error in pipe creation.
	if (ret_code < 0) {

		printf("Parent: Pipe allocation failed!!!\n");
		return -1;
	
	}
    // Expected result is 0.
    printf ("%d\n", ret_code);


    // Expected value of fds in parent will be 3 and 4.
    printf ("%d\n", fd[0]);
    printf ("%d\n", fd[1]);


    // Create child.	
	int pid = fork();
    if (pid < 0) {

        printf ("Parent: Somehow not able to create process!!!\n");
        exit (-1);

    }

	if (!pid) {                 // Child 1 code.

        int pid_2 = fork();
        if (pid_2 < 0) {

            printf ("Child 1: Somehow not able to create another process!!!\n");
            exit (-2);

        }

        if (!pid_2) {           // Child 2 code.
        
            // Write to the pipe.
            ret_code = write (fd[1], "two", 3);
            if (ret_code < 0) {
                
                printf ("Child 2: Writing to the pipe is failed!!!\n");
                return -1;

            }
            // Expected return should be 3.
            printf ("%d\n", ret_code);

            // Exit.
            exit (1);

        }


        // This is for the order.
        sleep (5);
		
        
        // Read from the pipe.
        char read_buffer [4];
		ret_code = read (fd[0], read_buffer, 3);
        read_buffer[3] = '\0';
		if (ret_code < 0) {
		
			printf ("Child 1: Reading from the pipe is failed!!!\n");
			return -1;

		}

        // Expected return will be 3 and buffer will be "two".
        printf ("%d\n", ret_code);
		printf ("%s\n", read_buffer);

        
        // Write to the pipe.
        ret_code = write (fd[1], "one", 3);
        if (ret_code < 0) {
            
            printf ("Child 1: Writing to the pipe is failed!!!\n");
            return -1;

        }
        // Expected return should be 3.
        printf ("%d\n", ret_code);


        // Exit.
		exit (1);

	}


    // This is for order among processes.
    sleep (20);


    // Read from the pipe.
    char read_buffer [4];
    ret_code = read (fd[0], read_buffer, 3);
    read_buffer[3] = '\0';
    if (ret_code < 0) {
    
        printf ("Parent: Reading from the pipe is failed!!!\n");
        return -1;

    }

    // Expected return will be 3 and buffer will be "one".
    printf ("%d\n", ret_code);
    printf ("%s\n", read_buffer);


    // Simple return.
	return 0;

}
