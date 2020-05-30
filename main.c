#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

// TODO : cosmetic changes, maybe rename some of the stuff here, try different ways of implementing some of the functions

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
#define LSH_RL_BUFSIZE 1024

//Function declaration for builtin shell commands

int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

//list of builtin commands, followed by their corresponding functions

char *builtin_str[] = {
    "cd",
    "help",
    "exit",
};

int (*builtin_func[]) (char **) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit,
};

int lsh_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

//builtin function implementation

int lsh_cd(char **args)
{
    if(args[1] == NULL){
        fprintf(stderr, "lsh: EXPECTED ARGUMENT TO \"cd\"\n");
    }else{
        if(chdir(args[1]) != 0){
            perror("lsh");
        }
    }

    return 1;
}

int lsh_help(char **args)
{
    int i;
    printf("Prototype \n");
    printf("Type in program names and args, then hit enter\n");
    printf("The following are built in: \n");

    for(i = 0; i < lsh_num_builtins(); i++){
        printf("%s\n", builtin_str[i]);
    }

    printf("Use the man command for info on other programs\n");
    return 1;
}

int lsh_exit(char **args)
{
    return 0;
}

int lsh_execute(char **args)
{
    int i;

    if(args[0] == NULL){
        //empty command
        return 1;
    }

    for(i = 0; i < lsh_num_builtins(); i++){
        if(strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }

    return lsh_launch(args);
}


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
