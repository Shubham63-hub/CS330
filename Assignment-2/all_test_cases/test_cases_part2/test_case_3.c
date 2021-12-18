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
	

	// Try to write 4097 characters to the ppipe.
	char write_buff[4097];
    int i;
    for (i = 0; i < 4097; i++)
        write_buff[i] = 's';

	ret_code = write (fd[1], write_buff, 4097);
	if (ret_code < 0) {
		
        printf ("Persistent Pipe write is failed!!!\n");
		return -1;

	} else {

        // Expected result should be 4096.
		printf ("%d\n", ret_code);

	}


	// Read from the ppipe.
	char read_buff[4097];
	ret_code = read (fd[0], read_buff, 4097);
	read_buff[4096] = 'h';
	if (ret_code < 0) {

		printf ("Persistent Pipe read is failed!!!\n");
		return -1;

	} else {

        // Expected result should be 4096 and last read character sould be 's'
        // and after that it should be 'h'.
		printf ("%d\n", ret_code);
        printf ("%c\n", read_buff[4095]);
        printf ("%c\n", read_buff[4096]);

	}


	// Finally simple return.
	return 0;

}
