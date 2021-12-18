#include<ulib.h>


/**
 *  Functions tested in this test case:
 *  1. pipe creation
 *  2. pipe read
 *  3. pipe write
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
	

	// Write 4096 characters to the pipe.
	char write_buff[4096];
    int i;
    for (i = 0; i < 4096; i++)
        write_buff[i] = 's';

	ret_code = write (fd[1], write_buff, 4096);
	if (ret_code < 0) {
		
        printf ("Pipe write is failed!!!\n");
		return -1;

	} else {

        // Expected result should be 4096.
		printf ("%d\n", ret_code);

	}


	// Read from the pipe.
	char read_buff[2001];
	ret_code = read (fd[0], read_buff, 2000);
	read_buff[2000] = 'h';
	if (ret_code < 0) {

		printf ("Pipe read is failed!!!\n");
		return -1;

	} else {

        // Expected result should be 2000 and last read character sould be 's'
        // and after that it should be 'h'.
		printf ("%d\n", ret_code);
        printf ("%c\n", read_buff[1999]);
        printf ("%c\n", read_buff[2000]);

	}


	// Write 1000 characters to the pipe.
	char write_buff_2[1000];
    for (i = 0; i < 1000; i++)
        write_buff_2[i] = 'i';

	ret_code = write (fd[1], write_buff_2, 1000);
	if (ret_code < 0) {
		
        printf ("Pipe write is failed!!!\n");
		return -1;

	} else {

        // Expected result should be 1000.
		printf ("%d\n", ret_code);

	}


	// Try reading 3106 characters from the pipe.
	char read_buff_2[3106];
	ret_code = read (fd[0], read_buff_2, 3106);
	read_buff_2[3096] = 'v';
	if (ret_code < 0) {

		printf ("Pipe read is failed!!!\n");
		return -1;

	} else {

        // Expected result should be 3096 and last read character sould be 'i'
        // and after that it should be 'v'.
		printf ("%d\n", ret_code);
        printf ("%c\n", read_buff_2[3095]);
        printf ("%c\n", read_buff_2[3096]);

	}


	// Finally simple return.
	return 0;

}
