/**
 * @file quash.c
 *
 * Quash's main file
 */ /**************************************************************************
 * Included Files
 **************************************************************************/
#include "quash.h" // Putting this above the other includes allows us to ensure
// this file's headder's #include statements are self
// contained.

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

/**************************************************************************
 * Private Variables
 **************************************************************************/ /**
 * Keep track of whether Quash should request another command or not.
 */ // NOTE: "static" causes the "running" variable to only be declared in this // compilation unit (this file and all files that include it). This is similar 
// to private in other languages. 

#define BSIZE 1024
#define MAXPIPES 12

static bool running;

/**************************************************************************
 * Private Functions
 ************************************************************************** 
 * Start the main loop by setting the running flag to true
 */ 

static void start() {
  running = true;
}

/**************************************************************************
 * Public Functions
 **************************************************************************/ 

bool is_running() {
  return running;
}

void terminate() {
  running = false;
}

bool get_command(command_t* cmd, FILE* in) {
  printf("Quash: ");
  if (fgets(cmd->cmdstr, MAX_COMMAND_LENGTH, in) != NULL) {
    size_t len = strlen(cmd->cmdstr);
    char last_char = cmd->cmdstr[len - 1];
    if (last_char == '\n' || last_char == '\r') {
      // Remove trailing new line character.
      cmd->cmdstr[len - 1] = '\0';
      cmd->cmdlen = len - 1;
    }
    else
      cmd->cmdlen = len;

    return true;
  }
  else
    return false;
}

char* local_path;
char* local_term;
char* local_user;
char* local_home;
char* local_pwd;

char** str_split(char* a_str, const char a_delim){
  char** result    = 0;
  size_t count     = 0;
  char* tmp        = a_str;
  char* last_comma = 0;
  char delim[2];
  delim[0] = a_delim;
  delim[1] = 0;

  while (*tmp){
    if (a_delim == *tmp){
      count++;
      last_comma = tmp;
    }
    tmp++;
  }

  count += last_comma < (a_str + strlen(a_str) - 1);
  count++;

  result = malloc(sizeof(char*) * count);

  if (result){
    size_t idx  = 0;
    char* token = strtok(a_str, delim);

    while (token){
      assert(idx < count);
      *(result + idx++) = strdup(token);
      token = strtok(0, delim);
    }
    assert(idx == count - 1);
    *(result + idx) = 0;
  }

  return result;
}

char** parseCommand(char* command){
  return str_split(command, " ");
}

void removeSpaces(char* source){
  char* i = source;
  char* j = source;
  while(*j != 0){
    *i = *j++;
    if(*i != ' ' || *i != '\n' || *i != '\r')
      i++;
  }
  *i = 0;
}

void set(char* input){
  int length = strlen(input);
  if(!strncmp(input, "PATH=", 5)){
    char *pathbuffer = malloc (length + 1);
    if (pathbuffer == NULL) {
      puts("Out of memory.");
      terminate();
    }
    strcpy (pathbuffer, input);
    free(local_path);
    local_path = pathbuffer;
  }
  else if(!strncmp(input, "HOME=", 5)){
    char *homebuffer = malloc (length + 1);
    if (homebuffer == NULL) {
      puts("Out of memory.");
      terminate();
    }
    puts("I got here.");
    strcpy (homebuffer, input);
    free(local_home);
    local_home = homebuffer;
    //local_home = input;
  }
  else{
    puts("Invalid.");
  }

}

