#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

//function that splits input command into seperate args
//ex: "ls -l /home" -> ["ls","-l","/home",NULL]
void parse_command(char *input, char **args) {
    char *token = strtok(input, " \n");
    int i = 0;

    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \n");
    }
    args[i] = NULL;
}
//handle built-in shell commands(cd and exit)
// Returns: 0 for exit, 1 for other built-ins, 2 for not a built-in
int execute_builtin(char** args) {

    //handle 'cd'
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            //cd needs a directory arg
            fprintf(stderr, "Expected argument to \"cd\"\n");
        } else {
            //chdir changes current directory, 0 on success
            if (chdir(args[1]) != 0) {
                perror("shell");
            }
        }

        return 1;
    } else if (strcmp(args[0], "exit") == 0) { //handle exit
        return 0;
    }
    return 2;
}


int main(void) {
    //store input command and parsed arguments
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARGS];
    int status = 1;


    while (1) {
        printf("myshell> ");
        fflush(stdout);

        //read input
        if (!fgets(input, MAX_INPUT_SIZE, stdin)) {
            break;
        }

        //remove trailing newline
        input[strcspn(input, "\n")] = 0;

        //parse command
        parse_command(input, args);

        if (args[0] == NULL) continue; // empty command

        //check for built-in commands
        int builtin_status = execute_builtin(args);
        if (builtin_status != 2) {
            status = builtin_status;
            continue;
        }

        //create new process
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }
        else if (pid == 0) { //child process
            //replace child process with new command
            execvp(args[0], args);
            //if execvp returns, failure
            perror("shell");
            exit(1);
        } else {
            //wait for child process to complete
            waitpid(pid, NULL, 0);
        }

    }
    return 0;

}