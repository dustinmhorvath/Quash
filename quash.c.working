#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#define MAXPATHNUM 24
#define MAXPATHLENGTH 1024
#define MAX_ARGV 15
#define MAX_ARG_LEN 128

#define _PROGRAM_NAME "whoami"
#define TRUE (!FALSE)
#define FALSE (0)
#define BUFFER_LENGTH 2048


// Execute from pipe lab
void execute(char ***argv, int flag, char **environment) {
  pid_t pid;

  int status;

  if ((pid = fork()) < 0) {
    printf("Could not create fork.\n");
    exit(1);
  }
  else if (pid == 0) {
    errno = 0;
    if (execvpe(**argv, *argv, environment) < 0) {
      printf("%s\n", strerror(errno));
      exit(1);
    }
  }
  else {
    if (flag == FALSE)
      while (wait(&status) != pid){}
    else
      printf("[1] %i\n", pid);
  }

  return;
}


int change_dir(char *buffer) {
  int error_code = 0;
  // Declare a buffer guaranteed(ish) to hold the input
  char buffer_excessive[2048];
  // Make copy of the input
  strcpy(buffer_excessive, buffer);
  // Split the buffer up using newline as delimiter
  char* pch = strtok(buffer_excessive, " \n");

  // strncmp only compares the first 'count' characters of the string 
  // it's given, in this case, 2 and then 5
  if (strncmp(pch, "cd", 2) == 0 || strncmp(pch, "chdir", 5) == 0) {
    // strtok is deprecated, but still works.
    pch = strtok(NULL, " \n");
    if (pch == NULL ) {
      errno = 0;

      // If their input after 'cd' is null, send to their HOME path
      error_code = chdir(getenv("HOME"));
      if (error_code < 0) {
        printf("%s\n", strerror(errno));
        return FALSE;
      }
      else {
        return TRUE;
      }
    }
    // Check first character of second argument to see if it starts with /
    else if (strncmp(pch, "/", 1) == 0) {
      errno = 0;
      error_code = chdir(pch);

      if (error_code < 0) {
        printf("%s\n", strerror(errno));
        return FALSE;
      }
      else {
        return TRUE;
      }
    }
    else {
      // Case for relative paths
      errno = 0;

      char directory[MAXPATHLENGTH];
      // gets the current directory
      getcwd(directory, sizeof(directory));
      // build the new path with concatenate. '..' should still work.
      strcat(directory, "/");
      strcat(directory, pch);
      // Swith to the new built one
      error_code = chdir(directory);
      // Barf on user if they type something silly
      if (error_code < 0) {
        printf("%s\n", strerror(errno));
        return FALSE;
      }
      else {
        return TRUE;
      }

    }

  }
  return FALSE;
}


// Send output of an execute to file
void execute_redirect_out(char ***argv, int background, char **environment, char* path){
  pid_t pid;
  int status;

  if ((pid = fork()) < 0) {
    printf("Could not create fork.\n");
    exit(1);
  }
  else if (pid == 0) {
    errno = 0;
    FILE* filepointer;
    filepointer = fopen(path, "w");
    dup2(fileno(filepointer), STDOUT_FILENO);
    fclose(filepointer);
    if (execvpe(**argv, *argv, environment) < 0) {
      printf("%s\n", strerror(errno));
      exit(1);
    }
  }
  else {
    if (background == FALSE){
      while (wait(&status) != pid){}
    }
    else{
      printf("[1] %i\n", pid);
    }
  }

  return;
}

// Execute but with file input
void execute_redirect_in(char ***argv, int background, char **environment, char* path) {
  pid_t pid;
  int status;

  if ((pid = fork()) < 0) {
    printf("Could not create fork.\n");
    exit(1);
  } else if (pid == 0) {
    errno = 0;
    FILE *filepointer;
    filepointer = fopen(path, "r");
    dup2(fileno(filepointer), STDIN_FILENO);
    fclose(filepointer);
    if (execvpe(**argv, *argv, environment) < 0) {
      printf("%s\n", strerror(errno));
      exit(1);
    }
  } else {
    if (background == FALSE)
      while (wait(&status) != pid)
        ;
    else
      printf("[1] %i\n", pid);
  }

  return;
}

