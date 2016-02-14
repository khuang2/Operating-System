/***************************************************************************
  process.h
	
	This file includes the data structures and functions for the process

***************************************************************************/

#ifndef  PROCESS_H
#define  PROCESS_H

#define READY_QUEUE_LOCK 0x7FE0000A;
#define TIMER_QUEUE_LOCK 0x7FE00010;

#define         PTBL_DISK_RESIDENT_BIT                  0x1000

#define                  DO_LOCK                     1
#define                  DO_UNLOCK                   0
#define                  SUSPEND_UNTIL_LOCKED        TRUE
#define                  DO_NOT_SUSPEND              FALSE

#define         LEGAL_MESSAGE_LENGTH           (INT16)64


typedef struct{
	long DiskID;
	long Sector;
}ShadowedPage;

typedef struct{
	INT32 processID;
	INT32 Page;
	INT32 time;//for the use of LRU
}ShadowedFrame;

typedef struct {
	long time;              //use time as a key on timer queue
	long delay;             //sleep time
	char *processName;   
	long contextPointer;   //pointer to the context
	INT32 priority;
	INT32 pid; 
	INT32 terminated;
	INT32 suspend;	
}PCB; //Process Control Block

UINT16 PageTable[15][1024];//max number of process is 15

ShadowedPage ShadowedTable[15][1024]; //store the disk information of pages.

ShadowedFrame  FrameTable[64]; //store the information of page that resident on frames.

struct PCBNode{
	PCB *pcb;
	struct PCBNode *next;
};


typedef struct {

	long TargetPid;
	long SourcePid;
	long ActualSourcePid;
	long SendLength;
	long ReceiveLength;
	long ActualSendLength;
	long send_loop_count;
	long receive_loop_count;
	char MessageBuffer[LEGAL_MESSAGE_LENGTH ];
	char MessageSent[LEGAL_MESSAGE_LENGTH ];
}Message;

struct MessageNode{
	Message *message;
	struct MessageNode *next;
};

struct PCBNode *timerQHead;
struct PCBNode *readyQHead;
struct PCBNode *pcbQHead;
struct PCBNode *suspendQHead;
struct MessageNode *messageQHead;

/*8 disk queues*/
struct PCBNode *diskQHead1;
struct PCBNode *diskQHead2;
struct PCBNode *diskQHead3;
struct PCBNode *diskQHead4;
struct PCBNode *diskQHead5;
struct PCBNode *diskQHead6;
struct PCBNode *diskQHead7;
struct PCBNode *diskQHead8;


INT32 PID;   // new created PID
PCB *deReadyPCB; //it's the pcb dequed from ready queue- current pcb
INT32 pcbNum;
INT32 msgNum;


INT32 readyNumber;
INT32 timerNumber;
INT32 suspendNumber;

UINT16 Bits;//to indicate the current frame
short BitsUtil;
INT16 replacementFlag;
int flag;  //flag to indicate whether page replacement starts
int firstFault; //to indicate if it's the first time of page fault
int DiskItr;//Disk ID iterator
int SectorItr;//Sector iterator

int sharedID;//shared memory ID

/***************************************************************************
 Timer Queue, which is implemented by linked list.
***************************************************************************/
void enTimerQueue(struct PCBNode **head_ref, PCB *pcb); //function to add a node with the given PCB to the queue.

PCB *deTimerQueue(struct PCBNode **head_ref); //function to delete the front node from the queue.

void printTimerQueue(struct PCBNode **head_ref);
/***************************************************************************
Ready Queue
***************************************************************************/
void enReadyQueue(struct PCBNode **head_ref, PCB *pcb); //function to add a node with the given PCB to the queue.

PCB *deReadyQueue(struct PCBNode **head_ref); //function to delete the front node from the queue.

void printReadyQueue(struct PCBNode **head_ref);
/***************************************************************************
PCB Queue
***************************************************************************/
void enPCBQueue(struct PCBNode **head_ref, PCB *pcb);

/***************************************************************************
Suspend Queue
***************************************************************************/
void enSuspendQueue(struct PCBNode **head_ref, PCB *pcb); //function to add a node with the given PCB to the queue.

PCB *deSuspendQueue(struct PCBNode **head_ref); //function to delete the front node from the queue.

/***************************************************************************
Message Queue
***************************************************************************/
void enMessageQueue(struct MessageNode **head_ref, Message *message);

Message *deMessageQueue(struct MessageNode **head_ref);

void printMsgQueue(struct MessageNode **head_ref);

/***************************************************************************
Disk Queue
***************************************************************************/
void enDiskQueue(struct PCBNode **head_ref, PCB *pcb);

PCB *deDiskQueue(struct PCBNode **head_ref);

void printDiskQueue(struct PCBNode **head_ref);

/***************************************************************************
OS_Create_Process
***************************************************************************/
void OSCreateProcess(long *arg0,long *arg1,long *arg2,long *arg3,long *arg4);

/***************************************************************************
getProcessID
***************************************************************************/
void getProcessID(long *arg0,long *arg1,long *arg2);

/***************************************************************************
Dispatcher
***************************************************************************/
void Dispatcher();

/***************************************************************************
Terminate Process
***************************************************************************/
void terminateSysProcess(long *arg0,long *arg1);

/***************************************************************************
Start Timer
***************************************************************************/
void startTimer(long *arg0);

/***************************************************************************
suspendProcess
***************************************************************************/
void suspendProcess(long *arg0, long *arg1);

/***************************************************************************
resumeProcess
***************************************************************************/
void resumeProcess(long *arg0, long *arg1);

/***************************************************************************
changePriority
***************************************************************************/
void changePriority(long *arg0, long *arg1, long *arg2);

void Loop();

/***************************************************************************
statePrinter
***************************************************************************/
void statePrinter(short call_type,long* arg0, long *arg1, long *arg2, long *arg3, long *arg4);

/***************************************************************************
Send Message
***************************************************************************/
void Send_message(long* arg0, long *arg1, long *arg2, long *arg3);

/***************************************************************************
Receive Message
***************************************************************************/
void ReceiveMessage(long* arg0, long *arg1, long *arg2, long *arg3, long *arg4, long *arg5);

/***************************************************************************
Disk Write
***************************************************************************/
void DiskWrite(long* arg0, long *arg1, long *arg2);

/***************************************************************************
Disk Read
***************************************************************************/
void DiskRead(long* arg0, long *arg1, long *arg2);

/***************************************************************************
Page Replacement
***************************************************************************/
void PageReplaceOut(UINT16 position,int processID,int Page);

void PageReplaceIn(UINT16 position,int Status);

INT32 LeastRecentlyUsed();

/***************************************************************************
Memory Printer
***************************************************************************/
void MemoryPrinter();

/***************************************************************************
Define Shared Area
***************************************************************************/
void DefineSharedArea(long* arg0,long* arg1,long* arg2,long* arg3,long* arg4);
#endif

