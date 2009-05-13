#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#define UNIX_ASSURE(X,msg) {if((X) == -1) {perror(msg "\n"); exit(1);}}

int main() {
  struct passwd * p = getpwnam("judge");
  uid_t judge_uid = p->pw_uid;
  int problemid = 0;
  int fds[2];
  pipe(fds);
  pid_t pid = fork();
  UNIX_ASSURE(pid, "fork");
  if(pid == 0) {
    system("whoami");
    //Chroot here
    UNIX_ASSURE( setreuid(judge_uid, judge_uid) , "setreuid");
    struct rlimit l = {40*1024*1024,40*1024*1024};
    UNIX_ASSURE( setrlimit(RLIMIT_AS,&l) , "setrlimit");
    printf("==================> COMPILATION <===================\n");
    int i=system("g++ -O2 -Wall test.cc -o target/a.out");
    printf("%d\n",i);
    perror("x");
  } else {
    int i;
    wait(&i);
  }
  

  //Drop privaleges
  //Compile source
  //Run program for all test inputs
  //Write output back
  
  //
  //
}
