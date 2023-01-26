#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
// Macros
#define CMDLINE_MAX 512
#define MAX_ARGUMENTS 16
#define TOKEN_MAX 32

void print_prompt() {
    printf("sshell@ucd$ ");
    fflush(stdout);
};

void get_cmd(char *cmd) {
    fgets(cmd, CMDLINE_MAX, stdin);
};

void print_exit() {
    fprintf(stderr, "Bye...\n");
    fprintf(stderr, "+ completed 'exit' [0]\n");
}

void print_working_dir() {
    printf("%s\n", getcwd(NULL, 0));
    fprintf(stderr, "+ completed 'pwd' [0]\n");
}

int change_directory(char *dir) {
    int a = chdir(dir);
    if (a == -1) {
        fprintf(stderr, "Error: cannot cd into directory\n");
        return 1;
    } else {
        return 0;
    }
}

// Helper function to remove spaces from output redirection filename
char *removeSpaces(char *s)
{
    int i = 0, j = 0;
    while (s[i])
    {
        if (s[i] != ' ')
          s[j++] = s[i];
        i++;
    }
    s[j] = '\0';
    return s;
};

bool trunc_outred(char *cmd) {
    if (strstr(cmd,">") != NULL) {
        return true;
    } else {
        return false;
    }
};

bool append_outred(char *cmd) {
    if (strstr(cmd,">>") != NULL) {
        return true;
    } else {
        return false;
    }
};

char* output_redirection(char *cmd, char *cmdcpy) {
    //array of strings that stores the command and the file that the output is redirected to
    char *outred[2];
    int x = 0;
    if (append_outred(cmd)) {
        printf("Append output redirection\n");
        outred[x] = strtok(cmd, ">>");
        while(outred[x] != NULL) {
            outred[++x] = strtok(NULL, ">>");
        }
        removeSpaces(outred[1]);
        strcpy(cmdcpy,outred[0]);
        int fd = open(outred[1], O_WRONLY|O_APPEND|O_CREAT, 0644);
        // redirect output to file
        dup2(fd, STDOUT_FILENO);
        close(fd);
    } else if (trunc_outred(cmd)) {
        printf("Trunc output redirection\n");
        // string 0 is command itself, string 1 is the output file
        outred[x] = strtok(cmd, ">");
        while(outred[x] != NULL) {
            outred[++x] = strtok(NULL, ">");
        }
        removeSpaces(outred[1]);
        // put the command itself in cmdcpy
        strcpy(cmdcpy,outred[0]);
        // file descriptor to open the file specified by user
        int fd = open(outred[1], O_WRONLY|O_CREAT|O_TRUNC, 0644);
        // redirect output to file
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
};

struct fullCmd{
    char cmd0[10];  // first argument
    char restofcmd[17][50];         // entire command itself including the first argument
};

char** parse_args(char *cmd) {
    // to store original command
    char cmdcpy[CMDLINE_MAX];
    output_redirection(cmd, cmdcpy);
    //strcpy(cmdcpy,cmd);
    int x = 0;
    // Output redirection
    //strcpy(cmdcpy, output_redirection(cmd));
    struct fullCmd c1;
    // splits the command up
    char *array[16];
    int i = 0;
    array[i] = strtok(cmdcpy, " ");
    strcpy(c1.cmd0,array[i]);
    while(array[i] != NULL){
        array[++i] = strtok(NULL, " ");
    }
    for(int j = 0; j<i;j++){
        strcpy(c1.restofcmd[j],array[j]);
    }
    // printf("%s\n",c1.cmd0);
    // for(int j = 0; j<i;j++){
    //     printf("%d. %s\n",j+1,c1.restofcmd[j]);
    // }
    int n = i;
    char** args;
    //char * args[17] 
    args = malloc(17*sizeof(char*));
    // printf("%ld\n",sizeof(c.restofcmd));
    // int n = int(sizeof(c.restofcmd))
    for(i = 0; i<n;i++){
        args[i] = malloc(33*sizeof(char));
        args[i] = c1.restofcmd[i];
    }
    args[i] = malloc(33*sizeof(char));
    args[i] = NULL;
    // for(int j = 0; j<=i;j++){
    //             printf("%d. %s\n",j+1,args[j]);
    //         }
    return args;
};

int execute(char *cmd){
    pid_t pid;
    int status;
    pid = fork();
    if (pid == 0) {
        // Child process
        char** args = parse_args(cmd);
        // Check for cd
        if (!strcmp(args[0], "cd")) {
            int a = change_directory(args[1]);
            free(args);
            return a;
        }
        else {
            execvp(args[0], args);
            free(args);
            fprintf(stderr, "Error: command not found\n");
            return 1;
        }
    } else if (pid > 0){
        // Parent process
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
        exit(1);
    }
    return 0;
};

int main(void) {
        char cmd[CMDLINE_MAX];
        while (1) {
                char *nl;
                int retval;
                /* Print prompt */
                print_prompt();
                /* Get command line */
                get_cmd(cmd);
                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }
                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';
                /* Builtin command */
                // exit
                if (!strcmp(cmd, "exit")) {
                    print_exit();
                    break;
                } else if (!strcmp(cmd, "pwd")) {
                    // pwd
                    print_working_dir();
                    continue;
                } else {
                    /* Regular command or cd*/
                    retval = execute(cmd);
                    fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
                    continue;
                }
        }
        return EXIT_SUCCESS;
};
