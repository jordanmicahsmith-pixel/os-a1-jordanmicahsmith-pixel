#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/uproc.h"
#include "user/user.h"

static char *states[] = {"unused", "used", "sleeping", "runnable", "running", "zombie"};
struct uproc procs[NPROC];     // global, so it doesn't eat the small user stack

int main(void) {
  int n = getprocs(procs, NPROC);
  if (n < 0) {
    fprintf(2, "ps: getprocs failed\n");
    exit(1);
  }
  printf("PID\tPPID\tSTATE\tSIZE\tNAME\n");
  for (int i = 0; i < n; i++)
    printf("%d\t%d\t%s\t%lu\t%s\n", procs[i].pid, procs[i].ppid,
           states[procs[i].state], procs[i].sz, procs[i].name);
  exit(0);
}