#include<ulib.h>


/**
 *  Functions tested in this test case:
 *  1. ppipe creation
 *  2. ppipe read
 *  3. ppipe write
 *  4. fork handler
 *  5. ppipe close
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


    // Read 4000 characters from the ppipe.
    char read_buffer[4001];
    ret_code = read (fd[0], read_buffer, 4000);
    read_buffer[4000] = 'h';
    if (ret_code < 0) {

        printf ("Parent: Persistent Pipe read is failed!!!\n");
        return -1;

    } else {

        // Expected result should be 4000 and the last character in the read
        // buffer should be 's' and then next should be 'h'.
        printf ("%d\n", ret_code);
        printf ("%c\n", read_buffer[3999]);
        printf ("%c\n", read_buffer[4000]);

    }


    // Create child.	
	int pid = fork();
    if (pid < 0) {

        printf ("Parent: Somehow not able to create process!!!\n");
        exit (-1);

    }

	if (!pid) {                 // Child 1 code.

        // Read 10 characters from the ppipe.
        char read_buffer[11];
        ret_code = read (fd[0], read_buffer, 10);
        read_buffer[10] = 'i';
        if (ret_code < 0) {

            printf ("Child 1: Persistent Pipe read is failed!!!\n");
            return -1;

        } else {

            // Expected result should be 10 and the last character in the read
            // buffer should be 's' and then next should be 'i'.
            printf ("%d\n", ret_code);
            printf ("%c\n", read_buffer[9]);
            printf ("%c\n", read_buffer[10]);

        }


        // Create another child inside this child.
        int pid_2 = fork();
        if (pid_2 < 0) {

            printf ("Child 1: Somehow not able to create another process!!!\n");
            exit (-2);

        }

        if (!pid_2) {           // Child 2 code.

            // This is for order.
            sleep (10);


            // Close the read end here.
            ret_code = close (fd[0]);
            if (ret_code < 0) {
            
                printf ("Child 2: closing on read end is failed!!!\n");
                return -1;

            } else {

                // Expected return should be 0.
                printf ("%d\n", ret_code);

            }


            // This is for the order
            sleep (20);


            // Exit.
            exit (2);

        }


        // Read 10 characters from the ppipe.
        ret_code = read (fd[0], read_buffer, 10);
        read_buffer[10] = 'v';
        if (ret_code < 0) {

            printf ("Child 1: Persistent Pipe read is failed!!!\n");
            return -1;

        } else {

            // Expected result should be 10 and the last character in the read
            // buffer should be 's' and then next should be 'v'.
            printf ("%d\n", ret_code);
            printf ("%c\n", read_buffer[9]);
            printf ("%c\n", read_buffer[10]);

        }


        // This is for order.
        sleep (10);
        sleep (10);
        sleep (20);


        // Exit.
		exit (1);

	}


    // This is for order among processes.
    sleep (20);


    // Read 40 characters from the ppipe.
    char read_buffer_2[41];
    read_buffer_2[40] = 'b';
    ret_code = read (fd[0], read_buffer_2, 40);
    if (ret_code < 0) {

        printf ("Parent: Persistent Pipe read is failed!!!\n");
        return -1;

    } else {

        // Expected result should be 40 and the last character in the read
        // buffer should be 's' and next should be 'b';
        printf ("%d\n", ret_code);
        printf ("%c\n", read_buffer_2[39]);
        printf ("%c\n", read_buffer_2[40]);

    }


    // Call flush which will return 4020.
    ret_code = flush_ppipe (fd);
    if (ret_code < 0) {
    
        printf ("Parent: Flush on Persistent Pipe is failed!!!\n");
        return -1;

    } else {
    
        // Expected return will be 4020.
        printf ("%d\n", ret_code);
    
    }


    // Make parent last to terminate.
    sleep (30);


    // Simple return.
	return 0;

}
