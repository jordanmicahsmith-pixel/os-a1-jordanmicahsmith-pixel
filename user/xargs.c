#include "kernel/types.h"
#include "kernel/param.h"     // gives MAXARG
#include "user/user.h"

#define BUFSZ 2048

char buf[BUFSZ];      // storage for the words we read
char *args[MAXARG];   // the argument list we hand to exec
int nargs, base, bufpos, failed;

void usage(void) {
  fprintf(2, "usage: xargs [-n N] cmd [args...]\n");
  exit(1);
}

void toomany(void) {
  fprintf(2, "xargs: too many arguments\n");
  exit(1);
}

void run(void) {
  int st;
  args[nargs] = 0;                 // exec needs a 0 at the end of the list
  int pid = fork();
  if (pid < 0) {
    fprintf(2, "xargs: fork failed\n");
    exit(1);
  }
  if (pid == 0) {
    exec(args[0], args);
    fprintf(2, "xargs: exec %s failed\n", args[0]);
    exit(1);
  }
  wait(&st);
  if (st != 0)
    failed = 1;
  nargs = base;                    // throw away this batch's words, keep the cmd
  bufpos = 0;
}

int main(int argc, char *argv[]) {
  int n = 0, start = 1;

  if (argc > 1 && strcmp(argv[1], "-n") == 0) {
    if (argc < 4) usage();
    n = atoi(argv[2]);
    if (n < 1) usage();
    start = 3;
  }
  if (start >= argc) usage();      // no command given

  for (int i = start; i < argc; i++) {
    if (nargs >= MAXARG - 1) toomany();
    args[nargs++] = argv[i];
  }
  base = nargs;                    // cmd + its fixed args

  char ch, *w = 0;
  int inword = 0;
  while (read(0, &ch, 1) == 1) {
    if (ch == ' ' || ch == '\t' || ch == '\n') {
      if (inword) {                // a word just ended: save it
        buf[bufpos++] = 0;
        args[nargs++] = w;
        inword = 0;
        if (n > 0 && nargs - base == n)
          run();
      }
      if (ch == '\n' && n == 0 && nargs > base)
        run();                     // line mode: one run per non-empty line
    } else {
      if (!inword) {               // a new word starts
        if (nargs >= MAXARG - 1) toomany();
        w = &buf[bufpos];
        inword = 1;
      }
      if (bufpos >= BUFSZ - 2) toomany();
      buf[bufpos++] = ch;
    }
  }
  if (inword) {                    // input ended mid-word, no trailing newline
    buf[bufpos++] = 0;
    args[nargs++] = w;
  }
  if (nargs > base)
    run();                         // leftover words (the "last run may be shorter")
  exit(failed ? 1 : 0);
}