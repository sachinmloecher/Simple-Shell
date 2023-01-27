#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#define CMDLINE_MAX 512
#define MAX_ARGUMENTS 16
#define TOKEN_MAX 32
// initializing all functions
void print_prompt();
char *removeNewline(char *s);
void get_cmd(char *cmd);
void print_exit();
void print_working_dir();
int change_directory(char *dir);
char *removeSpaces(char *s);
bool trunc_outred(char *cmd);
bool append_outred(char *cmd);
bool file_is_given(char *cmd);
void output_redirection(char *cmd);
char **parse_args(char *cmd, int flag);
int execute(char *cmd);
void pipeline(char *processes[], int n);
int countpipes(char* cmd);
int checkingForCd = 1;
int retpipe[4];
struct fullCmd
{
    // Contains actual command
    char cmd0[10];
    // Contains entire command with arguments
    char restofcmd[17][50];
};
void print_prompt()
{
    printf("sshell@ucd$ ");
    fflush(stdout);
};
char *removeNewline(char *s)
{
    int i = 0, j = 0;
    while (s[i])
    {
        if (s[i] != '\n')
        {
            s[j++] = s[i];
        }
        i++;
    }
    s[j] = '\0';
    return s;
};
void get_cmd(char *cmd)
{
    fgets(cmd, CMDLINE_MAX, stdin);
};
void print_exit()
{
    fprintf(stderr, "Bye...\n");
    fprintf(stderr, "+ completed 'exit' [0]\n");
};
void print_working_dir()
{
    printf("%s\n", getcwd(NULL, 0));
    fprintf(stderr, "+ completed 'pwd' [0]\n");
};
int change_directory(char *dir)
{
    int a = chdir(dir);
    if (a == -1)
    {
        fprintf(stderr, "Error: cannot cd into directory\n");
        return 1;
    } else
    {
        return 0;
    }
};
// Remove spaces from output redirection filename
char *removeSpaces(char *s)
{
    int i = 0, j = 0;
    while (s[i])
    {
        if (s[i] != ' ')
        {
            s[j++] = s[i];
        }
        i++;
    }
    s[j] = '\0';
    return s;
};
int countpipes(char* cmd)
{
    int count = 0;
    for(int i = 0;cmd[i];i++)
    {
        if(cmd[i]=='|')
            count++;
    }
    return count;
};
// Check for truncate output redirection
bool trunc_outred(char *cmd)
{
    if (strstr(cmd,">") != NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
};
// Check for append output redirection
bool append_outred(char *cmd)
{
    if (strstr(cmd,">>") != NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
};
bool file_is_given(char *cmd)
{
    char *outputred = strrchr(cmd, '>');
    if (outputred)
    {
        if (!strcmp(outputred, ">\0") || !strcmp(outputred, "> \0"))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return true;
    }
};
void output_redirection(char *cmd)
{
    //Contains the command and the file that the output is redirected to
    char *outred[2];
    int x = 0;
    if (append_outred(cmd))
    {
        // Get given file name
        outred[x] = strtok(cmd, ">>");
        while(outred[x] != NULL)
        {
            outred[++x] = strtok(NULL, ">>");
        }
        removeSpaces(outred[1]);
        strcpy(cmd,outred[0]);
        int fd = open(outred[1], O_WRONLY|O_APPEND|O_CREAT, 0644);
        // Redirect output to file
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    else if (trunc_outred(cmd))
    {
        // Get given file name
        outred[x] = strtok(cmd, ">");
        while(outred[x] != NULL)
        {
            outred[++x] = strtok(NULL, ">");
        }
        removeSpaces(outred[1]);
        strcpy(cmd,outred[0]);
        int fd = open(outred[1], O_WRONLY|O_CREAT|O_TRUNC, 0644);
        // Redirect output to file
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
};
char** parse_args(char *cmd, int flag)
{
    // To store original command
    char cmdcpy[CMDLINE_MAX];
    strcpy(cmdcpy,cmd);
    if(strstr(cmdcpy,">")!=NULL && !flag)
    {
        output_redirection(cmdcpy);
    }
    struct fullCmd c1;
    // Splits the command up by " "
    char *array[16];
    int i = 0;
    array[i] = strtok(cmdcpy, " ");
    strcpy(c1.cmd0,array[i]);
    while(array[i] != NULL)
    {
        array[++i] = strtok(NULL, " ");
    }
    for(int j = 0; j<i;j++)
    {
        strcpy(c1.restofcmd[j],array[j]);
    }
    int n = i;
    char** args;
    args = malloc(17*sizeof(char*));
    for(i = 0; i<n;i++)
    {
        args[i] = malloc(33*sizeof(char));
        args[i] = c1.restofcmd[i];
    }
    args[i] = malloc(33*sizeof(char));
    args[i] = NULL;
    return args;
};
int execute(char *cmd)
{
    pid_t pid;
    int status;
    pid = fork();
    if (pid == 0)
    {
        // Child process
        char** args = parse_args(cmd,0);
        execvp(args[0], args);
        free(args);
        fprintf(stderr, "Error: command not found\n");
        return 1;
    }
    else if (pid > 0)
    {
        // Parent process
        waitpid(pid, &status, 0);
    }
    else
    {
        perror("fork");
        exit(1);
    }
    return 0;
};
void pipeline(char *processes[], int n_commands)
{
    int pipefd[6];
    int i;
    // for loop to initialize pipes
    for(i = 0; i<n_commands-1;i++)
    {
        pipe(pipefd+2*i);
    }
    fflush(stdout);
    // connecting pipes to respective commands and input/output
    for(i = 0;i<n_commands;i++)
    {
        if (fork()==0)
        {
            if(i == 0)
            {
                dup2(pipefd[i+1], STDOUT_FILENO);  // first command in pipe
            }
            else if(i == n_commands-1)
            {
                dup2(pipefd[2*(i-1)], STDIN_FILENO);   // last command in pipe
            }
            else
            {
                dup2(pipefd[2*i+1], STDOUT_FILENO);
                dup2(pipefd[2*(i-1)], STDIN_FILENO);
            }
            for(int j = 0; j< 2*(n_commands-1); j++)
            {
                close(pipefd[j]);
            }
            char** args = parse_args(processes[i],0);
            retpipe[i] = execvp(args[0], args);
            free(args);
            retpipe[i] = 1;
        }
    }
    // closing pipes once finished
    for(i = 0; i<2*(n_commands-1);i++)
    {
        close(pipefd[i]);
    }
    int status;
    for(i = 0; i<n_commands; i++)
    {
        wait(&status);
    }
};
int main(void)
{
        char cmd[CMDLINE_MAX];
        while (1)
        {
            char *nl;
            int retval;
            char cmdagain[CMDLINE_MAX];
            /* Print prompt */
            print_prompt();
            /* Get command line */
            get_cmd(cmd);
            /* Print command line if stdin is not provided by terminal */
            if (!isatty(STDIN_FILENO))
            {
                printf("%s", cmd);
                fflush(stdout);
            }
            /* Remove trailing newline from command line */
            nl = strchr(cmd, '\n');
            if (nl)
            {
                *nl = '\0';
            }
            /* Builtin command */
            // exit
            if (!strcmp(cmd, "exit"))
            {
                print_exit();
                break;
            }
            // pwd
            else if (!strcmp(cmd, "pwd"))
            {
                print_working_dir();
                continue;
            }
            /* Piping */
            else if (strstr(cmd,"|") != NULL)
            {
                strcpy(cmdagain,cmd);
                // Counting number of pipes
                int count = countpipes(cmd);
                int m = 0;
                // Separating commands into different processes using strtok
                char* processes[4];
                processes[m] = strtok(cmd, "|");
                while(processes[m] != NULL)
                {
                    processes[++m] = strtok(NULL, "|");
                }
                // Check if any missing command in pipe
                if(m!=(count+1))
                {
                    printf("Error: missing command\n");
                    continue;
                }
                // Calling pipeline function
                pipeline(processes,m);
                fprintf(stderr, "+ completed '%s' ", removeNewline(cmdagain));
                for(int r = 0; r<m;r++)
                {
                    printf("[%d]",retpipe[r]);
                }
                printf("\n");
                continue;
                }
                else
                {
                    // Check that file is given
                    if (!file_is_given(cmd))
                    {
                        fprintf(stderr, "Error: no output file\n");
                        continue;
                    }
                    char **args = parse_args(cmd,1);
                    // cd
                    if (!strcmp(args[0], "cd"))
                    {
                        int e = change_directory(args[1]);
                        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, e);
                    }
                    else
                    {
                        // Regular command
                        retval = execute(cmd);
                        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
                        continue;
                    }
                }
        }
        return EXIT_SUCCESS;
    };
