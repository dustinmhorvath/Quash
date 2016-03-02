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

#define BSIZE 256
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
  while(*j != 0)
  {
    *i = *j++;
    if(*i != ' ')
      i++;
  }
  *i = 0;
}

void changeDirectory(char* path){
  // Absolute path case
  if(!strncmp(path, "/", 1)){
    char *temp_pwd = "PWD=";
    char *temppwdbuffer = malloc (strlen (temp_pwd) + strlen (path) + 1);
    if (temppwdbuffer == NULL) {
      puts("Out of memory, discarding command");
    }
    strcpy(temppwdbuffer, temp_pwd);
    strcat(temppwdbuffer, path);
    free(local_pwd);
    local_pwd = temppwdbuffer;

  }
  else{
    puts("I don't handle relative paths yet.");
  }

}

int exec_command(char* input){

  char** tokens = str_split(input, '|');
  pid_t parent = getpid();
  int n;
  int status;
  int io[MAXPIPES][2];
  char buf[BSIZE];

  if (tokens){

    int count;
    for (count = 0; *(tokens + count); count++){
      // count the number of commands
    }
    // Declare pipe ints and instantiate pipes dynamically
    if(count > 1){
      for(int pipeindex = 0; pipeindex < (count - 1); pipeindex++){
        pipe(io[pipeindex]);
      }
    }

    pid_t pids[count];

    int i;
    for (i = 0; i < count; i++){

      char** command = parseCommand( *(tokens + i) );

      if(!strncmp(command[0], "cd ", 3)){
        // Cut off the "cd "
        char *truncated = (char*) malloc(sizeof(command) - 3);
        strncpy(truncated, command[0] + 3, strlen(command[0]));
        // Strip any whitespace
        removeSpaces(truncated);
        // Pass new string path to function
        changeDirectory(truncated);
      }

      pids[i] = fork();
      if (pids[i] == 0){
        // Handle closing of pipes dynamically
        // first command case (closes read end)
        if(i == 0 && count > 1){
          for(int pipeindex = 1; pipeindex < (count - 1); pipeindex++){
            close(io[pipeindex][0]);
            close(io[pipeindex][1]);
            close(io[i][0]);
          }
          dup2(io[i][1], STDOUT_FILENO);
          }
        // Handle last command in pipe sequence
        else if(i != 0 && i == count - 1){
          for(int pipeindex = 0; pipeindex < (count - 1) - 1; pipeindex++){
            close(io[pipeindex][0]);
            close(io[pipeindex][1]);
            close(io[i-1][1]);
          }
          dup2(io[i-1][0], STDIN_FILENO);
        }

        // Special cases 

        if(!strncmp(command[0], "cd ", 3)){
          // Cut off the "cd "
          char *truncated = (char*) malloc(sizeof(command) - 3);
          strncpy(truncated, command[0] + 3, strlen(command[0]));
          // Strip any whitespace
          removeSpaces(truncated);

          changeDirectory(truncated);
        }
        else if(!strncmp(command[0], "set ", 4)){
          puts("read set");
        }

        // If not a special case, execute using sh and env
        else{
          char *env[] = {
            local_pwd,
            local_term,
            local_home,
            local_path,
            local_user,
            0
          };

          char *argv[] = { "/bin/sh", "-c", command[0], 0 };
          /*
             if((status = execve(argv[0], &argv[0], env)) < 0){
             puts("Error on execve.");
             return EXIT_FAILURE;
             }
             */
          execve(argv[0], &argv[0], env);

        }
        for(int pipeindex = 0; pipeindex < (count - 1); pipeindex++){
          close(io[pipeindex][0]);
          close(io[pipeindex][1]);
        }

        exit(0);
      }

      free(*(tokens + i));
    }

    for(int i = 0; i < count; i++){
      if ((waitpid(pids[i], &status, 0)) == -1) {
        fprintf(stderr, "Process %d encountered an error. ERROR%d", i, errno);
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
  local_pwd = "PWD=";
  local_home = "HOME=";
  char *pathbuffer = malloc (strlen (local_path) + strlen (getenv("PATH")) + 1);
  char *termbuffer = malloc (strlen (local_term) + strlen (getenv("TERM")) + 1);
  char *userbuffer = malloc (strlen (local_user) + strlen (getenv("USER")) + 1);
  char *pwdbuffer = malloc (strlen (local_pwd) + strlen (getenv("PWD")) + 1);
  char *homebuffer = malloc (strlen (local_home) + strlen (getenv("HOME")) + 1);
  if (homebuffer == NULL) {
    puts("Out of memory.");
    terminate();
  }
  strcpy (pathbuffer, local_path);
  strcpy (termbuffer, local_term);
  strcpy (userbuffer, local_user);
  strcpy (pwdbuffer, local_pwd);
  strcpy (homebuffer, local_home);
  strcat(pathbuffer, getenv("PATH"));
  strcat(termbuffer, getenv("TERM"));
  strcat(userbuffer, getenv("USER"));
  strcat(pwdbuffer, getenv("PWD"));
  strcat(homebuffer, getenv("HOME"));
  local_path = pathbuffer;
  local_term = termbuffer;
  local_user = userbuffer;
  local_pwd = pwdbuffer;
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
