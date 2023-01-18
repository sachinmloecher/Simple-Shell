all:
	gcc -Wall -Wextra -Werror sshell.c -o sshell

clean:
	rm $(sshell)