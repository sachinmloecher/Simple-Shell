# ECS 150: Project #1 - Simple Shell - Report

___Sachin Loecher and Shivam Bhandari___

The Aim of this project was building a shell and understanding how a shell 
works.

This Report aims to go over the phases of how we implemented this project.

### Summary
The 'sshell' is a program that executes user inputted jobs. Its features
include:
1. Built in commands ('exit', 'pwd', 'cd')
2. User commands with optional arguments ('ls', 'echo'...)
3. Piping commands (|)
4. Two forms of output redirection (>>, >)

### Phase 0 - Preliminary Work

We used the provided skeleton.c file to start this project. Provided code 
already implemented exit. It is already a simple mini shell.

We then built our Makefile. Our makefile uses the following command

https://github.com/sachinmloecher/ECS150P1/blob/fdeac5152b10548dee5eb4f08a47177c59228cc0/Makefile#L2

Also the clean rule removes any generated files.

https://github.com/sachinmloecher/ECS150P1/blob/fdeac5152b10548dee5eb4f08a47177c59228cc0/Makefile#L5

### Phase 1 - Switching system() to fork+exec+wait methods

We implemented our own manual system() function (execute()) using the fork+exec
+wait method explained in lecture. This function is only called to execute 
regular user supplied commands such as ls or echo. It first forks into parent 
and child processes, gets the inputted arguments using parsing, calls execvp()
to execute the command, and returns an error if execvp returns. The execvp 
kills the child process after execution.

### Phase 2 - Parsing arguments

In this phase, we built parse_args to parse the command into a data structure.
We use a struct fullCmd, which contains a string array, and an array of 
strings, which stores all the arguments separated into an array of strings, 
and a an int size, which contains the number of distinct instructions in our 
command. We parse the arguments using strtok, to separate them based on spaces. 
We then store it into an array of strings and return it.

### Phase 3 - Builtin Commands

We implemented the three built in commands ('exit', 'pwd', 'cd') in the 
main function. This is because commands like exit and pwd do not take 
arguments, and will be executed regardless. 'cd' required arguments, 
so we parse the arguments, change the directory using the syscall 'chdir()', 
and handle errors appropriately. All built in commands are checked for and
 executed directly from the main function.

### Phase 4: Output redirection

We implemented a simple function to take care of output redirection 
('output_redirection()'). This function checks the command for '>>' or '>', 
and splits it into the given output file and the command/arguments. We then 
open the given file, and use the syscall 'dup2()' to redirect the stdout of 
the command to the file descriptor.

### Phase 5: Pipeline Commands

Arguably the hardest part of our project. Essentially we check for '|' in the 
command and parse ot by splitting it into different commands called processes.
We store it into an array and then then pass it into our pipeline function, 
which takes care of the rest.

#### The pipeline function
 
A for loop in this function initializes the pipes according to the number of 
commands. We then loop through every command, calling fork to execute them 
in new processes. In each loop, we connect pipes to their respective inputs 
and outputs, and close the pipe file descriptor after. After the pipes are 
closed, it is a simple matter of just parsing the arguments and calling 
execvp. 
 
### Testing our code

We tested our code on csif manually running different commands and checking
for error management.
 
### Improvements
 
Unfortunately, we were unable to implement background jobs, but it is a 
feature we intend to implement in the future to complete this project. We also
look to make the code more robust, and break down the code into even smaller 
functions for better understanding.
 
*** ALL OUR CODE WAS WRITTEN ON OUR OWN, EXCEPT FOR THE SAMPLES OF CODE WE 
PICKED UP FROM THE LECTURE SLIDES OF THE PROFESSOR. ***
