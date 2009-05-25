#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
int main() {
  int fd = open("/tmp/foo",O_WRONLY | O_CREAT, 0777);
  if(fd == -1) return 22;
  const char * x="hello world\n";
  write(fd,x,strlen(x));
  close(fd);
  return 0;
}
