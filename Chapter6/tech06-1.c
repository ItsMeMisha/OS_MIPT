//SOMETHING WRONG --- WA IN TESTS
/* Программе на стандартном потоке ввода задается выражение в синтаксисе языка Си.

Необходимо вычислить значение этого выражения (итоговый результат представим типом int) и вывести его на стандартный поток вывода.
*/


#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

const char ProgramBegin[] = "#include <stdio.h>\n"
                            "int main () {\n"
                            "   printf(\"%d\\n\", ";
const char ProgramEnd[] = ");\n"
                          "   return 0;\n"
                          "}";

const char tempSrcFileName[] = "tmpcprog.c";
const char tempExecFileName[] = "tmpcprog.out";
const char ExecCommand[] = "./tmpcprog.out";

enum Error {
    ALLRIGHT = 0,
    BADFORK = 1,
    BADEXEC = 2,
    FILEERROR = 3,
    READERROR = 4
};

int main()
{
    char arg[ARG_MAX];
    char* result = fgets(arg, ARG_MAX, stdin);
    if (result == NULL)
        return READERROR;

    FILE* progSrc = fopen(tempSrcFileName, "w");
    if (progSrc == NULL)
        return FILEERROR;
    fprintf(progSrc, "%s%s%s", ProgramBegin, arg, ProgramEnd);
    fclose(progSrc);

    pid_t pid = fork();
    if (pid == 0) {
        int error = execlp(
            "gcc",
            "gcc",
            "-std=c11",
            "-o",
            tempExecFileName,
            tempSrcFileName,
            NULL);
        if (error == -1)
            exit(BADEXEC);

    } else if (pid == -1) {
        unlink(tempSrcFileName);
        exit(BADFORK);
    }

    int status = 0;
    waitpid(pid, &status, 0);
    unlink(tempSrcFileName);

    pid = fork();
    if (pid == 0) {
        int error = execlp(ExecCommand, tempExecFileName, NULL);
        if (error == -1)
            exit(BADEXEC);

    } else if (pid == -1) {
        unlink(tempExecFileName);
        exit(BADFORK);
    }

    waitpid(pid, &status, 0);
    unlink(tempExecFileName);

    return 0;
}
