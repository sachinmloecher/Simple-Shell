# ECS 150: Project #1 - Simple Shell - Report

___Sachin Loecher and Shivam Bhandari___

The Aim of this project was building a shell and understanding how a shell works.

This Report aims to go over the phases of how we built this project.

### Phase 0 - Preliminary Work

We used the provided skeleton.c file to start this project. Provided code already implemented exit. It is already a simple mini shell.

We then built our Makefile. Our makefile runs

https://github.com/sachinmloecher/ECS150P1/blob/fdeac5152b10548dee5eb4f08a47177c59228cc0/Makefile#L2

Also the clean rule removes any generated files.

https://github.com/sachinmloecher/ECS150P1/blob/fdeac5152b10548dee5eb4f08a47177c59228cc0/Makefile#L5

### Phase 1 - Switching system() to fork+exec+wait methods

### Phase 2 - Parsing arguments

In this phase, we built parse_args to parse the command into a data structure. We use a struct fullCmd, which contains a string array, and an array of strings, which stores all the arguments separated into an array of strings, and a an int size, which contains the number of distinct instructions in our command. We parse the arguments using strtok, to separate them based on spaces. We then store it into an array of strings and return it.

### Phase 2 - Builtin Commands

### Phase 4: Output redirection

We now implement output redirection . We first check if there is ">" present in the command. We then split our command into the file to be outputted into and the command itself. We do a bunch of error management with helper functions. After that, we use dup2 to redirect the stdout to the file descriptor we opened.

### Phase 5: Pipeline Commands

Arguably the hardest part of our project. 







