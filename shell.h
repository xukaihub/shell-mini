#ifndef _SHELL_H_
#define _SHELL_H_

typedef int (*CMAIN)(int argc, char *argv[]);

int cmd_register(const char *name, CMAIN main);

void cmd_init(void);

void cmd_damon(char ch);
#endif
