#include<inc/types.h>
#define NODENUM 1000
#define INF 0x909090
typedef struct Node
{
    int x;
    int y;
    struct Node *next,*prev;
}NODE;

NODE* get_head();
NODE* get_tail();
void init_list();

void add(int x,int y);

NODE* del(NODE* node);
