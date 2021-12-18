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


    // Create child.	
	int pid = fork();
    if (pid < 0) {

        printf ("Parent: Somehow not able to create process!!!\n");
        exit (-1);

    }

	if (!pid) {                 // Child code.

        // Write to the ppipe.
        ret_code = write (fd[1], "piazza", 6);
        if (ret_code < 0) {
            
            printf ("Child: Writing to the ppipe is failed!!!\n");
            return -1;

        }
        // Expected return should be 6.
        printf ("%d\n", ret_code);

        // Exit.
        exit (1);

    }


    // This is for order among processes.
    sleep (5);


    // Create another child.
    pid = fork();

    if (pid < 0) {

        printf ("Parent: Somehow not able to create another process!!!\n");
        exit (-1);

    }

	if (!pid) {                 // Child code.

        // Read from the ppipe.
        char read_buffer [5];
		ret_code = read (fd[0], read_buffer, 4);
        read_buffer[4] = '\0';
		if (ret_code < 0) {
		
			printf ("Child: Reading from the ppipe is failed!!!\n");
			return -1;

		}

        // Expected return will be 4 and buffer will be "piaz".
        printf ("%d\n", ret_code);
		printf ("%s\n", read_buffer);

        // Exit.
        exit (1);

    }


    // This is for order among processes.
    sleep (5);

    
    // Read from the ppipe.
    char read_buffer [11];
    int i;
    for (i = 0; i < 10; i++)
        read_buffer[i] = 's';

    ret_code = read (fd[0], read_buffer, 10);
    read_buffer[10] = '\0';
    if (ret_code < 0) {
    
        printf ("Parent: Reading from the ppipe is failed!!!\n");
        return -1;

    }

    // Expected return will be 6 and buffer will be "piazzassss".
    printf ("%d\n", ret_code);
    printf ("%s\n", read_buffer);

    
    // Simple return.
	return 0;

}
