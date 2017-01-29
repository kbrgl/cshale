#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<signal.h>
#include<curses.h>
#include<string.h>
#include"styles.h"

#define PROGRAM_NAME "cshale"
#define GREETING "Welcome to "PROGRAM_NAME
#define TRUE 1
#define FALSE 0
#define PROMPT "$ "
#define INPUT_BUFSIZE 2048
#define INPUT_DELIM " \n\r\t"

char * reads();
int exec(char *cmd);
int repl();
void sigint_handler();
void prompt();

int main(int argc, char **argv) {
    signal(SIGINT, sigint_handler);
    puts(GREETING);
    return repl();
}

char * reads() {
    int bufsize = INPUT_BUFSIZE;
    char *buf = malloc(sizeof(char) * bufsize);

    if (!buf) return NULL;

    int i = 0;
    int c;

    for (;;) {
        c = getchar();
        if (c == '\n') {
            buf[i] = '\0';
            return buf;
        } else if (c == EOF) {
            if (i == 0) {
                return NULL;
            } else {
                buf[i] = '\0';
                return buf;
            }
        } else {
            buf[i] = c;
        }
        i++;
        if (i >= bufsize) {
            bufsize += INPUT_BUFSIZE;
            buf = realloc(buf, sizeof(char) * bufsize);

            if (!buf) return NULL;
        }
    }
}

int strslice(char *str, int begin, int len)
{
    int l = strlen(str);

    if (len < 0) len = l - begin;
    if (begin + len > l) len = l - begin;
    memmove(str + begin, str + begin + len, l - len + 1);

    return len;
}

int launch_prog(char **argv) {
    int status;
    pid_t pid, wpid;
    pid = fork();
    if (pid == 0) {
        if (execvp(argv[0], argv) == -1) {
            exit(EXIT_FAILURE);
        }
    } else if (pid == -1) {
        return -1;
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return WEXITSTATUS(status);
}

char ** tokenize(char* string) {
    int bufsize = INPUT_BUFSIZE / 4;
    char** toks = malloc(sizeof(char*) * bufsize);
    if (!toks) return NULL;
    int i = 0;

    char* token = strtok(string, INPUT_DELIM);
    while (token != NULL) {
        toks[i] = token;
        i++;
        if (toks[i] == '\0') {
            bufsize += INPUT_BUFSIZE / 4;
            toks = realloc(toks, sizeof(char*) * bufsize);
            if (!toks) return NULL;
        }
        token = strtok(NULL, INPUT_DELIM);
    }
    return toks;
}

int cd(char **argv) {
    int len;
    for(len = 0; argv[len] != '\0'; len++);
    if (len < 2) {
        puts("error: cd requires path argument");
        return EXIT_FAILURE;
    }
    char *path = argv[1];
    if (path[0] == '~'){
        int pathlen;
        for(pathlen = 0; path[pathlen] != '\0'; pathlen++);
        if (pathlen > 2) {
            strslice(path, 0, 2);
            int rc = chdir(getenv("HOME"));
            if (rc != 0) return rc;
            return chdir(path);
        } else {
            return chdir(getenv("HOME"));
        }

    }
    return chdir(path);
}

int exec(char *cmd) {
    char **argv = tokenize(cmd);
    if (argv == NULL) return EXIT_FAILURE;

    // TODO: refactor this. Make something more general which doesn't require me
    // to add new if statements each time.
    if (strcmp(argv[0], "cd") == 0)
        return cd(argv);
    else if (strcmp(argv[0], "exit") == 0)
        exit(EXIT_SUCCESS);
    else
        return launch_prog(argv);
}

void sigint_handler() {
    // do nothing on SIGINT
}

int repl() {
    int exit_code = EXIT_SUCCESS;
    while(TRUE) {
        prompt(exit_code);
        char* cmd = reads();
        if (cmd == NULL) return EXIT_FAILURE;
        exit_code = exec(cmd);
        free(cmd);
    }
    return EXIT_SUCCESS;
}

void prompt(int exit_code) {
    if (exit_code != EXIT_SUCCESS) {
        printf("(%i) ", exit_code);
        fputs(COLOR_FG_RED, stdout);
    } else {
        fputs(COLOR_FG_BLUE, stdout);
    }
    fputs(PROMPT COLOR_FG_DEFAULT, stdout);
    fflush(stdout);
}
