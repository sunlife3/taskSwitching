 
#include "task.h"

extern void q_ChangePriority(int id,int pri);

void task1(int cnt){
    q_ChangePriority(0,0);
    printf("1 execution : %d\r\n",cnt);
}
void task2(int cnt){
    q_ChangePriority(1,0);
    printf("2 execution : %d\r\n",cnt);
}
void task3(int cnt){
    q_ChangePriority(2,0);
    printf("3 execution : %d\r\n",cnt);
}
void task4(int cnt){
    q_ChangePriority(3,0);
    printf("4 execution : %d\r\n",cnt);
}
