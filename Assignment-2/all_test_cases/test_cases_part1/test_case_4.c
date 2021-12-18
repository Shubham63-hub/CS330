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

	if (!pid) {               // Child code.
		
		char read_buffer[6];

        // To have order.
		sleep (5);

        // Read from the pipe.
		ret_code = read (fd[0], read_buffer, 5);
        read_buffer[5] = '\0';
		if (ret_code < 0) {
		
			printf ("Child: Reading from the pipe is failed!!!\n");
			return -1;

		}

        // Expected return will be 5 and buffer will be "hello".
        printf ("%d\n", ret_code);
		printf ("%s\n", read_buffer);

        // Exit.
		exit (1);

	}


    // Write to the pipe.
	ret_code = write (fd[1], "hello", 5);
	if (ret_code < 0) {
		
		printf ("Parent: Writing to the pipe is failed!!!\n");
		return -1;

	}
    // Expected return should be 5.
    printf ("%d\n", ret_code);
    

    // This is for order between parent and child.
	sleep (20);


    // Simple return.
	return 0;

}
