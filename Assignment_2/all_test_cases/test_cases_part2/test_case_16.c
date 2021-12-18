#include<ulib.h>


/**
 *  Functions tested in this test case:
 *  1. ppipe creation
 *  2. ppipe read
 *  3. ppipe write
 *  4. fork handler
 */

int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {
	
    // Array to store both read and write fds of the ppipe.
	int fd[2];
	
	// Create ppipe.
	int ret_code = ppipe(fd);
	
	// Check for any error in ppipe creation.
	if (ret_code < 0) {

		printf("Parent: Persistent Pipe allocation failed!!!\n");
		return -1;
	
	}
    // Expected result is 0.
    printf ("%d\n", ret_code);


    // Expected value of fds in parent will be 3 and 4.
    printf ("%d\n", fd[0]);
    printf ("%d\n", fd[1]);


    // Write 4096 characters to the ppipe.
    char write_buffer[4096];
    int i;
    for (i = 0; i < 4096; i++)
        write_buffer[i] = 's';

    ret_code = write (fd[1], write_buffer, 4096);
    if (ret_code < 0) {

        printf ("Parent: Persistent Pipe write is failed!!!\n");
        return -1;

    } else {

        // Expected result should be 4096.
        printf ("%d\n", ret_code);

    }


    // Read 1 character from the ppipe.
    char read_buffer;
    ret_code = read (fd[0], &read_buffer, 1);
    if (ret_code < 0) {

        printf ("Parent: Persistent Pipe read is failed!!!\n");
        return -1;

    } else {

        // Expected result should be 1 and the read buffer will be 's'.
        printf ("%d\n", ret_code);
        printf ("%c\n", read_buffer);

    }


    // Create child.	
	int pid = fork();
    if (pid < 0) {

        printf ("Parent: Somehow not able to create process!!!\n");
        exit (-1);

    }

	if (!pid) {                 // Child 1 code.

        // Call flush which will return 1.
        ret_code = flush_ppipe (fd);
        if (ret_code < 0) {
        
            printf ("Child 1: Flush on Persistent Pipe is failed!!!\n");
            return -1;

        } else {
        
            // Expected return will be 1.
            printf ("%d\n", ret_code);
        
        }


        // Exit.
		exit (1);

	}


    // This is for order among processes.
    sleep (10);


    // Write 100 characters to the ppipe.
    char write_buffer_2[100];
    for (i = 0; i < 100; i++)
        write_buffer_2[i] = 'h';

    ret_code = write (fd[1], write_buffer_2, 100);
    if (ret_code < 0) {

        printf ("Parent: Persistent Pipe write is failed!!!\n");
        return -1;

    } else {

        // Expected result should be 1.
        printf ("%d\n", ret_code);

    }


    // Simple return.
	return 0;

}