//Execute two commands with a pipe
// Arguments
// argv1 - arguments for command 1
// background1 - 1/0 to indicate wether to run in backround
// env1 - enviroment for command 1
void execute_piped(char ***argv1, int background1, char **env1, char ***argv2,
    int background2, char **env2) {

  //Init. PID Containers		
  pid_t pid, pid2;

  //Init. Status Containers
  int status, status2;

  //Pipe Container
  int io[2];

  //Pipe It
  pipe(io);

  //Fork and stop if we get an error
  if ((pid = fork()) < 0) {
    printf("Could not create fork.\n");
    exit(1);
  } else if (pid == 0) {
    //We are in a fork(child process)
    errno = 0;
    //Close unused end of pipe
    close(io[0]);

    //replace STDDOUT with write end of pipe
    dup2(io[1], STDOUT_FILENO);

    //Execute command 1
    if (execvpe(**argv1, *argv1, env1) < 0) {
      printf("%s\n", strerror(errno));
      exit(1);
    }
  } else {
    //First proecss cannot be in  backround
    if (background1 == FALSE) {

    }

    else {
      printf("[1] %i\n", pid);
    }
  }

  //Do same as above only set STDIN to read end of pipe.
  if ((pid2 = fork()) < 0) {
    printf("Could not create fork.\n");
    exit(1);
  } else if (pid2 == 0) {
    errno = 0;

    close(io[1]);
    dup2(io[0], STDIN_FILENO);
    if (execvpe(**argv2, *argv2, env2) < 0) {
      printf("%s\n", strerror(errno));
      exit(1);
    }

  } else {
    if (background2 == FALSE) {

    }

    else {
      printf("[1] %i\n", pid);
    }
  }

  //Close pipe completely
  close(io[1]);
  close(io[0]);

  //Wait for child2 to finish before we return the call.
  while (waitpid(pid2, &status, 0) != pid2) {
  };
  return;
}

void execute_piped_out_redir(char ***argv1, int background1, char **env1,
    char ***argv2, int background2, char **env2, char* path) {
  pid_t pid, pid2;
  int status, status2;
  int io[2];

  pipe(io);

  if ((pid = fork()) < 0) {
    printf("*** ERROR: forking child process failed\n");
    exit(1);
  } else if (pid == 0) {
    errno = 0;
    close(io[0]);
    dup2(io[1], STDOUT_FILENO);

    if (execvpe(**argv1, *argv1, env1) < 0) {
      printf("%s\n", strerror(errno));
      exit(1);
    }
  } else {
    if (background1 == FALSE) {

    }

    else {
      printf("[1] %i\n", pid);
    }
  }

  if ((pid2 = fork()) < 0) {
    printf("*** ERROR: forking child process 2 failed\n");
    exit(1);
  } else if (pid2 == 0) {
    errno = 0;

    close(io[1]);
    dup2(io[0], STDIN_FILENO);
    FILE *filepointer;
    filepointer = fopen(path, "w");
    dup2(fileno(filepointer), STDOUT_FILENO);
    fclose(filepointer);
    if (execvpe(**argv2, *argv2, env2) < 0) {
      printf("%s\n", strerror(errno));
      exit(1);
    }

  } else {
    if (background2 == FALSE) {

    }

    else {
      printf("[1] %i\n", pid);
    }
  }
  close(io[1]);
  close(io[0]);

  while (waitpid(pid2, &status, 0) != pid2) {
  };
  return;
}

//What kind of commands are available. 
//This is a relic of my shattered dream to write my own parser.
typedef enum {
  REG, PIPE, REDIR_IN, REDIR_OUT
} cmd_type;


// Declare structure that holds everything needed for a command
typedef struct {
  char** argv;
  int argc;
  char **envv;
  cmd_type com_type;
  int is_background;
  char target[MAXPATHLENGTH];
} CommandStruct;


//Given a string of a single command with arguments and an enviroment,
//returns a pointer to a CommandStruct that has been mallocaed
CommandStruct* parse_to_command(char* buffer, char **env) {

  // Make buffer large enough to always hold the input
  char buffer_excessive[256];
  char* pch;
  char** argv = (char**) malloc(sizeof(char*) * MAX_ARGV);
  char** argv2;
  int argc = 0;
  int background;
  char* command_buffer;

  CommandStruct* command;

  command = (CommandStruct*) malloc(sizeof(CommandStruct));


  strcpy(buffer_excessive, buffer);

  //Tokenize on spaces
  pch = strtok(buffer_excessive, " \n");
  while (pch != NULL ) {
    command_buffer = (char*) malloc((strlen(pch) * sizeof(char)) + 1);

    memset(command_buffer, '\0', (strlen(pch) * sizeof(char)) + 1);

    strcpy(command_buffer, pch);

    argv[argc] = command_buffer;

    command_buffer = NULL;

    // strtok is deprecated, but I keep breaking things with alternatives
    pch = strtok(NULL, " \n");
    argc++;
  }

  // Check if this is a background operation
  if (argc != 0) {
    if (strcmp(argv[argc - 1], "&") == 0) {
      background = TRUE;
      argv[argc - 1][0] = '\0';
    } else {
      background = FALSE;
    }
  }

  argv2 = (char**) malloc(sizeof(char*) * (argc + 1));
  int i = 0;
  for (i = argc; i < MAX_ARGV - 1; i++) {

    argv[argc] = '\0';
  }

  //Set our command to its contents
  command->argc = argc;
  command->is_background = background;
  command->argv = argv;
  command->com_type = REG;
  command->envv = env;

  //Return the pointer to the malloced CommandStruct
  return command;
}

char* check(char* string) {
  read(0, string, BUFFER_LENGTH);
  return string;
}

