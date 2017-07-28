#include <stddef.h>
#include <string.h>
#include "shell.h"

#define CMD_LEN_MAX 20
#define CMD_MAX 30
#define ARGV_LEN_MAX 6
#define ARGC_MAX 8
#define HIST_MAX 9

#define BACKSPACE_KEY 0x08
#define ENTER_KEY 0x0d
#define ESC_KEY 0x1b
#define UP_KEY 'A'
#define DOWN_KEY 'B'
#define BACK "\b \b"
#define PROMPT "$ "

#define MIN(a, b) (((a)<(b))?(a):(b))

static int cmdlen;
static int hist_cnt;
static int hist_pos;
static int escape;
static int arrow;

struct cmap {
    char name[ARGV_LEN_MAX];
    CMAIN cmain;
};

static char hist[HIST_MAX][CMD_LEN_MAX];
static char line[CMD_LEN_MAX];
static struct cmap cmd_map[CMD_MAX];
static char argv_buf[ARGC_MAX][ARGV_LEN_MAX];

static int cmd_poll(void)
{
    return 1;
}

static int cmd_getc(void)
{
    return 'a';
}

static int cmd_putc(char ch)
{
    putchar(ch);
    return ch;
}

static int cmd_puts(char *s)
{
    int len = 0;
    char ch;
    
    while (ch = *s++) {
        len++;
        cmd_putc(ch);
    }
    return len;
}

static int cmd_split(char *str, int *argc, char *argv[])
{
    char *p;
    int len = 0;
    int cnt = 0;

    if (!str || !argc || !argv)
        return -1;

    p = strtok(str, " ");
    while (p && cnt < ARGC_MAX) {
        len = MIN(strlen(p), ARGV_LEN_MAX);
        strncpy(argv[cnt], p, len);
        argv[cnt][ARGV_LEN_MAX - 1] = '\0';
        p = strtok(NULL, " ");
        cnt++;
    }
    *argc = cnt;
    return 0;
}

static int cmd_run(const char *cmd)
{
    int i, len, argc;
    char *p;
    char *argv[ARGC_MAX];
    char str[CMD_LEN_MAX] = {0};

    len = MIN(strlen(cmd), CMD_LEN_MAX);
    strncpy(str, cmd, len);
    str[CMD_LEN_MAX - 1] = '\0';

    memset(argv_buf, 0, ARGC_MAX * ARGV_LEN_MAX);
    for (i = 0; i < ARGC_MAX; i++)
        argv[i] = argv_buf[i];
    cmd_split(str, &argc, argv);

    for (i = 0; i < CMD_MAX; i++) {
        if (!strncmp(argv[0], cmd_map[i].name, ARGV_LEN_MAX) && 
                cmd_map[i].cmain)
            return cmd_map[i].cmain(argc, argv);
    }
    return -1;
}

static int add_to_hist(char *cmd)
{
    int len;

    if (!strncmp(cmd, hist[(hist_cnt - 1) % HIST_MAX], CMD_LEN_MAX)) {
        hist_pos = hist_cnt;
        return 0;
    }
    memset(hist[hist_cnt % HIST_MAX], 0, CMD_LEN_MAX);
    len = MIN(strlen(cmd), CMD_LEN_MAX);
    hist_cnt++;
    hist_pos = hist_cnt;

    memset(hist[hist_cnt % HIST_MAX], 0, CMD_LEN_MAX);
    return 0;
}

static char *get_up_hist(void)
{
    char *str;
    int step = hist_cnt - hist_pos;

    if (step < HIST_MAX) {
        hist_pos--;
        str = hist[hist_pos % HIST_MAX];
        if (str[0] == '\0') {
            hist_pos++;
            return NULL;
        }
        return str;
    }
    return NULL;
}

static char *get_down_hist()
{
    if (hist_pos < hist_cnt)
        return hist[++hist_pos % HIST_MAX];
    return NULL;
}

static void clean_line(void)
{
    while (cmdlen--)
        cmd_puts(BACK);
}

static void do_arrow(char ch)
{
    char *str;

    if (UP_KEY == ch) {
        if (str = get_up_hist()) {
            clean_line();
            cmdlen = cmd_puts(strncpy(line, str, CMD_LEN_MAX));
        }
    } else if(DOWN_KEY == ch) {
        if (str = get_down_hist()) {
            clean_line();
            cmdlen = cmd_puts(strncpy(line, str, CMD_LEN_MAX));
        }
    }
    arrow = escape = 0;
}

static void do_enter(void)
{
    if (cmdlen) {
        cmd_putc('\n');
        line[cmdlen] = '\0';
        cmd_run(line);
        add_to_hist(line);
        memset(line, 0, CMD_LEN_MAX);
        cmdlen = 0;
        cmd_puts(PROMPT);
    } else {
        cmd_putc('\n');
        cmd_puts(PROMPT);
    }
    arrow = escape = 0;
}

static void do_backspace(void)
{
    if (cmdlen) {
        line[--cmdlen] = '\0';
        cmd_puts(BACK);
    }
    arrow = escape = 0;
}

static void do_putc(char ch)
{
    cmd_putc(ch);
    if (cmdlen < CMD_LEN_MAX -1)
        line[cmdlen++] = ch;
}

void cmd_damon(char ch)
{
    // char ch;
    char *str;

    //if (!cmd_poll())
    //{
    //    return;
    //}
    //ch = cmd_getc();

    if (ESC_KEY == ch)
        escape = !(arrow = 0);
    else if (escape && '[' == ch)
        arrow = !(escape = 0);
    else if (arrow)
        do_arrow(ch);
    else if (ENTER_KEY == ch)
        do_enter();
    else if (BACKSPACE_KEY == ch)
        do_backspace();
    else
        do_putc(ch);
}

int cmd_register(const char *name, CMAIN cmain)
{
    int i, len;

    if (!name || !cmain)
        return -1;

    for (i = 0; i < CMD_MAX; i++) {
        if (cmd_map[i].cmain)
            continue;
        len = MIN(strlen(name), ARGV_LEN_MAX);
        strncpy(cmd_map[i].name, name, len);
        cmd_map[i].name[ARGV_LEN_MAX - 1] = '\0';
        cmd_map[i].cmain = cmain;
        return 0;
    }
    return -1;
}

void cmd_init(void)
{
    int i;

    for (i = 0; i < CMD_MAX; i++) {
        memset(cmd_map[i].name, 0, ARGV_LEN_MAX);
        cmd_map[i].cmain = NULL;
    }

    for (i = 0; i < HIST_MAX; i++)
        memset(hist[i], 0, CMD_LEN_MAX);

    cmdlen = arrow = escape = 0;
    hist_cnt = hist_pos = HIST_MAX + 1;
}
