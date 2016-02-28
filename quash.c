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

/**************************************************************************
 * Private Variables
 **************************************************************************/ /**
 * Keep track of whether Quash should request another command or not.
 */ // NOTE: "static" causes the "running" variable to only be declared in this // compilation unit (this file and all files that include it). This is similar 
// to private in other languages. 

#define BSIZE 256

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

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

int exec_command(char* input){
  char** tokens = str_split(input, '|');
  int status;
  int n;
  int io[2];
  int pid;
  char buf[BSIZE];
  bool first = true;


  if (tokens){
    int i;
    for (i = 0; *(tokens + i); i++){
      if ((pid=fork()) == 0) {
        if(first){
          first = false;

        }

        if(status = system( *(tokens + i) ) < 0){
          fprintf(stderr, "\nError execing. ERROR#%d:%d\n", errno, strerror_r());
          return EXIT_FAILURE;
        }
      
      
      
      
        exit(0);
      }
      if ((waitpid(pid, &status, 0)) == -1) {
        fprintf(stderr, "Process %d encountered an error. ERROR%d", i, errno);
        return EXIT_FAILURE;
      }



      
      free(*(tokens + i));
    }
    printf("\n");
    free(tokens);
  }


}


/**
 * Quash entry point
 *
 * @param argc argument count from the command line
 * @param argv argument vector from the command line
 * @return program exit status
 */ int main(int argc, char** argv) {
  command_t cmd; //< Command holder argument
  
  start();
  
  puts("Welcome to Quash!");
  puts("Type \"exit\" to quit");
  // Main execution loop
  while (is_running() && get_command(&cmd, stdin)) {
    // NOTE: I would not recommend keeping anything inside the body of
    // this while loop. It is just an example.
    // The commands should be parsed, then executed.
    if (!strcmp(cmd.cmdstr, "exit"))
      terminate(); // Exit Quash
    else{
     exec_command(cmd.cmdstr); 
      
      
      
      //puts(cmd.cmdstr); // Echo the input string
    }
  }
  return EXIT_SUCCESS;
}