int exec_command(char* input){

  char** tokens = str_split(input, '|');
  pid_t parent = getpid();
  int n;
  int status;
  int io[MAXPIPES][2];
  char buf[BSIZE];
  int numbercommands = 0;

  if (tokens){

    for (numbercommands = 0; *(tokens + numbercommands); numbercommands++){
      // count the number of commands
    }
    // Declare pipe ints and instantiate pipes dynamically
    if(numbercommands > 1){
      for(int pipeindex = 0; pipeindex < (numbercommands - 1); pipeindex++){
        pipe(io[pipeindex]);
      }
    }

    pid_t pids[numbercommands];

    int i;
    for (i = 0; i < numbercommands; i++){

      // Confirmed getting command names correctly after each pipe
      char* command = *(tokens + i);

      if(!strncmp(command, "cd ", 3)){
        // Cut off the "cd "
        char *truncated = (char*) malloc(sizeof(command) - 3);
        strncpy(truncated, command + 3, strlen(command));
        // Strip any whitespace
        removeSpaces(truncated);
        // Pass new string path to function
        chdir(truncated);
      }

      else if(!strncmp(command, "set ", 4)){
        char *truncated = (char*) malloc(sizeof(command) - 4);
        strncpy(truncated, command + 4, strlen(command));
        removeSpaces(truncated);
        set(truncated);
      }

      else{
        pids[i] = fork();
        if (pids[i] == 0){
          // Handle closing of pipes dynamically
          // first command case (closes read end)
          if(i == 0 && numbercommands > 1){
            for(int pipeindex = 1; pipeindex < (numbercommands - 1); pipeindex++){
              close(io[pipeindex][0]);
              close(io[pipeindex][1]);
              close(io[i][0]);
            }
            dup2(io[i][1], STDOUT_FILENO);
          }
          // Handle last command in pipe sequence
          else if(i != 0 && i == numbercommands - 1){
            for(int pipeindex = 0; pipeindex < (numbercommands - 1) - 1; pipeindex++){
              close(io[pipeindex][0]);
              close(io[pipeindex][1]);
              close(io[i-1][1]);
            }
            dup2(io[i-1][0], STDIN_FILENO);
          }

          char *env[] = {
            local_term,
            local_home,
            local_path,
            local_user,
            0
          };

          char *argv[] = { "/bin/sh", "-c", command, 0 };

          status = execve(argv[0], &argv[0], env);

          exit(status);
        }
        else if( pids[i] == -1){
          perror("fork() failed");
          exit(EXIT_FAILURE);
        }

      } // ends fork else
      free(*(tokens + i));

    }

    for(int pipeindex = 0; pipeindex < (numbercommands - 1); pipeindex++){
      close(io[pipeindex][0]);
      close(io[pipeindex][1]);
    }

    for(int i = 0; i < numbercommands; i++){
      printf("waiting on pid %d\n", pids[i]);
      if (waitpid(pids[i], &status, 0) == -1) {
        // fprintf(stderr, "Process %d encountered an error. ERROR%d", i, errno);
        return EXIT_FAILURE;
      } 
    }

    //printf("\n");
    free(tokens);
  }
}

void storeEnv(){
  local_path = "PATH=";
  local_term = "TERM=";
  local_user = "USER=";
  local_home = "HOME=";
  char *pathbuffer = malloc (strlen (local_path) + strlen (getenv("PATH")) + 1);
  char *termbuffer = malloc (strlen (local_term) + strlen (getenv("TERM")) + 1);
  char *userbuffer = malloc (strlen (local_user) + strlen (getenv("USER")) + 1);
  char *homebuffer = malloc (strlen (local_home) + strlen (getenv("HOME")) + 1);
  if (homebuffer == NULL) {
    puts("Out of memory.");
    terminate();
  }
  strcpy (pathbuffer, local_path);
  strcpy (termbuffer, local_term);
  strcpy (userbuffer, local_user);
  strcpy (homebuffer, local_home);
  strcat(pathbuffer, getenv("PATH"));
  strcat(termbuffer, getenv("TERM"));
  strcat(userbuffer, getenv("USER"));
  strcat(homebuffer, getenv("HOME"));
  local_path = pathbuffer;
  local_term = termbuffer;
  local_user = userbuffer;
  local_home = homebuffer;

}

/**
 * Quash entry point
 *
 * @param argc argument count from the command line
 * @param argv argument vector from the command line
 * @return program exit status
 */ 
int main(int argc, char** argv) {
  command_t cmd; //< Command holder argument
  storeEnv();

  start();

  if(argc > 1){
    int arglength = argc - 2;
    for(int i = 1; i < argc; i++){
      arglength = arglength + strlen(argv[i]);
    }
    char *commandbuffer = malloc (arglength + 2);
    printf("%d\n",arglength);
    for(int i = 1; i < argc; i++){
      strcat(commandbuffer, argv[i]);
      strcat(commandbuffer, " ");
    }
    exec_command(commandbuffer);


    terminate();
  }


  puts("Welcome to Quash!");
  puts("Type \"exit\" to quit");
  // Main execution loop
  while (is_running() && get_command(&cmd, stdin)) {
    // NOTE: I would not recommend keeping anything inside the body of
    // this while loop. It is just an example.
    // The commands should be parsed, then executed.
    if (!strcmp(cmd.cmdstr, "exit") || !strcmp(cmd.cmdstr, "quit")){
      puts("Bye");
      terminate(); // Exit Quash
    }
    else{
      exec_command(cmd.cmdstr); 



      //puts(cmd.cmdstr); // Echo the input string
    }
  }
  return EXIT_SUCCESS;
}
