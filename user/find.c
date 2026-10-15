#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char typefilter = 0;

int match(char *p, char *s)
{
    if (*p == 0)                 // pattern used up:
        return *s == 0;            //   match only if name is also used up
    
    if (*p == '*')
    {
        if (match(p + 1, s))       // let * match nothing
            return 1;
        if (*s != 0)               // or let * swallow one character and try again
            return match(p, s + 1);
        return 0;
    }

    if (*s == 0 || *p != *s)     // ordinary character must match exactly
        return 0;
    return match(p + 1, s + 1);
}

int find(char *dir, char *pat)
{
    char buf[256], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(dir, 0)) < 0)
        return -1; // cannot open directory
    if (fstat(fd, &st) < 0 || st.type != T_DIR)
    {  // not a directory
        close(fd);
        return -1;
    }
    if (strlen(dir) + 1 + DIRSIZ + 1 > sizeof(buf))
    {   // path too long
        close(fd);
        return 0;
    }

    strcpy(buf, dir);
    p = buf + strlen(buf);
    *p++ = '/';                     // buf is now "dir/", p points just after the slash

    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if (de.inum == 0)
            continue;
        memmove(p, de.name, DIRSIZ);  // buf is now "dir/name"
        p[DIRSIZ] = 0;
        if (strcmp(p, ".") == 0 || strcmp(p, "..") == 0)
            continue;
        if (stat(buf, &st) < 0)
            continue;

        int hit = match(pat, p);      // match against the NAME only, not the whole path
        if (st.type == T_DIR) {
            if (hit && typefilter != 'f')
                printf("%s\n", buf);
            find(buf, pat);             // the recursion: search inside this subfolder
        } else {                      // T_FILE or T_DEVICE
            if (hit && typefilter != 'd')
                printf("%s\n", buf);
        }
    }
    close(fd);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3 && argc != 5)
    {
        fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
        exit(1);
    }
    if (argc == 5)
    {
        if (strcmp(argv[3], "-type") != 0 ||
            (strcmp(argv[4], "f") != 0 && strcmp(argv[4], "d") != 0))
            {
            fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
            exit(1);
            }
        typefilter = argv[4][0];
    }
    if (find(argv[1], argv[2]) < 0)
    {
    fprintf(2, "find: cannot open %s\n", argv[1]);
    exit(1);
    }

    exit(0);
}