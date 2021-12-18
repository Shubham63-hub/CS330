#include<ulib.h>


/**
 *  Functions tested in this test case:
 *  1. pipe creation
 *  4. fork handler
 */

int main (u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {
	
    // Array to store both read and write fds of the pipe.
	int fd[2];

    /**
     *  Call pipe upto the (maximum possible fds)/ 2 which is MAX_OPEN_FILES/ 2
     *  in the parent. 0, 1, 2 are already present, so possible fds will be
     *  (MAX_OPEN_FILES/ 2 - 3) which is 8 - 3 = 5. That means we should call
     *  pipe 2 times in the parent.
     */

	// 1st time.
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


    // 2nd time.
	ret_code = pipe(fd);
	
	// Check for any error in pipe creation.
	if (ret_code < 0) {

		printf("Parent: Pipe allocation failed!!!\n");
		return -1;
	
	}
    // Expected result is 0.
    printf ("%d\n", ret_code);

    // Expected value of fds in parent will be 5 and 6.
    printf ("%d\n", fd[0]);
    printf ("%d\n", fd[1]);


    // Create child.	
	int pid = fork();
    if (pid < 0) {

        printf ("Parent: Somehow not able to create process!!!\n");
        exit (-1);

    }

	if (!pid) {                 // Child 1 code.

        /**
         *  Call pipe 2 times in this child.
         */

        // 1st time.
        ret_code = pipe(fd);
        
        // Check for any error in pipe creation.
        if (ret_code < 0) {

            printf("Child 1: Pipe allocation failed!!!\n");
            return -1;
        
        }
        // Expected result is 0.
        printf ("%d\n", ret_code);

        // Expected value of fds in parent will be 7 and 8.
        printf ("%d\n", fd[0]);
        printf ("%d\n", fd[1]);


        // 2nd time.
        ret_code = pipe(fd);
        
        // Check for any error in pipe creation.
        if (ret_code < 0) {

            printf("Child 1: Pipe allocation failed!!!\n");
            return -1;
        
        }
        // Expected result is 0.
        printf ("%d\n", ret_code);

        // Expected value of fds in parent will be 9 and 10.
        printf ("%d\n", fd[0]);
        printf ("%d\n", fd[1]);


        // Create another child inside this child.
        int pid_2 = fork();
        if (pid_2 < 0) {

            printf ("Child 1: Somehow not able to create another process!!!\n");
            exit (-2);

        }

        if (!pid_2) {           // Child 2 code.

            /**
             *  Call pipe 3 times in this child.
             */

            // 1st time.
            ret_code = pipe(fd);
            
            // Check for any error in pipe creation.
            if (ret_code < 0) {

                printf("Child 2: Pipe allocation failed!!!\n");
                return -1;
            
            }
            // Expected result is 0.
            printf ("%d\n", ret_code);

            // Expected value of fds in parent will be 11 and 12.
            printf ("%d\n", fd[0]);
            printf ("%d\n", fd[1]);


            // 2nd time.
            ret_code = pipe(fd);
            
            // Check for any error in pipe creation.
            if (ret_code < 0) {

                printf("Child 2: Pipe allocation failed!!!\n");
                return -1;
            
            }
            // Expected result is 0.
            printf ("%d\n", ret_code);

            // Expected value of fds in parent will be 13 and 14.
            printf ("%d\n", fd[0]);
            printf ("%d\n", fd[1]);


            // 3rd time.
            ret_code = pipe(fd);
            
            // Expected return is -EOTHERS, i.e., -7.
            printf ("%d\n", ret_code);


            // Exit.
            exit (2);

        }


        // This is for the order.
        sleep (10);
		
        
        // Exit.
		exit (1);

	}


    // This is for order among processes.
    sleep (30);


    // Simple return.
	return 0;

}