int main(int argc, char **argv, char **envp) {
  char line[BUFFER_LENGTH];

  uid = geteuid();
  pw = getpwuid(uid);

  // the holder for search paths
  char search[MAXPATHNUM][MAXPATHLENGTH];
  int path_len = 0;

  register struct passwd *pw;
  register uid_t uid;
  char **env;
  env = envp;

  int dirchange;

  char* pipeptr;
  char* inptr;
  char* outptr;
  CommandStruct* basic;
  CommandStruct* firstC;
  CommandStruct* SecondC;
  char *buffer;

  dirchange = -1;
  puts("Welcome to Quash. Type 'Exit' when finished.");

  while (TRUE) {
    // Print a prompt
    printf("Quash: ");
    fflush(stdout);

    //Make sure our line/buffer is indeed empty
    memset(line, '\0', sizeof(line));

    //Set line to stuff from terminal
    //Also set buffer to alias line
    buffer = check(line);

    //Check for various things in the line
    //to determine what combination of commands to run
    pipeptr = strpbrk(buffer, "|");
    inptr = strpbrk(buffer, "<");
    outptr = strpbrk(buffer, ">");

    //Handle exit command
    if (strncmp(buffer, "exit", 4) == 0
        || strncmp(buffer, "quit", 4) == 0) {
      return EXIT_SUCCESS;
    }

    //Handle CD command
    if (strncmp(buffer, "cd", 2) == 0) {
      change_dir(buffer);
    } else {
      //If we have an inbound redirect, nothing else can happen.
      if (inptr != NULL ) {
        char* pch = strtok(buffer, "<\n");
        char first[256];
        strcpy(first, pch);
        char second[256];
        pch = strtok(NULL, "<\n");
        strcpy(second, pch);

        basic = parse_to_command(first, env);
        execute_redirect_in(&(basic->argv), basic->is_background,
            basic->envv, second);

        memset(first, '\0', sizeof(char) * 256);
        memset(second, '\0', sizeof(char) * 256);
        memset(buffer, '\0', sizeof(buffer));
        free(basic);
        basic = NULL;
      } else {
        //Otherwise, figure out what combo we have and run it.
        if (pipeptr != NULL && outptr != NULL ) {
          //Redirect out AND pipe

          //Tokenize once on pipe ( } )
          char* pch = strtok(buffer, "|\n");
        char first[256];
        strcpy(first, pch);
        char second[256];
        pch = strtok(NULL, "|\n");
        strcpy(second, pch);

        char third[256];

        //Tokenzie twice on >
        //Being sure to copy stuff to buffers
        //to avoid tampering with the pointer
        pch = strtok(second, ">\n");
        strcpy(third, pch);
        char fourth[256];
        pch = strtok(NULL, ">\n");
        strcpy(fourth, pch);


        //Build our commands
        firstC = parse_to_command(first, env);

        SecondC = parse_to_command(third, env);


        //Execute everything
        execute_piped_out_redir(&(firstC->argv),
            firstC->is_background, firstC->envv,
            &(SecondC->argv), SecondC->is_background,
            SecondC->envv, fourth);

        //Free and clear stuff
        free(firstC);
        free(SecondC);
        firstC = NULL;
        SecondC = NULL;
        memset(first, '\0', sizeof(char) * 256);
        memset(second, '\0', sizeof(char) * 256);
        memset(third, '\0', sizeof(char) * 256);
        memset(fourth, '\0', sizeof(char) * 256);
        memset(buffer, '\0', sizeof(buffer));
      } else if (pipeptr != NULL && outptr == NULL ) {
        //Just a pipe

        char* pch = strtok(buffer, "|\n");
        char first[256];
        strcpy(first, pch);
        char second[256];
        pch = strtok(NULL, "|\n");
        strcpy(second, pch);
        firstC = parse_to_command(first, env);

        SecondC = parse_to_command(second, env);

        execute_piped(&(firstC->argv), firstC->is_background,
            firstC->envv, &(SecondC->argv),
            SecondC->is_background, SecondC->envv);
        free(firstC);
        free(SecondC);
        firstC = NULL;
        SecondC = NULL;
        memset(first, '\0', sizeof(char) * 256);
        memset(second, '\0', sizeof(char) * 256);
        memset(buffer, '\0', sizeof(buffer));

      } else if (pipeptr == NULL && outptr != NULL ) {

        char* pch = strtok(buffer, ">\n");
        char first[256];
        strcpy(first, pch);
        char second[256];
        pch = strtok(NULL, ">\n");
        strcpy(second, pch);

        basic = parse_to_command(first, env);
        execute_redirect_out(&(basic->argv), basic->is_background,
            basic->envv, second);

        memset(first, '\0', sizeof(char) * 256);
        memset(second, '\0', sizeof(char) * 256);
        memset(buffer, '\0', sizeof(buffer));
        free(basic);
        basic = NULL;

        //Just an outbound redirect
      } else if (pipeptr == NULL && outptr == NULL ) {
        basic = parse_to_command(buffer, env);

        execute(&(basic->argv), basic->is_background, basic->envv);
        memset(basic, 0, sizeof(*basic));
        memset(buffer, '\0', sizeof(buffer));
        free(basic);
        basic = NULL;

      }
    }
  }

  //Clear buffer/line again. Just to be sure
  memset(buffer, '\0', sizeof(buffer));
  memset(line, '\0', sizeof(line));

}
return EXIT_SUCCESS;
}
