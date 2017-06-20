#include<lib/common.h>
#include<kernel/console.h>
#include<lib/keyboard.h>
#include<lib/serial.h>
#define BUFLEN 1024
static char buf[BUFLEN];

void putc(char s){
	serial_printc(s);
}
char *
readline(const char *prompt)
{
	int i, c, echoing;

	if (prompt != NULL)
		printf("%s", prompt);

	i = 0;
	echoing = 1;
	while (1) {
		c = getc();
		if (c < 0) {
			printf("read error: %e\n", c);
			return NULL;
		} else if ((c == '\b' || c == '\x7f') && i > 0) {
			if (echoing)
			    putc('\b');
			i--;
		} else if (c >= ' ' && i < BUFLEN-1) {
			if (echoing)
				putc(c);
			buf[i++] = c;
		} else if (c == '\n' || c == '\r') {
			if (echoing)
				putc('\n');
			buf[i] = 0;
			return buf;
		}
	}
}

