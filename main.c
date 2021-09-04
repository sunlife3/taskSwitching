#include "stdio.h"
#include "emu.h"
#include "task.h"

// number of task 
#define TASK_NUM 4
// number of task priority 
#define TASK_PRI_NUM 4
// task stack size
#define TASK_STACK_SIZE 64

struct x86_64REGISTERS{
    //Refer to x86_64 Architecture
    unsigned long long rax,rbx,rcx,rdx,rsi,rdi,rbp,rsp; 
};

struct ARMREGISTERS{
    //Refer to ARM Architecture
    unsigned long long r[13];
    unsigned long long sp;
    unsigned long long lr;
    unsigned long long pc; 
};

struct REGISTERS reg;

unsigned int taskStack[TASK_NUM][TASK_STACK_SIZE];

extern void task1(int cnt);
extern void task2(int cnt);
extern void task3(int cnt);
extern void task4(int cnt);


typedef struct {
  FP  ptrTask;
  int priority;
} taskinfo_t;

taskinfo_t tasks[TASK_NUM] = {
    {task1, 1},
    {task2, 2},
    {task3, 2},
    {task4, 3},
};

#define WAIT  0
#define READY 1
#define RUN   2
#define DORMANT 3

typedef struct TCB {
    unsigned int *ptr_sp;
    struct TCB *ptr_prev;
    struct TCB *ptr_next;
    int id;
    int priority;
    int state;
} tcb_t;

int runningID;

// Task controll block
tcb_t tcb[TASK_NUM];

// Lead of ready queue
// have ONLY ptr_next (NOT have task information)
// priority 3  queue_ready[3] *ptr_next -> task4
// priority 2  queue_ready[2] *ptr_next -> task2 -> task3 
// priority 1  queue_ready[1] *ptr_next -> task1 
tcb_t queue_ready[TASK_PRI_NUM];

static void q_Add(tcb_t *leadReadyQueue, int taskID){
    tcb_t *taskptr = &tcb[taskID];
    tcb_t *readyQueuePtr = leadReadyQueue;//priority specified
    while(readyQueuePtr->ptr_next != 0){
        //search last TCB of the queue
        readyQueuePtr = readyQueuePtr->ptr_next;
    }

    //add task to last of que 
    readyQueuePtr -> ptr_next = taskptr;
    taskptr -> ptr_prev = readyQueuePtr;
    taskptr -> ptr_next = 0;
    //printf("%d,%d",taskptr->id, taskptr->priority);
}

int q_GetTopPriority(){
    tcb_t *readyQueuePtr = queue_ready;
    for(int pri=TASK_PRI_NUM ; 0<pri; pri--){
        if(readyQueuePtr[pri].ptr_next != 0){
            return readyQueuePtr[pri].ptr_next -> priority;
        }
    }
    return 0;//priority 0
}

void q_Remove(int ID){
    tcb_t *task = &tcb[ID];

    task->ptr_prev->ptr_next = task->ptr_next;
    task->ptr_next->ptr_prev = task->ptr_prev;
    task->ptr_prev = 0;
    task->ptr_next = 0; 
}

void q_Delete(int ID){
    tcb_t *task = &tcb[ID];

    task->ptr_prev->ptr_next = task->ptr_next;
    task->ptr_next->ptr_prev = task->ptr_prev;
    task->ptr_prev = 0;
    task->ptr_next = 0; 
    task->state = DORMANT;
}

void q_ChangePriority(int ID, int pri){
    //ID:taskID of priority escalated task
    //pri: task proprity after change
    tcb_t *task = &tcb[ID];
    task->priority = pri;
    q_Remove(ID);
    q_Add(&queue_ready[task->priority], ID);
}

static void ReadyQueueAdder(int taskID){
    tcb_t *taskptr = &tcb[taskID];
    q_Add(&queue_ready[taskptr->priority], taskID);
}

static void allTaskInit(){
    char* state;
    printf("******* TASK State *******\n");
    printf("id| Priority |  SP | State\r\n");
    for(int id=0; id < TASK_NUM; id++){
        tcb[id].id = id;
        tcb[id].priority = tasks[id].priority;
        tcb[id].ptr_sp = &taskStack[id][TASK_STACK_SIZE-16];
        tcb[id].state = READY;
        switch(tcb[id].state){
            case 0:
                state = "wait";
                break;
            case 1:
                state = "ready";
                break;
            case 2:
                state = "run";
                break;
            case 3:
                state = "dormant";
                break;
            default:
                state = "error";
        }
        ReadyQueueAdder(tcb[id].id);
        printf("%d |     %d    |  %d  | %s\r\n", tcb[id].id, tcb[id].priority, *tcb[id].ptr_sp,state);
    }
    printf("\n********** READY QUE *************\n");

    for(int pri=0;pri < TASK_PRI_NUM; pri++){
        printf("Priority:%d  ",pri);
        tcb_t *ptr = &queue_ready[pri];
        printf("(READY QUE ADDRESS:%p)\n",&queue_ready[pri]);
        while(ptr->ptr_next != 0){
            ptr = ptr->ptr_next;
            printf("  ID:%d, ptr_prev:%p -> ptr_next:%p \n",
            ptr->id, ptr->ptr_prev,ptr);
        }
      printf("\n\n");
    }
    printf("Task information initiarized\r\n");
}

static void RTOSKernel(){//カーネルと名付けているがこれは嘘だ
    int pri;
    pri = q_GetTopPriority();
    printf("Start from Priority%d\n",pri);
    tcb_t *RunningTask = queue_ready[pri].ptr_next;
    RunningTask->state = RUN;
    printf("NEXT task ... ID:%d state:%d TCBaddr:%p\n",RunningTask->id,RunningTask->state,RunningTask);
    for(int cnt=0; cnt<5; cnt++){
        if(RunningTask->ptr_next == 0){
            pri = pri - 1;
            RunningTask->state = READY;
            RunningTask = queue_ready[pri].ptr_next;
            RunningTask->state = RUN;
            printf("NEXT...ID:%d state:%d TCBaddr:%p\n",RunningTask->id,RunningTask->state,RunningTask);
        }else{
            RunningTask->state = READY;
            RunningTask = RunningTask->ptr_next;
            RunningTask->state = RUN;
            printf("NEXT...ID:%d state:%d TCBaddr:%p\n",RunningTask->id,RunningTask->state,RunningTask);
        }
    }
}

int main(){
    printf("RTOS emulator\n");
    allTaskInit();
    RTOSKernel();
}