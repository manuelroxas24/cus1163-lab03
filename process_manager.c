#include "process_manager.h"

/*
 * Function 1: Basic Producer-Consumer Demo
 * Creates one producer child (sends 1,2,3,4,5) and one consumer child (adds them up)
 */
int run_basic_demo(void) 
{
    int pipe_fd[2];
    pid_t producer_pid, consumer_pid;
    int status;
    
    printf("\nParent process (PID: %d) creating children...\n", getpid());
    
    
    if (pipe(pipe_fd) == -1)
    {
        perror("pipe");
        return -1;
    }

     // Producer Child Creation
    producer_pid = fork();
    if (producer_pid < 0) 
    {
        perror("fork producer");
        return -1;
    } 
    else if (producer_pid == 0)
    {
        close(pipe_fd[0]);
        producer_process(pipe_fd[1], 1);
    }
    else 
    {
        printf("Created producer child (PID: %d)\n", producer_pid);
    }

    // Consumer Child Creation
    consumer_pid = fork();
    if (consumer_pid < 0) 
    {
        perror("fork consumer");
        return -1;
    }
    else if (consumer_pid == 0)
    {
        close(pipe_fd[1]);
        consumer_process(pipe_fd[0], 0);
    }
    else
    {
        printf("Created consumer child (PID: %d)\n", consumer_pid);
    }

    // Exit Status
    close(pipe_fd[0]);
    close(pipe_fd[1]);

    if (waitpid(producer_pid, &status, 0) > 0)
    { 
        printf("Producer child (PID: %d) exited with status %d\n", producer_pid, WEXITSTATUS(status));
    }
    if (waitpid(consumer_pid, &status, 0) > 0)
    {
        printf("Consumer child (PID: %d) exited with status %d\n", consumer_pid, WEXITSTATUS(status));
    }
    return 0;
}



/*
 * Function 2: Multiple Producer-Consumer Pairs
 * Creates multiple pairs: pair 1 uses numbers 1-5, pair 2 uses 6-10, etc.
 */
int run_multiple_pairs(int num_pairs) 
{
    pid_t pids[10]; // Store all child PIDs
    int pid_count = 0;

    printf("\nParent creating %d producer-consumer pairs...\n", num_pairs);

    for (int i = 0; i < num_pairs; i++)
    {
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1)
        {
            perror("pipe");
            return -1;
        }
    
          // Producer Child Creation
        pid_t pid = fork();
        if (pid < 0) 
        {
            perror("fork producer");
            return -1;
        } 
        else if (pid == 0) 
        {
            close(pipe_fd[0]);
            producer_process(pipe_fd[1], i * 5 + 1);
        } 
        else 
        {
            printf("=== Pair %d ===\n", i + 1);
            pids[pid_count++] = pid;
        }

        // Consumer Child Creation
        pid = fork();
        if (pid < 0) 
        {
            perror("fork consumer");
            return -1;
        } 
        else if (pid == 0) 
        {
            close(pipe_fd[1]);
            consumer_process(pipe_fd[0], i + 1);
        } 
        else 
        {
            pids[pid_count++] = pid;
        }

        close(pipe_fd[0]);
        close(pipe_fd[1]);
    }

    // Exit Status
    for (int i = 0; i < pid_count; i++) 
    {
        int status;
        if (waitpid(pids[i], &status, 0) > 0)
        {
            printf("Child (PID: %d) exited with status %d\n", pids[i], WEXITSTATUS(status));
    
        }

    }
    printf("\nAll pairs completed successfully!\n");
    return 0;

}

/*
 * Producer Process - Sends 5 sequential numbers starting from start_num
 */
void producer_process(int write_fd, int start_num) {
    printf("Producer (PID: %d) starting...\n", getpid());
    
    // Send 5 numbers: start_num, start_num+1, start_num+2, start_num+3, start_num+4
    for (int i = 0; i < NUM_VALUES; i++) {
        int number = start_num + i;
        
        if (write(write_fd, &number, sizeof(number)) != sizeof(number)) {
            perror("write");
            exit(1);
        }
        
        printf("Producer: Sent number %d\n", number);
        usleep(100000); // Small delay to see output clearly
    }
    
    printf("Producer: Finished sending %d numbers\n", NUM_VALUES);
    close(write_fd);
    exit(0);
}

/*
 * Consumer Process - Receives numbers and calculates sum
 */
void consumer_process(int read_fd, int pair_id) {
    int number;
    int count = 0;
    int sum = 0;
    
    printf("Consumer (PID: %d) starting...\n", getpid());
    
    // Read numbers until pipe is closed
    while (read(read_fd, &number, sizeof(number)) > 0) {
        count++;
        sum += number;
        printf("Consumer: Received %d, running sum: %d\n", number, sum);
    }
    
    printf("Consumer: Final sum: %d\n", sum);
    close(read_fd);
    exit(0);
}