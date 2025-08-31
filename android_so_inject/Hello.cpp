#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>

int main(int argc, char** argv)
{
    printf("hello ptrace1 \n");
    getchar();
    printf("hello ptrace2 \n");
    getchar();
    printf("hello ptrace3 \n");
    return 0;
}
