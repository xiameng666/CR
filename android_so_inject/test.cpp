#include <stdio.h>
__attribute__((constructor)) void init()
{
    printf("兄嘚，我注入了!!!\n");
}