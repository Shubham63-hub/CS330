#include<ulib.h>


/**
 *  Functions tested in this test case:
 *  1. pipe creation
 *  2. pipe write with memory checker in VMA segment.
 */

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {

    // Array to store read and write fds of the pipe.
	int fd[2];

	// Create the pipe.
	int ret_code = pipe (fd);
	if (ret_code < 0) {

        printf ("Pipe op is failed!!!\n");
		return -1;

	}
    // Expected return will be 0.
    printf ("%d\n", ret_code);


    // Expected values for these fds will be 3 and 4.
	printf ("%d\n", fd[0]);
    printf ("%d\n", fd[1]);


	// Write to the pipe.
	char *write_buff = mmap (NULL, 4096, PROT_READ|PROT_WRITE, MAP_POPULATE);
    int i;
    for (i = 0; i < 10; i++)
        write_buff[i] = 's';

	ret_code = write (fd[1], write_buff, 5);
	if (ret_code < 0) {

        printf ("Pipe write is failed with err code: %d!!!\n", ret_code);
		return -1;

	} else {

        // Expected result should be 5.
		printf ("%d\n", ret_code);

	}


	// Finally simple return.
	return 0;

}
