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


    // Read 2000 characters from the ppipe.
    char read_buffer[2001];
    ret_code = read (fd[0], read_buffer, 2000);
    read_buffer[2000] = 'h';
    if (ret_code < 0) {

        printf ("Parent: Persistent Pipe read is failed!!!\n");
        return -1;

    } else {

        // Expected result should be 2000 and the last character in the read
        // buffer should be 's' and then next should be 'h'.
        printf ("%d\n", ret_code);
        printf ("%c\n", read_buffer[1999]);
        printf ("%c\n", read_buffer[2000]);

    }


    // Create child.	
	int pid = fork();
    if (pid < 0) {

        printf ("Parent: Somehow not able to create process!!!\n");
        exit (-1);

    }

	if (!pid) {                 // Child 1 code.

        // Call flush which will return 2000.
        ret_code = flush_ppipe (fd);
        if (ret_code < 0) {
        
            printf ("Child 1: Flush on Persistent Pipe is failed!!!\n");
            return -1;

        } else {
        
            // Expected return will be 2000.
            printf ("%d\n", ret_code);
        
        }


        // Create another child inside this child.
        int pid_2 = fork();
        if (pid_2 < 0) {

            printf ("Child 1: Somehow not able to create another process!!!\n");
            exit (-2);

        }

        if (!pid_2) {           // Child 2 code.

            // Write 1000 characters to the ppipe.
            char write_buffer[1000];
            for (i = 0; i < 1000; i++)
                write_buffer[i] = 'v';

            ret_code = write (fd[1], write_buffer, 1000);
            if (ret_code < 0) {

                printf ("Child 2: Persistent Pipe write is failed!!!\n");
                return -1;

            } else {

                // Expected result should be 1000.
                printf ("%d\n", ret_code);

            }


            // Exit.
            exit (2);

        }


        // This is for the order.
        sleep (5);
		
        
        // Read 3146 characters from the ppipe.
        char read_buffer_2 [3146];
		ret_code = read (fd[0], read_buffer_2, 3146);
        read_buffer_2 [3096] = 'b';
		if (ret_code < 0) {
		
			printf ("Child 1: Reading from the ppipe is failed!!!\n");
			return -1;

		}

        // Expected return will be 3096 and the last character in the read
        // buffer should be 'v' and next should be 'b'.
        printf ("%d\n", ret_code);
		printf ("%c\n", read_buffer_2[3095]);
		printf ("%c\n", read_buffer_2[3096]);

        
        // Exit.
		exit (1);

	}


    // This is for order among processes.
    sleep (30);


    // Write 1050 characters to the ppipe.
    char write_buffer_2[1050];
    for (i = 0; i < 1050; i++)
        write_buffer_2[i] = 'v';

    ret_code = write (fd[1], write_buffer_2, 1050);
    if (ret_code < 0) {

        printf ("Parent: Persistent Pipe write is failed!!!\n");
        return -1;

    } else {

        // Expected result should be 1000.
        printf ("%d\n", ret_code);

    }


    // Simple return.
	return 0;

}
