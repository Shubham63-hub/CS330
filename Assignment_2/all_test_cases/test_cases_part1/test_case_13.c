#include<ulib.h>


/**
 *  Functions tested in this test case:
 *  1. pipe creation
 *  2. pipe read
 *  3. pipe write
 *  4. fork handler
 *  5. pipe close
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


    // Write "hellocs330" to the pipe.
    ret_code = write (fd[1], "hellocs330", 10);
    if (ret_code < 0) {

        printf ("Parent: Pipe write is failed!!!\n");
        return -1;

    } else {

        // Expected result should be 10.
        printf ("%d\n", ret_code);

    }


    // Create child.	
	int pid = fork();
    if (pid < 0) {

        printf ("Parent: Somehow not able to create process!!!\n");
        exit (-1);

    }

	if (!pid) {                 // Child 1 code.

        // Read 4 characters from the pipe.
        char read_buffer[5];
        ret_code = read (fd[0], read_buffer, 4);
        read_buffer[4] = '\0';
        if (ret_code < 0) {

            printf ("Child 1: Pipe read is failed!!!\n");
            return -1;

        } else {

            // Expected result should be 4 and the read buffer should be "hell".
            printf ("%d\n", ret_code);
            printf ("%s\n", read_buffer);

        }


        // Create another child inside this child.
        int pid_2 = fork();
        if (pid_2 < 0) {

            printf ("Child 1: Somehow not able to create another process!!!\n");
            exit (-2);

        }

        if (!pid_2) {           // Child 2 code.
        
            // Read 4 characters from the pipe.
            char read_buffer[5];
            ret_code = read (fd[0], read_buffer, 4);
            read_buffer[4] = '\0';
            if (ret_code < 0) {

                printf ("Child 2: Pipe read is failed!!!\n");
                return -1;

            } else {

                // Expected result should be 4 and the read buffer should
                // be "ocs3".
                printf ("%d\n", ret_code);
                printf ("%s\n", read_buffer);

            }
           

            // Exit.
            exit (2);

        }


        // This is for the order.
        sleep (5);
		
        
        // Read 2 characters from the pipe.
        char read_buffer_2 [3];
		ret_code = read (fd[0], read_buffer_2, 2);
        read_buffer_2 [2] = '\0';
		if (ret_code < 0) {
		
			printf ("Child 1: Reading from the pipe is failed!!!\n");
			return -1;

		}

        // Expected return will be 2 and the read buffer should be "30"
        printf ("%d\n", ret_code);
		printf ("%s\n", read_buffer_2);

        
        // Exit.
		exit (1);

	}


    // This is for order among processes.
    sleep (20);


    // Try reading 100 characters from the pipe.
    char read_buffer [100];
    read_buffer[0] = 's';
    ret_code = read (fd[0], read_buffer, 100);
    if (ret_code < 0) {
    
        printf ("Parent: Reading from the pipe is failed!!!\n");
        return -1;

    }

    // Expected return will be 0 and first character in the buffer will
    // be 's'.
    printf ("%d\n", ret_code);
    printf ("%c\n", read_buffer[0]);


    // Write "help" to the pipe.
    ret_code = write (fd[1], "help", 4);
    if (ret_code < 0) {

        printf ("Parent: Pipe write is failed!!!\n");
        return -1;

    } else {

        // Expected result should be 4.
        printf ("%d\n", ret_code);

    }


    // Try reading 10 characters from the pipe.
    char read_buffer_2 [10];
    ret_code = read (fd[0], read_buffer_2, 10);
    read_buffer_2 [4] = '\0';
    if (ret_code < 0) {
    
        printf ("Parent: Reading from the pipe is failed!!!\n");
        return -1;

    }

    // Expected return will be 4 and read buffer should be "help".
    printf ("%d\n", ret_code);
    printf ("%s\n", read_buffer_2);


    // Close read end and try to read.
    // close should return 0 and read should return -1.
    printf ("%d\n", close (fd[0]));
    ret_code = read (fd[0], read_buffer_2, 2);
    printf ("%d\n", ret_code);


    // Close write end and try to write.
    // close should return 0 and write should return -1.
    printf ("%d\n", close (fd[1]));
    ret_code = write (fd[1], "shiv", 4);
    printf ("%d\n", ret_code);


    // Simple return.
	return 0;

}
