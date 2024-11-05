#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>

#define MAX_PROCESSES 100
#define MAX_ARGUMENTS 10

// Struct to represent each process's control block
// Stores essential details for each process
// Each process has their own pcb and are linked in a circular list via next.
typedef struct pcb
{
    struct pcb *next; // Points to the next process (circular linked list)
    char *command;    // The command that the process will excute
    pid_t pid;        // Process ID
    int proc_argc;    // Number of arguments
    char **proc_argv; // Array of arguments
    char status;      // Status: 'r' (ready or stopped), 'a' (active and running), 't' (terminated)
} proc;

// Global variables
proc *processes[MAX_PROCESSES]; // process list (array holding all the process control block) - Array to hold all processes
proc *active;                   // pointer to running process (points to the currently running process ) - Pointer to the currently active process
int process_count = 0;          // Number of processes (keeps track of how many process are created) - Number of processes
int quantum = 0;                // Quantum value in milliseconds - Time slice (in milliseconds)

// Signal handler for SIGALRM : stops the currently active process when the timer goes off (after quantum expries)
// Ensures each process gets its time slice
void sigalrm_handler(int signum)
{
    // Stop the current active process when the quantum expires
    kill(active->pid, SIGSTOP);
}

// Parses input arguments to create process control blocks (PCB)
// Takes the input from the user, parses the commands and their arguments, and stores them in the PCB structure
// @argc: Number of arguments
// @argv: Array of argument strings
proc *parse_arguments(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: ./schedule quantum [prog1 args : prog2 args : ...]\n");
        exit(1);
    }

    // Set the quantum value from command-line arguments (from the first argument)
    quantum = atoi(argv[1]);

    // Start parsing arguments after the quantum
    int arg_index = 2;
    int proc_index = 0;

    // Parse all processes and their arguments
    while (arg_index < argc && proc_index < MAX_PROCESSES)
    {
        proc *new_process = (proc *)malloc(sizeof(proc)); // Allocate new_process process control block (PCB)
        processes[proc_index] = new_process;

        // Create the command by add "./" prefix (assuming local executable)
        // Jemma's PATH variable is not working (specific approach )
        new_process->command = (char *)malloc(strlen(argv[arg_index]) + 6);
        strcpy(new_process->command, "/bi./");
        strcpy(new_process->command + 5, argv[arg_index]);

        // Count arguments for the process
        int args_end = arg_index;
        int arg_count = 0;
        while (args_end < argc && arg_count < MAX_ARGUMENTS && strcmp(argv[args_end], ":") != 0)
        {
            args_end++;
            arg_count++;
        }

        if (arg_count >= MAX_ARGUMENTS)
        {
            printf("Too many arguments.\n");
            exit(1);
        }

        new_process->proc_argc = arg_count;
        new_process->proc_argv = (char **)malloc(arg_count * sizeof(char *) + 1);

        // Copy arguments into the process control block (PCB)
        for (int i = 0; i < arg_count; i++)
        {
            new_process->proc_argv[i] = (char *)malloc(strlen(argv[arg_index + i]) + 1);
            strcpy(new_process->proc_argv[i], argv[arg_index + i]);
        }
        new_process->proc_argv[arg_count] = (char *)0; // Should this not be NULL, NULL terminate the argument array
        new_process->status = 'n';                     // 'n' indicates new process
        proc_index++;
        process_count++;
        arg_index = args_end + 1; // Skip the colon ":" - Move to next process (after colon)
    }

    // Verify that all arguments are processed
    if (arg_index < argc)
    {
        printf("Too many processes.\n");
        exit(1);
    }

    // Zero out the remaining array
    for (proc_index = process_count; proc_index < MAX_PROCESSES; proc_index++)
        processes[proc_index] = (proc *)0;

    // Create links between process blocks
    for (proc_index = 0; proc_index < process_count - 1; proc_index++)
    {
        processes[proc_index]->next = processes[proc_index + 1];
    }
    processes[proc_index]->next = processes[0];
    return processes[0];
}

/* void roundRobinSchedule (struct pcb *head){
    proc* current = head;
    do {

// Fork all processes, ensuring they are created and stopped initially
        pid_t pid = fork();
        if (pid == 0){
            // In chold process, pause it immediately
            raise(SIGSTOP);
            execvp(current->command, current->proc_argv);
            exit(0);
        } else {
            current->pid
        }
        }
    }

}
*/

// make all the process
proc *forkall(proc *head)
{
    proc *cur = head;
    do
    {
        pid_t new_process = fork();
        if (new_process == 0)
        {
            raise(SIGSTOP);
            execv(cur->command + 5, cur->proc_argv);
            execv(cur->command + 3, cur->proc_argv);
            cur->command[3] = 'n';
            execv(cur->command, cur->proc_argv);
            exit(1);
        }
        else
        {
            cur->pid = new_process;
            cur->status = 'r';
            cur = cur->next;
        }
    } while (cur != head);
    return head;
}

int freeall()
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (processes[i])
        {
            proc *cur = processes[i];
            free(cur->command);
            for (int j = 0; j <= cur->proc_argc; j++)
            {
                free(cur->proc_argv[j]);
            }
            free(cur->proc_argv);
            free(cur);
        }
    }
    return 0;
}
// round-robin scheduler created
int main(int argc, char *argv[])
{
    proc *head = parse_arguments(argc, argv);
    forkall(head);

    active = processes[0]; // choose first process to be active
    proc *prev = processes[process_count - 1];
    int proc_status = 0;
    struct itimerval timer;
    timer.it_value.tv_sec = quantum / 1000;
    timer.it_value.tv_usec = (quantum % 1000) * 1000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    while (active->next != active)
    {
        signal(SIGALRM, sigalrm_handler);
        kill(active->pid, SIGCONT);
        setitimer(ITIMER_REAL, &timer, NULL);
        active->status = 'a';
        waitpid(active->pid, &proc_status, WUNTRACED);
        if (WIFSTOPPED(proc_status))
        {
            active->status = 'r';
            prev = active;
        }
        else
        {
            active->status = 't';
            prev->next = active->next;
        }
        active = active->next;
    }
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);
    kill(active->pid, SIGCONT);
    waitpid(active->pid, &proc_status, WUNTRACED);
    freeall();
    return 0;
}
