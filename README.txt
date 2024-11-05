Jemma Arona, Luis Guzman

Special Instructions:
; No special instructions; simply run make and execute the program using ./signal.
; The program works as expected.
; The process structs include additional space before each command to manually test paths (for Jemma’s testing, so that Jemma can manually test PATH directories when running exec)

Overview:
This project implements a simple round-robin process scheduler in C. The scheduler reads a set of processes, forks them, and schedules them in a round-robin manner with a quantum time slice using the SIGALRM signal to switch between processes. It uses Process Control Blocks (PCBs) to manage each process, storing relevant details like the command, process ID, and arguments.

Features:

- Round-robin scheduling with time slicing
- Handles multiple processes with arguments
- Uses fork() and execv() to spawn child processes
- Memory management using dynamic allocation
- Circular linked list to manage process control blocks (PCBs)

Files:

1. schedule.c: Contains the main logic for parsing arguments, forking processes, and implementing the round-robin scheduler.
2. Makefile: Automates the build process.
