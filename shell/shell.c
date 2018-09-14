#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &shell_cd,
  &shell_help,
  &shell_exit
};

int num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int shell_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "> esperando argumento para \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("> ");
    }
  }
  return 1;
}


int shell_help(char **args)
{
  int i;
  printf("digitar uma da funções e pressione enter.\n");
  printf("Funções:\n");

  for (i = 0; i < num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  return 1;
}

int shell_exit(char **args)
{
  return 0;
}


int shell_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("> ");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("> ");
  } else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}


int shell_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    return 1;
  }

  for (i = 0; i < num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return shell_launch(args);
}

#define READ_BUFSIZE 1024

char *read_line(void)
{
  int bufsize = READ_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, ">: erro de alocação\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    
    if (position >= bufsize) {
      bufsize += READ_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, ">: erro de alocação\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define PARSE_BUFSIZE 64
#define PARSE_DELIM " \t\r\n\a"

char **shell_parse(char *line)
{
  int bufsize = PARSE_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, ">: erro de alocação\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, PARSE_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += PARSE_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, ">: erro de alocação\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, PARSE_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

void loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = read_line();
    args = shell_parse(line);
    status = shell_execute(args);

    free(line);
    free(args);
  } while (status);
}


int main(int argc, char **argv)
{
  
  loop();

  return EXIT_SUCCESS;
}