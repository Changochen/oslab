#include<lib/blocklist.h>
NODE blocks[NODENUM];
NODE* head,*tail;
void init_list(){
    for(int i=0;i<NODENUM;i++){
        blocks[i].y=INF;
    }
    head=&blocks[0];
    blocks[0].x=0;
    blocks[0].y=150;
    blocks[0].next=tail;
    tail=blocks[0].next;
    tail->prev=&blocks[0];
}

NODE* get_head(){
    return head;
}
NODE* get_tail(){
    return tail;
}
void add(int x,int y){
    for(int i=0;i<NODENUM;i++){
        if(blocks[i].y==INF){
            blocks[i].x=x;
            blocks[i].y=y;
            tail->prev->next=&blocks[i];
            blocks[i].prev=tail->prev;
            tail->prev=&blocks[i];
            blocks[i].next=tail;
            return;
        }
    }
}

NODE* del(NODE* node){
    if(head->x==node->x&&head->y==node->y){
        head=head->next;
    }
    NODE* next=node->next;
    node->prev->next=node->next;
    node->next->prev=node->prev;
    node->y=INF;
    return next;
}
