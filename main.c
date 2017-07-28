#include <stdio.h>
#include "shell.h"

int fuck_main(int argc, char *argv[])
{
    int i;
    printf("haha\n");
    for (i = 0; i < argc; i++)
        printf("%s\n", argv[i]);
    return 0;
}
char fu[] = "fuck -t -s";

int main(void)
{
    char ch;
    int i, cnt;

    cmd_init();
    cmd_register("fuck", fuck_main);

    cnt = strlen(fu);
    for (i = 0; i < cnt; i++)
        cmd_damon(fu[i]);
    cmd_damon('\n');
    return 0;
}
