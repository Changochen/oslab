// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <lib/common.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/x86.h>

#include <kernel/console.h>
#include <kernel/monitor.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line

extern void ls();
extern void start_game();
extern void cat(char* filename);
extern void mv(char*,char*);
extern void cp(char*,char*);
extern void rm(char*);
struct Command {
    const char *name;
    const char *desc;
    // return -1 to force monitor to exit
    int (*func)(int argc, char** argv, struct Trapframe* tf);
};


int mon_ls(int argc, char **argv, struct Trapframe *tf);

int mon_game(int argc, char **argv, struct Trapframe *tf){
    start_game();
    return 0;
}

int mon_echo(int argc, char **argv, struct Trapframe *tf){
    for(int i=1;i<argc;i++){
        printf("%s ",argv[i]);
    }
    printf("\n");
    return 0;
}


int mon_mv(int argc,char **argv,struct Trapframe *tf){
    if(argc!=3){
        printf("Usage :mv srcfile destfile\n");
        return -1;
    }
    mv(argv[1],argv[2]);
    return 0;
}

int mon_cp(int argc,char **argv,struct Trapframe *tf){
    if(argc!=3){
        printf("Usage :cp srcfile destfile\n");
        return -1;
    }
    cp(argv[1],argv[2]);
    return 0;
}

int mon_cat(int argc, char **argv, struct Trapframe *tf){
    for(int i=1;i<argc;i++){
        cat(argv[i]);
    }
    return 0;
}

int mon_rm(int argc, char **argv, struct Trapframe *tf){
    if(argc!=2){
        printf("Usage:rm filename\n");
        return -1;
    }
    rm(argv[1]);
    return 0;
}
static struct Command commands[] = {
    { "help", "Display this list of commands", mon_help },
    { "ls"  , "List all the file in the root directory", mon_ls},
    { "echo", "echo what you input", mon_echo },
    { "cat",  "Show the content of file",mon_cat},
    { "mv" ,  "Rename file", mon_mv},
    { "rm" ,  "Delete file", mon_rm},
    { "cp" ,  "Copy file", mon_cp},
    { "game", "Run the game" , mon_game}
};

/***** Implementations of basic kernel monitor commands *****/

    int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(commands); i++)
        printf("%s - %s\n", commands[i].name, commands[i].desc);
    return 0;
}


int mon_ls(int argc, char **argv, struct Trapframe *tf){
    ls();
    return 0;
}
/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

    static int
runcmd(char *buf, struct Trapframe *tf)
{
    int argc;
    char *argv[MAXARGS];
    int i;

    // Parse the command buffer into whitespace-separated arguments
    argc = 0;
    argv[argc] = 0;
    while (1) {
        // gobble whitespace
        while (*buf && strchr(WHITESPACE, *buf))
            *buf++ = 0;
        if (*buf == 0)
            break;

        // save and scan past next arg
        if (argc == MAXARGS-1) {
            printf("Too many arguments (max %d)\n", MAXARGS);
            return 0;
        }
        argv[argc++] = buf;
        while (*buf && !strchr(WHITESPACE, *buf))
            buf++;
    }
    argv[argc] = 0;

    // Lookup and invoke the command
    if (argc == 0)
        return 0;
    for (i = 0; i < ARRAY_SIZE(commands); i++) {
        if (strcmp(argv[0], commands[i].name) == 0)
            return commands[i].func(argc, argv, tf);
    }
    printf("Unknown command '%s'\n", argv[0]);
    return 0;
}

void
monitor(struct Trapframe *tf)
{
    char *buf;

    printf(" _   _       _           ____  _          _ _ \n");
    printf("| \\ | | __ _(_)_   _____/ ___|| |__   ___| | |\n");
    printf("|  \\| |/ _` | \\ \\ / / _ \\___ \\| '_ \\ / _ \\ | |\n");
    printf("| |\\  | (_| | |\\ V /  __/___) | | | |  __/ | |\n");
    printf("|_| \\_|\\__,_|_| \\_/ \\___|____/|_| |_|\\___|_|_|\n");
    printf("Type 'help' for a list of commands.\n");


    while (1) {
        buf = readline("$> ");
        if (buf != NULL)
            if (runcmd(buf, tf) < 0)
                break;
    }
}
