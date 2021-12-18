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


    // Write "hellocs330" to the ppipe.
    ret_code = write (fd[1], "hellocs330", 10);
    if (ret_code < 0) {

        printf ("Parent: Persistent Pipe write is failed!!!\n");
        return -1;

    } else {

        // Expected result should be 10.
        printf ("%d\n", ret_code);

    }


    // Create child.	
	int pid = fork();
    if (pid < 0) {

        printf ("Parent: Somehow not able to create process!!!\n");
        exit (-1);

    }

	if (!pid) {                 // Child 1 code.

        // Read 4 characters from the ppipe.
        char read_buffer[5];
        ret_code = read (fd[0], read_buffer, 4);
        read_buffer[4] = '\0';
        if (ret_code < 0) {

            printf ("Child 1: Persistent Pipe read is failed!!!\n");
            return -1;

        } else {

            // Expected result should be 4 and the read buffer should be "hell".
            printf ("%d\n", ret_code);
            printf ("%s\n", read_buffer);

        }


        // Create another child inside this child.
        int pid_2 = fork();
        if (pid_2 < 0) {

            printf ("Child 1: Somehow not able to create another process!!!\n");
            exit (-2);

        }

        if (!pid_2) {           // Child 2 code.
        
            // Read 4 characters from the ppipe.
            char read_buffer[5];
            ret_code = read (fd[0], read_buffer, 4);
            read_buffer[4] = '\0';
            if (ret_code < 0) {

                printf ("Child 2: Persistent Pipe read is failed!!!\n");
                return -1;

            } else {

                // Expected result should be 4 and the read buffer should
                // be "ocs3".
                printf ("%d\n", ret_code);
                printf ("%s\n", read_buffer);

            }
           

            // Exit.
            exit (2);

        }


        // This is for the order.
        sleep (5);
		
        
        // Read 2 characters from the ppipe.
        char read_buffer_2 [3];
		ret_code = read (fd[0], read_buffer_2, 2);
        read_buffer_2 [2] = '\0';
		if (ret_code < 0) {
		
			printf ("Child 1: Reading from the ppipe is failed!!!\n");
			return -1;

		}

        // Expected return will be 2 and the read buffer should be "oc"
        printf ("%d\n", ret_code);
		printf ("%s\n", read_buffer_2);

        
        // Exit.
		exit (1);

	}


    // This is for order among processes.
    sleep (20);


    // Try reading 10 characters from the ppipe.
    char read_buffer [11];
    read_buffer[0] = 's';
    ret_code = read (fd[0], read_buffer, 10);
    read_buffer[10] = '\0';
    if (ret_code < 0) {
    
        printf ("Parent: Reading from the ppipe is failed!!!\n");
        return -1;

    }

    // Expected return will be 10 and the read buffer will be "hellocs330"
    printf ("%d\n", ret_code);
    printf ("%s\n", read_buffer);


    // Simple return.
	return 0;

}
