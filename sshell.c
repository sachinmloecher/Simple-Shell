#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#define CMDLINE_MAX 512
//New function
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
}
struct fullCmd{
    char cmd0[10];  // first argument
    char restofcmd[17][50];         // entire command itself including the first argument
};
int execute(char* cmd){
    pid_t pid;
    int status;
    pid = fork();
    if (pid == 0) {
        //child process
        char cmdcpy[CMDLINE_MAX]; // to store original command
        char *outred[2]; //array of strings that stores the command and the file that the output is redirected to
        strcpy(cmdcpy,cmd);
        int x = 0;
        // output redirection
        // splitting the command
        if (strstr(cmd,">") != NULL) {
            outred[x] = strtok(cmd, ">");            // string 0 is command itself, string 1 is the output file
            while(outred[x] != NULL){
            outred[++x] = strtok(NULL, ">");
            }
            removeSpaces(outred[1]);
            strcpy(cmdcpy,outred[0]);                // put the command itself in cmdcpy
            int fd;                                     // file descriptor to open the file specified by user
            fd = open(outred[1], O_WRONLY | O_CREAT,0644);
            dup2(fd, STDOUT_FILENO) ;               // redirect output to file
            close(fd);
            }
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
        char *args[17];
        // printf("%ld\n",sizeof(c.restofcmd));
        // int n = int(sizeof(c.restofcmd))
        for(i = 0; i<n;i++){
            args[i] = c1.restofcmd[i];
        }
        args[i] = NULL;
        // for(int j = 0; j<=i;j++){
        //             printf("%d. %s\n",j+1,args[j]);
        //         }
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else if (pid > 0){
        // parent process
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
        exit(1);
    }
    return 0;
}
int main(void)
{
        char cmd[CMDLINE_MAX];
        while (1) {
                char *nl;
                int retval;
                /* Print prompt */
                printf("sshell$ ");
                fflush(stdout);
                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);
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
                if (!strcmp(cmd, "exit")) {
                        fprintf(stdout, "Bye...\n");
                        break;
                }
                /* Regular command */
                retval = execute(cmd);
                fprintf(stderr, "Return status value for '%s': %d\n",
                        cmd, retval);
        }
        return EXIT_SUCCESS;
} 
