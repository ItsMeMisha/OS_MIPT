#include <stdio.h>

extern void normalize_path(char* path);

int main () {

char path1[] = "./asdf/.././dsa/ewq/../ser";
char path2[] = "abrakadabra///abc";
char path3[] = "/var/log/../lib/./ejexec";

normalize_path(path1);
printf ("%s\n", path1);

normalize_path(path2);
printf ("%s\n", path2);

normalize_path(path3);
printf ("%s\n", path3);

}
