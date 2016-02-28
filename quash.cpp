#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <queue>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <cstring>

class Command {
public:

  std::string commandName;
  int argc;
  bool background;
  char* env[6] = {
    getenv("HOME"),
    getenv("PATH"),
    getenv("TZ"),
    getenv("USER"),
    getenv("LOGNAME"),
    0
  };

  char *argv[64];

  Command(std::string input){
    enum { kMaxArgs = 64 };
    argc = 0;

    char *p2 = strtok(input, ' ');
    while (p2 && argc < kMaxArgs-1){
      argv[argc++] = p2;
      p2 = strtok(0, " ");
    }
    argv[argc] = 0;



    commandName = input;
    background = false;
    char* tempName = const_cast<char*>(input.c_str());
  }

};

int exec_command(Command command){
  pid_t pid;
  int status;
  int i;

  if ((pid=fork()) == 0) {
    //execlp(command.argv[0], &command.argv[0]);
    execve(argv[0], &argv[0], envp);
    fprintf(stderr, "ERROR\n");
    exit(0);
  }

  if ((waitpid(pid, &status, 0)) == -1) {
    fprintf(stderr, "Process %d encountered an error. ERROR%d", i, errno);
    return EXIT_FAILURE;
  }

}

int main(){
  std::string userinput = "";

  while(getline(std::cin, userinput) && userinput != "exit"){

    Command toexecute = Command(userinput);
    exec_command(toexecute);

  }
  return 0;
}

