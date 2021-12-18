#include<ulib.h>


/**
 *  Functions tested in this test case:
 *  1. ppipe creation
 *  2. ppipe close
 */

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {

    // Array to store read and write fds of the ppipe.
	int fd[2];

	// Create the ppipe.
	int ret_code = ppipe (fd);
	if (ret_code < 0) {

        printf ("Persistent Pipe op is failed!!!\n");
		return -1;

	}
    // Expected return will be 0.
    printf ("%d\n", ret_code);


    // Expected values for these fds will be 3 and 4.
	printf ("%d\n", fd[0]);
    printf ("%d\n", fd[1]);

    
    // Close the read end.
    // Expected return should be 0.
    printf ("%d\n", close (fd[0]));

    
    // Write to the ppipe. It should work.
    ret_code = write (fd[1], "shiv", 4);
    if (ret_code < 0) {

        printf ("Writing to the ppipe is failed!!!\n");
        return -1;

    }
    // Expected return will be 4.
    printf ("%d\n", ret_code);


    // Now try to read on the closed read end.
    // It should return -1.
    char read_buffer [4];
    ret_code = read (fd[0], read_buffer, 2);
    printf ("%d\n", ret_code);
    

	// Finally simple return.
	return 0;

}
