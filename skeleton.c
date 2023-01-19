#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512

int execute(char *cmd){
    pid_t pid;
	int status;
	pid = fork();
    if (pid == 0) {		
        //child process
        char *dest = "/bin/";
        char *source = cmd;
        char *file = strcat(dest, source);
        char *args[] = {file, "", NULL};
        execv(file, args);
        perror("execv");
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
