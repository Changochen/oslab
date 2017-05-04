#ifndef _COMMON_H
#define _COMMON_H

#include "inc/types.h"
#include "inc/x86.h"

void __attribute__((__noinline__))  printf(const char *ctl, ...);
void __attribute__((__noinline__))  printk(const char *ctl, ...);
#endif
