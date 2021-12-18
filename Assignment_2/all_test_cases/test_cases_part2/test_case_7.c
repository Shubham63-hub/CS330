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

    
    // Close the write end.
    // Expected return should be 0.
    printf ("%d\n", close (fd[1]));


    // Read from the ppipe. It should work.
    char read_buffer [5];
    int i;
    for (i = 0; i < 4; i++)
        read_buffer[i] = 's';

    ret_code = read (fd[0], read_buffer, 4);
    read_buffer[4] = '\0';
    if (ret_code < 0) {

        printf ("Reading from the ppipe is failed!!!\n");
        return -1;

    }
    // Expected return should be 0 and the read buffer should be empty, i.e.,
    // here the buffer should be 'ssss'.
    printf ("%d\n", ret_code);
    printf ("%s\n", read_buffer);


    // Now try to write on the closed write end.
    // It should return -1.
    ret_code = write (fd[1], "shiv", 4);
    printf ("%d\n", ret_code);
    

	// Finally simple return.
	return 0;

}
