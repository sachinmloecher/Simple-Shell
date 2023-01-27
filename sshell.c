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
struct fullCmd{
    char cmd0[10];  // first argument
    char restofcmd[17][50];         // entire command itself including the first argument
};
int checkingForCd = 1;
void print_prompt() {
    printf("sshell@ucd$ ");
    fflush(stdout);
};
char *removeNewline(char *s)
{
    int i = 0, j = 0;
    while (s[i])
    {
        if (s[i] != '\n')
          s[j++] = s[i];
        i++;
    }
    s[j] = '\0';
    return s;
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
void output_redirection(char *cmd) {
    //array of strings that stores the command and the file that the output is redirected to
    char *outred[2];
    int x = 0;
    if (append_outred(cmd)) {
        // printf("Append output redirection\n");
        outred[x] = strtok(cmd, ">>");
        while(outred[x] != NULL) {
            outred[++x] = strtok(NULL, ">>");
        }
        removeSpaces(outred[1]);
        strcpy(cmd,outred[0]);
        int fd = open(outred[1], O_WRONLY|O_APPEND|O_CREAT, 0644);
        // redirect output to file
        dup2(fd, STDOUT_FILENO);
        close(fd);
    } else if (trunc_outred(cmd)) {
        // printf("Trunc output redirection\n");
        // string 0 is command itself, string 1 is the output file
        outred[x] = strtok(cmd, ">");
        while(outred[x] != NULL) {
            outred[++x] = strtok(NULL, ">");
        }
        removeSpaces(outred[1]);
        // printf("%s\n",outred[1]);
        // put the command itself in cmdcpy
        strcpy(cmd,outred[0]);
        // file descriptor to open the file specified by user
        int fd = open(outred[1], O_WRONLY|O_CREAT|O_TRUNC, 0644);
        // redirect output to file
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
};
char** parse_args(char *cmd, int flag) {
    // to store original command
    char cmdcpy[CMDLINE_MAX];
    strcpy(cmdcpy,cmd);
    if(strstr(cmdcpy,">")!=NULL && !flag){
        output_redirection(cmdcpy);
        // printf("Entering output redirection!!\n");
    }
    // printf("%s\n",cmd);
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
    int n = i;
    char** args;
    //char * args[17]
    args = malloc(17*sizeof(char*));
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
        char** args = parse_args(cmd,0);
        execvp(args[0], args);
        free(args);
        // printf("Process killed\n");
        fprintf(stderr, "Error: command not found\n");
        return 1;
    } else if (pid > 0){
        // Parent process
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
        exit(1);
    }
    return 0;
};
int pipeline(char *processes[], int n){
    int retval= 0;
    int pipefd[6];
    // n is the number of commands
    // int pid;
    int i;
    for(i = 0; i<n-1;i++)
        pipe(pipefd+2*i);
    // pid = fork();
    // printf("Hello pipeline\n");
    // fflush(stdout);
    for(i = 0;i<n;i++){
        if (fork()==0){
            if(i == 0){
                dup2(pipefd[i+1], STDOUT_FILENO);
                // close(pipefd[i+1]);
            }
            else if(i == n-1){
                dup2(pipefd[2*(i-1)], STDIN_FILENO);
                // close(pipefd[2*(i-1)]);
            }
            else{
                dup2(pipefd[2*i+1], STDOUT_FILENO);
                // close(pipefd[2*i+1]);
                dup2(pipefd[2*(i-1)], STDIN_FILENO);
                // close(pipefd[2*(i-1)]);
            }
        for(int j = 0; j< 2*(n-1); j++){
            close(pipefd[j]);
        }
        // printf("%d %s\n",1,processes[1].restofcmd[1]);
            if(i == n-1)
            {
                execute(processes[i]);
            }
            else{
            char** args = parse_args(processes[i],0);
            retval = execvp(args[0], args);
            }
        }
    }
    for(i = 0; i<2*(n-1);i++)
        close(pipefd[i]);
    int status;
    for(i = 0; i<n; i++)
    {
        wait(&status);
    }
    return retval;
}
int countpipes(char* cmd){
    int count = 0;
    for(int i = 0;cmd[i];i++){
        if(cmd[i]=='|')
            count++;
    }
    return count;
}
int main(void) {
        char cmd[CMDLINE_MAX];
        while (1) {
                char *nl;
                int retval;
                char cmdagain[CMDLINE_MAX];
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
                } else if (strstr(cmd,"|")!=NULL){
                    strcpy(cmdagain,cmd);
                    //int sizesofcommands[4]; // 4 different commands, each has a different size, where size is the number of args
                    int count = countpipes(cmd);
                    int m = 0;
                    char* processes[4];
                    processes[m] = strtok(cmd, "|");
                    while(processes[m] != NULL) {
                        processes[++m] = strtok(NULL, "|");
                    }
                    // check if any missing command in pipe
                    if(m!=(count+1)||count>3)
                    {
                        printf("Error: missing command\n");
                        continue;
                    }
                    retval = pipeline(processes,m);
                    fprintf(stderr, "+ completed '%s'[%d] \n", removeNewline(cmdagain),retval);
                    // for(int r = 0; r<2;r++)
                    // {
                    //     printf("[%ls]",*(retpipe+r));
                    // }
                    // printf("\n");
                    continue;
                }
                else {
                    char **args = parse_args(cmd,1);
                    // printf("Checking cd!!!\n");
                    if (!strcmp(args[0], "cd")) {
                        int e = change_directory(args[1]);
                        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, e);
                    }
                    else {
                        // Regular command
                        retval = execute(cmd);
                        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
                        continue;
                    }
                }
        }
        return EXIT_SUCCESS;
    };
