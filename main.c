#include <stdio.h>
#include <stdlib.h>

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
#define LSH_RL_BUFSIZE 1024

//Function declaration for builtin shell commands

int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

//list of builtin commands, followed by their corresponding functions







char *lsh_read_line(void)
{
    int bufsize = LSH_RL_BUFSIZE;
    int pos = 0;  // position
    char *buffer = malloc(sizeof(char)*bufsize);
    int c;

    if(!buffer){
        fprintf(stderr, "lsh: ALLOCATION ERROR \n");
        exit(EXIT_FAILURE);
    }

    while(1){
        c = getchar(); //read a character;

        //if we hit EOF, replace it with a null char and return
        if(c == EOF || c == "\n"){
            buffer[pos] = "\0";
            return buffer;
        }else{
            buffer[pos] = c;
        }
        pos++;

        //if buffer is exceeded, then reallocate
        if(pos >= bufsize){
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer){
                fprintf(stderr, "lsh: ALLOCATION ERROR\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}


char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE, pos = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if(!tokens){
        fprintf(stderr, "lsh: ALLOCATION ERROR\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);

    while(token != NULL){
        tokens[pos] = token;
        pos++;

        if(pos >= bufsize){
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));

            if(!tokens){
                fprintf(stderr, "lsh: ALLOCATION ERROR\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }

    tokens[pos] = NULL;
    return tokens;
}


int lsh_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();

    if(pid == 0){
        //child process
        if(execvp(args[0], args) == 1){
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }else if (pid < 0){
        //error forking
        perror("lsh");

    }else{
        //parent process
        do{
           wpid = waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status)&&!WIFSIGNALED(status));
    }

    return 1;
}



void lsh_loop(void)   //loop interpreting commands
{
    char *line;
    char **args;
    int status;

    do{
        printf("> ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);

    }while(status);

}



int main(int argc, char **argv)
{
    //Load config files

    //Run command loop

    lsh_loop();

    //Perform shutdown/cleanup

    return EXIT_SUCCESS;
}
