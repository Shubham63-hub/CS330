#include<ulib.h>


/**
 *  Functions tested in this test case:
 *  1. pipe creation
 */

int main(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {

    // Array to store read and write fds of the pipe.
	int fd[2];

    /**
	 *  Call pipe upto the maximum possible fds which is MAX_OPEN_FILES.
     *  0, 1, 2 are already present, so possible fds will be MAX_OPEN_FILES - 3
     *  which is 16 - 3 = 13. That means we can call pipe successfully 6 times
     *  because at 7th time fd pair will not be available.
     */

    // 1st time.
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


    // 2nd time.
    ret_code = pipe (fd);
	if (ret_code < 0) {

        printf ("Pipe op is failed!!!\n");
		return -1;

	}
    // Expected return will be 0.
    printf ("%d\n", ret_code);

    // Expected values for these fds will be 5 and 6.
	printf ("%d\n", fd[0]);
    printf ("%d\n", fd[1]);


    // 3rd time.
    ret_code = pipe (fd);
	if (ret_code < 0) {

        printf ("Pipe op is failed!!!\n");
		return -1;

	}
    // Expected return will be 0.
    printf ("%d\n", ret_code);

    // Expected values for these fds will be 7 and 8.
	printf ("%d\n", fd[0]);
    printf ("%d\n", fd[1]);


    // 4th time.
    ret_code = pipe (fd);
	if (ret_code < 0) {

        printf ("Pipe op is failed!!!\n");
		return -1;

	}
    // Expected return will be 0.
    printf ("%d\n", ret_code);

    // Expected values for these fds will be 9 and 10.
	printf ("%d\n", fd[0]);
    printf ("%d\n", fd[1]);


    // 5th time.
    ret_code = pipe (fd);
	if (ret_code < 0) {

        printf ("Pipe op is failed!!!\n");
		return -1;

	}
    // Expected return will be 0.
    printf ("%d\n", ret_code);

    // Expected values for these fds will be 11 and 12.
	printf ("%d\n", fd[0]);
    printf ("%d\n", fd[1]);


    // 6th time.
    ret_code = pipe (fd);
	if (ret_code < 0) {

        printf ("Pipe op is failed!!!\n");
		return -1;

	}
    // Expected return will be 0.
    printf ("%d\n", ret_code);

    // Expected values for these fds will be 13 and 14.
	printf ("%d\n", fd[0]);
    printf ("%d\n", fd[1]);


    // 7th time.
    ret_code = pipe (fd);

    // Expected return will be -EOTHERS, i.e., -7.
    printf ("%d\n", ret_code);


	// Finally simple return.
	return 0;

}
