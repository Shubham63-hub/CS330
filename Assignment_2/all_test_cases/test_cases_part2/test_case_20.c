#include<ulib.h>


/**
 *  Functions tested in this test case:
 *  1. ppipe creation
 *  2. ppipe read
 *  3. ppipe write
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


	// Try to write to the ppipe with wrong end, e.g. fd[0].
	char write_buff[6] = "hello";
	ret_code = write (fd[0], write_buff, 5);
    
    // Expected result should be -EACCES, i.e., -4.
    printf ("%d\n", ret_code);


	// Read from the ppipe.
	char read_buff[6];
	ret_code = read (fd[1], read_buff, 5);
	read_buff[5] = '\0';
    
    // Expected result should be -EACCES, i.e., -4.
	printf ("%d\n", ret_code);


	// Finally simple return.
	return 0;

}
