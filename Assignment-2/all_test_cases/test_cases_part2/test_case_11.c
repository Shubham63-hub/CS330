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


    // Read from the ppipe.
    char read_buffer[2001];
    ret_code = read (fd[0], read_buffer, 2000);
    read_buffer[2000] = 'h';
    if (ret_code < 0) {

        printf ("Parent: Persistent Pipe read is failed!!!\n");
        return -1;

    } else {

        // Expected result should be 2000 and last read character sould be 's'
        // and after that it should be 'h'.
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


        // Write 1000 characters to the ppipe.
        char write_buffer[1000];
        for (i = 0; i < 1000; i++)
            write_buffer[i] = 'i';

        ret_code = write (fd[1], write_buffer, 1000);
        if (ret_code < 0) {

            printf ("Child 1: Persistent Pipe write is failed!!!\n");
            return -1;

        } else {

            // Expected result should be 0.
            printf ("%d\n", ret_code);

        }


        // Create another child inside this child.
        int pid_2 = fork();
        if (pid_2 < 0) {

            printf ("Child 1: Somehow not able to create another process!!!\n");
            exit (-2);

        }

        if (!pid_2) {           // Child 2 code.
        
            // Try reading 4006 characters from the ppipe.
            char read_buffer[4006];
            ret_code = read (fd[0], read_buffer, 4006);
            read_buffer[2096] = 'v';
            if (ret_code < 0) {

                printf ("Child 2: Persistent Pipe read is failed!!!\n");
                return -1;

            } else {

                // Expected result should be 2096 and last read character should
                // be 's' and after that it should be 'v'.
                printf ("%d\n", ret_code);
                printf ("%c\n", read_buffer[2095]);
                printf ("%c\n", read_buffer[2096]);

            }
           

            // Exit.
            exit (1);

        }


        // This is for the order.
        sleep (5);
		
        
        // Try reading 100 characters from the ppipe.
        char read_buffer [100];
        read_buffer[0] = 't';
		ret_code = read (fd[0], read_buffer, 100);
		if (ret_code < 0) {
		
			printf ("Child 1: Reading from the ppipe is failed!!!\n");
			return -1;

		}

        // Expected return will be 100 and first character in the buffer will
        // be 's'.
        printf ("%d\n", ret_code);
		printf ("%c\n", read_buffer[0]);

        
        // Exit.
		exit (1);

	}


    // This is for order among processes.
    sleep (20);


    // Try reading 100 characters from the ppipe.
    char read_buffer_2 [100];
    read_buffer_2[0] = 'r';
    ret_code = read (fd[0], read_buffer_2, 100);
    if (ret_code < 0) {
    
        printf ("Parent: Reading from the ppipe is failed!!!\n");
        return -1;

    }

    // Expected return will be 100 and first character in the buffer will
    // be 's'.
    printf ("%d\n", ret_code);
    printf ("%c\n", read_buffer_2[0]);


    // Simple return.
	return 0;

}
