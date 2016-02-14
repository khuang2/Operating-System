#include             "global.h"
#include             "syscalls.h"
#include             "protos.h"
#include			 "process.h"
#include             "string.h"
#include             <stdlib.h>
#ifdef NT
#include                 <windows.h>
#include                 <winbase.h>
#include                 <sys/types.h>
#endif

INT32 readyNumber=0;
INT32 timerNumber=0;
INT32 suspendNumber=0;
INT32 msgNum=0;
INT32 PID=0;
UINT16 Bits=0x0000; //to indicate the current frame
short BitsUtil=-1;
int flag=0; //to indicate whether page replacement happens
int firstFault=1; //to indicate if it's the first time of page fault
int DiskItr=0; //Disk ID iterator
int SectorItr=0; //Sector iterator

int sharedID=-1; //shared memory ID

/***************************************************************************
OS_Create_Process
***************************************************************************/
void OSCreateProcess(long *arg0,long *arg1,long *arg2,long *arg3,long *arg4){
	MEMORY_MAPPED_IO mmio;
	INT32 LockResult;
	INT32 UnlockResult;
	struct PCBNode *current;
	char *newPName;
	INT32 sameName=0;//if sameName of the process is created
	int i;
	int j;

	PCB *p_c_b=(PCB*)malloc(sizeof(PCB)); //I use it to create a new pcb

	/*initialization of shadowed page table, zero is an illegal disk ID*/
	for(i=0;i<15;i++){
		for(j=0;j<1024;j++)
			ShadowedTable[i][j].DiskID=0;
	}

	
	READ_MODIFY(0x7FE00001, DO_LOCK, SUSPEND_UNTIL_LOCKED,
		&LockResult);
	READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
		&LockResult);
	READ_MODIFY(0x7FE00003, DO_LOCK, SUSPEND_UNTIL_LOCKED,
		&LockResult);

	newPName=(char *)arg0;  //newly created pcb name
	current=pcbQHead; 
	if(current==NULL)
		sameName=0;
	else{
		
		while(current!=NULL&&strcmp(current->pcb->processName,newPName)!=0){
			current=current->next;
		}
		if(current!=NULL)
			sameName=1;
	}
	if((INT32)arg2<0||sameName==1||pcbNum==15){    //max number of process is 15
		*(arg4)=1;
	}else{
		
	/*initialization of page table*/
	for(i=0;i<15;i++){
		for(j=0;j<1024;j++)
			PageTable[i][j]=0x0000;
	}
		
	p_c_b->terminated=0;
	p_c_b->suspend=0;
	pcbNum++;	
	p_c_b->processName=(char *)calloc(sizeof(char),16);
	strcpy(p_c_b->processName,(char *)arg0);
	p_c_b->priority=(INT32)(arg2);
	p_c_b->pid=PID;
	*(arg3)=p_c_b->pid;
	*(arg4)=0;//ErrorReturned: no error
	PID++;

	mmio.Mode = Z502InitializeContext;
	mmio.Field1 = 0;
	mmio.Field2 = (long)arg1;
	mmio.Field3 = (long) PageTable[p_c_b->pid];
	MEM_WRITE(Z502Context, &mmio);   // Start this new Context Sequence

	p_c_b->contextPointer=(long)mmio.Field1;
	/*After setup the contex for the pcb, move it to both pcb queue for record and ready queue*/
	enPCBQueue(&pcbQHead,p_c_b);

	enReadyQueue(&readyQHead,p_c_b);

	READ_MODIFY(0x7FE00003, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
	READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
	&UnlockResult);
	READ_MODIFY(0x7FE00001, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
	}

}

/***************************************************************************
getProcessID
***************************************************************************/
void getProcessID(long *arg0,long *arg1,long *arg2){
	char *pName;
	struct PCBNode *current;
	pName=(char *)arg0;
	/*get current pid*/
	if(strcmp(pName,"")==0){
		*arg1=deReadyPCB->pid;
		*arg2=0;
	}else{
		/*traverse the pcb queue to find the pid*/
		current=pcbQHead;
		if(current==NULL)
			*arg2=1;
		else{
			while(current!=NULL&&strcmp(current->pcb->processName,pName)!=0){
				current=current->next;
			}
			if(current!=NULL&&current->pcb->terminated==0){
				*arg1=current->pcb->pid;
				*arg2=0;
			}else{
				*arg2=1;
			}
		}
	}
}



/***************************************************************************
Dispatcher£º deque pcb from ready queue, and wait until there is at least 
one pcb on ready queue. If the current PCB received is suspended, then put
the PCB into suspend queue, and call Dispatcher again. If the current PCB
received is terminated, then call Dispatcher again.
***************************************************************************/
void Dispatcher(){
	MEMORY_MAPPED_IO mmio;
	long processAddress;
	INT32 LockResult=0;
	INT32 UnlockResult=0;

	/* if nothing on readyQueue, wait for interrupt, and then loop until there is pcb on ready queue*/
	while(readyQHead==NULL){
		CALL(1);
	}
	READ_MODIFY(0x7FE00001, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	READ_MODIFY(0x7FE00003, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);

	deReadyPCB=deReadyQueue(&readyQHead);

	if(deReadyPCB->terminated==1){
		printf("pid %d has been terminated\n",deReadyPCB->pid);
		READ_MODIFY(0x7FE00003, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&UnlockResult);
		READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
		READ_MODIFY(0x7FE00001, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&UnlockResult);
		Dispatcher();
	}
	else if(deReadyPCB->suspend==1){
		enSuspendQueue(&suspendQHead,deReadyPCB);
		READ_MODIFY(0x7FE00003, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&UnlockResult);
		READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
		READ_MODIFY(0x7FE00001, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&UnlockResult);
		Dispatcher();
	}
	else{
		/*here, we need to unlock it twice since last time when
		we call Dispatcher and switch to a new context, we haven't
		unlock yet.Otherwise, there can exist bug sometime*/
		READ_MODIFY(0x7FE00003, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&UnlockResult);
		READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
		READ_MODIFY(0x7FE00001, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&UnlockResult);
		READ_MODIFY(0x7FE00003, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&UnlockResult);
		READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
		READ_MODIFY(0x7FE00001, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&UnlockResult);

		processAddress=(long)(deReadyPCB->contextPointer);
		mmio.Mode = Z502StartContext;
		mmio.Field1=processAddress;
		// Field1 contains the value of the context returned in the last call
		// Suspends this current thread
		mmio.Field2 = START_NEW_CONTEXT_AND_SUSPEND;
		MEM_WRITE(Z502Context, &mmio);     // switch context


	}

}

/***************************************************************************
TerminateProcess: 
***************************************************************************/
void terminateSysProcess(long *arg0,long *arg1){
	struct PCBNode *tempCurrent;
	INT32 LockResult=0;
	INT32 UnlockResult=0;

	READ_MODIFY(0x7FE00001, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	READ_MODIFY(0x7FE00003, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	if((INT32) arg0==-2||((INT32)arg0==-1&&readyQHead==NULL)){    //terminate all the processes
		*arg1=0;
		MEM_WRITE(Z502Halt, 0);
	}else if((INT32) arg0==-1){  //terminate the current process
		deReadyPCB->terminated=1;
		READ_MODIFY(0x7FE00003, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&UnlockResult);
		READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
		READ_MODIFY(0x7FE00001, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&UnlockResult);
		Dispatcher();
	}else{
		/*traverse ready queue to find the pid to be terminated*/
		tempCurrent=readyQHead;
		while(tempCurrent!=NULL&&tempCurrent->pcb->pid!=(INT32) arg0){
			tempCurrent=tempCurrent->next;
		}
		if(tempCurrent!=NULL){
			if(tempCurrent->pcb->terminated==1){
				*arg1=1;
			}else if(tempCurrent->pcb->terminated==0){
				*arg1=0;
				tempCurrent->pcb->terminated=1;		
			}
		}else{
			*arg1=1;

		}
	}
	READ_MODIFY(0x7FE00003, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
	READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
	&UnlockResult);
	READ_MODIFY(0x7FE00001, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
}

/***************************************************************************
Start Timer
***************************************************************************/
void startTimer(long *arg0){
	MEMORY_MAPPED_IO mmio;

	long currentTime;
	INT32 LockResult=0;
	INT32 UnlockResult=0;

	READ_MODIFY(0x7FE00001, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	READ_MODIFY(0x7FE00003, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);


	mmio.Mode = Z502ReturnValue;
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	MEM_READ(Z502Clock, &mmio);//hardware call
	currentTime=mmio.Field1;

	/*put the current pcb into timer queue, and set the time to wake up*/
	deReadyPCB->time=currentTime+(long) arg0;
	enTimerQueue(&timerQHead, deReadyPCB);


	if(timerQHead==NULL||deReadyPCB->time<=timerQHead->pcb->time){
		mmio.Mode = Z502Start;
		mmio.Field1 =(long)arg0; 
		mmio.Field2 = mmio.Field3 = 0;
		MEM_WRITE(Z502Timer, &mmio);
	}

	READ_MODIFY(0x7FE00003, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
	READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
	&UnlockResult);
	READ_MODIFY(0x7FE00001, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);

}


/***************************************************************************
suspendProcess

tests for each of the following:

 1. use of illegal process id.
 2. what happens when you suspend yourself - is it legal?  The answer
 to this depends on the OS architecture and is up to the developer.
 3. suspending an already suspended process.

 there are probably lots of other conditions possible.
***************************************************************************/
void suspendProcess(long *arg0, long *arg1){
	struct PCBNode *tempCurrent=NULL;	
	struct PCBNode *tempPrev=NULL;
	INT32 LockResult=0;
	INT32 UnlockResult=0;

	READ_MODIFY(0x7FE00001, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	READ_MODIFY(0x7FE00003, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);

	/*suspending ourselves is not allowed */
	if((INT32) arg0==-1){  //suspend current,but it returns a error
		deReadyPCB->suspend=1;
		*arg1=1;
	}else{
		tempCurrent=readyQHead;  //try to find the pid on ready queue
		while(tempCurrent!=NULL&&tempCurrent->pcb->pid!=(INT32) arg0){
			tempCurrent=tempCurrent->next;
		}
		if(tempCurrent!=NULL){ //the pid is on ready queue
			if(tempCurrent->pcb->suspend==1)   //the pcb is already suspended
				*arg1=1;
			else if(tempCurrent->pcb->suspend==0){
				tempCurrent->pcb->suspend=1;
				*arg1=0;
				suspendNumber++;
			}
		}else{   // the pid is not on ready queue
			tempCurrent=timerQHead;//check timer queue
			while(tempCurrent!=NULL&&tempCurrent->pcb->pid!=(INT32) arg0){
				tempPrev=tempCurrent;
				tempCurrent=tempCurrent->next;
			}
			if(tempCurrent!=NULL){ //the pid is on timer queue. mark it as suspended
				if(tempCurrent->pcb->suspend==1)
					*arg1=1;
				else{
				tempCurrent->pcb->suspend=1;
				*arg1=0;
				suspendNumber++;				
				}
			}else{  //the pid is not on timer queue either, then it is a illegal pid or it's already in suspended queue.
				*arg1=1;
			}
		}
	}

	READ_MODIFY(0x7FE00003, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
	READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
	&UnlockResult);
	READ_MODIFY(0x7FE00001, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
/*other conditions possible*/
}

/***************************************************************************
resumeProcess
 1. use of illegal process id.
 2. resuming a process that's not suspended.
 3. resume ourselves(error).

 there are probably lots of other conditions possible.
***************************************************************************/
void resumeProcess(long *arg0, long *arg1){
	struct PCBNode *tempCurrent=NULL;
	struct PCBNode *tempPrev=NULL; 
	PCB *resumedPCB;
	INT32 LockResult;
	INT32 UnlockResult;

	READ_MODIFY(0x7FE00001, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	READ_MODIFY(0x7FE00003, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);

	/*traverse ready queue to find target pid*/
	tempCurrent=readyQHead;
	while(tempCurrent!=NULL&&tempCurrent->pcb->pid!=(INT32) arg0){
		tempCurrent=tempCurrent->next;
	}
	if(tempCurrent!=NULL){  //target pcb is in ready queue
		if(tempCurrent->pcb->suspend==0){
			*arg1=1;
		}else if(tempCurrent->pcb->suspend==1){
			tempCurrent->pcb->suspend=0;
			*arg1=0;
			suspendNumber--;
		}
	}else{  //target pcb is in suspend q or already resumed or timer queue or illegal pid
		tempCurrent=suspendQHead;
		while(tempCurrent!=NULL&&tempCurrent->pcb->pid!=(INT32) arg0){
			tempPrev=tempCurrent;
			tempCurrent=tempCurrent->next;
		}
		if(tempCurrent!=NULL){  //target pcb is in suspend q
			tempCurrent->pcb->suspend=0;
			*arg1=0;
			suspendNumber--;
			if(tempPrev==NULL){  //target pcb is the head of suspend queue
				resumedPCB=deSuspendQueue(&suspendQHead);
				enReadyQueue(&readyQHead, resumedPCB);
			}else{
				tempPrev->next=tempCurrent->next;
				tempCurrent->next=NULL;
				enReadyQueue(&readyQHead, tempCurrent->pcb);
			}

		}else{  //try to find pcb on timer queue;
			tempCurrent=timerQHead;
			while(tempCurrent!=NULL&&tempCurrent->pcb->pid!=(INT32) arg0){
				tempCurrent=tempCurrent->next;
			}
			if(tempCurrent!=NULL){ //target pcb is in timer queue
				if(tempCurrent->pcb->suspend==0){
					*arg1=1;
				}else if(tempCurrent->pcb->suspend==1){
					tempCurrent->pcb->suspend=0;
					*arg1=0;
					suspendNumber--;
				}
			}else{
				*arg1=1;
			}
		}
	}
	READ_MODIFY(0x7FE00003, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
	READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
	&UnlockResult);
	READ_MODIFY(0x7FE00001, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
}

/***************************************************************************
changePriority
***************************************************************************/
#define         LEGAL_PRIORITY_1G               10
#define         ILLEGAL_PRIORITY_1G            999

void changePriority(long *arg0, long *arg1, long *arg2){
	struct PCBNode *tempCurrent=NULL;
	struct PCBNode *tempPrev=NULL;
	struct PCBNode *pcbNode=NULL;
	INT32 LockResult=0;
	INT32 UnlockResult=0;

	READ_MODIFY(0x7FE00001, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	READ_MODIFY(0x7FE00003, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);

	tempCurrent=readyQHead;
	
	if((INT32) arg1==ILLEGAL_PRIORITY_1G){
		*arg2=1;
		return;
	}
	if((INT32)arg0==-1){  //change priority of current pcb
		deReadyPCB->priority=(INT32)arg1;
		*arg2=0;
		return;
	}
	while(tempCurrent!=NULL&&tempCurrent->pcb->pid!=(INT32) arg0){
		tempPrev=tempCurrent;
		tempCurrent=tempCurrent->next;
	}
	if(tempCurrent!=NULL){  //target pcb found on ready queue;
		tempCurrent->pcb->priority=(INT32) arg1;
		if(tempCurrent==readyQHead){
			readyQHead=tempCurrent->next;
			readyNumber--;
		}else{
			tempPrev->next=tempCurrent->next;
			tempCurrent->next=NULL;
			readyNumber--;
		}
		enReadyQueue(&readyQHead,tempCurrent->pcb);
		*arg2=0;

	}else{
		/*traverse timer queue*/
		tempCurrent=timerQHead;
		while(tempCurrent!=NULL&&tempCurrent->pcb->pid!=(INT32) arg0){
			tempCurrent=tempCurrent->next;
		}
		if(tempCurrent!=NULL){// target pcb found on timer queue
			tempCurrent->pcb->priority=(INT32) arg1;
			*arg2=0;

		}else{     //illegal pid
			*arg2=1;
		}
	}
	READ_MODIFY(0x7FE00003, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
	READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
	&UnlockResult);
	READ_MODIFY(0x7FE00001, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
}

void Loop(){

	INT32 i=30;

	while(i!=0){
		i--;
	}

}

/***************************************************************************
statePrinter
***************************************************************************/
void statePrinter(short call_type,long* arg0, long *arg1, long *arg2, long *arg3, long *arg4){
	INT32 i;
	struct PCBNode *tempCurrent;
	SP_INPUT_DATA SPData; 
	INT32 LockResult=0;
	INT32 UnlockResult=0;

	READ_MODIFY(0x7FE00001, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	READ_MODIFY(0x7FE00003, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	tempCurrent=readyQHead;
	memset(&SPData, 0, sizeof(SP_INPUT_DATA));
	SPData.CurrentlyRunningPID = deReadyPCB->pid;
	SPData.NumberOfRunningProcesses = 0;
	SPData.NumberOfReadyProcesses = readyNumber;
	SPData.NumberOfProcSuspendedProcesses = suspendNumber;
	//SPData.NumberOfMessageSuspendedProcesses = ;
	SPData.NumberOfTimerSuspendedProcesses = timerNumber;

	switch(call_type){
		case SYSNUM_GET_TIME_OF_DAY:
			strcpy(SPData.TargetAction, "GetTime");
			SPData.TargetPID = deReadyPCB->pid;
			break;

		case SYSNUM_SLEEP:
			strcpy(SPData.TargetAction, "Sleep");
			SPData.TargetPID = deReadyPCB->pid;
			break;

		case SYSNUM_CREATE_PROCESS:
			strcpy(SPData.TargetAction, "Create");
			SPData.TargetPID = *arg3;
			break;

		case SYSNUM_GET_PROCESS_ID:
			strcpy(SPData.TargetAction, "GetPID");
			SPData.TargetPID = *arg1;
			break;

		case SYSNUM_SUSPEND_PROCESS:
			strcpy(SPData.TargetAction, "Suspend");
			SPData.TargetPID = (INT32) arg0;
			break;

		case SYSNUM_RESUME_PROCESS:
			strcpy(SPData.TargetAction, "Resume");
			SPData.TargetPID = (INT32) arg0;
			break;

		case SYSNUM_CHANGE_PRIORITY:
			strcpy(SPData.TargetAction, "priority");
			SPData.TargetPID = (INT32) arg0;
			break;

		case SYSNUM_TERMINATE_PROCESS:
			strcpy(SPData.TargetAction, "Terminate");
			SPData.TargetPID = (INT32) arg0;
			break;

	}

	tempCurrent=readyQHead;
	i=0;
	while(tempCurrent!=NULL){
		if(tempCurrent->pcb->suspend!=1&&tempCurrent->pcb->terminated!=1){
			SPData.ReadyProcessPIDs[i]=tempCurrent->pcb->pid;
			i++;		
		}
		tempCurrent=tempCurrent->next;
	}


	tempCurrent=pcbQHead;
	i=0;
	while(tempCurrent!=NULL){
		if(tempCurrent->pcb->terminated!=1&&tempCurrent->pcb->suspend==1){
			SPData.ProcSuspendedProcessPIDs[i]=tempCurrent->pcb->pid;
			i++;
		}
		tempCurrent=tempCurrent->next;
	}


	tempCurrent=timerQHead;
	i=0;
	while(tempCurrent!=NULL){
		if(tempCurrent->pcb->terminated!=1&&tempCurrent->pcb->suspend!=1){
			SPData.TimerSuspendedProcessPIDs[i]=tempCurrent->pcb->pid;
			i++;		
		}
		tempCurrent=tempCurrent->next;
	}


	CALL(SPPrintLine(&SPData));
	READ_MODIFY(0x7FE00003, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
	READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
	&UnlockResult);
	READ_MODIFY(0x7FE00001, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);
}

/***************************************************************************
Send Message
***************************************************************************/
void Send_message(long* arg0, long *arg1, long *arg2, long *arg3){
	struct PCBNode *tempCurrent;
	Message *td=(Message*) malloc(sizeof(Message));
	//char *message;
	INT32 x;
	long *resumeError=(long *)(&x);

	if((INT32)arg0==-1){   //any one can receive except itself
		if((INT32)arg2>LEGAL_MESSAGE_LENGTH||msgNum==11)  //illegal message length or message number exceeds the limit
			*arg3=1;
		else{
			td->SourcePid=deReadyPCB->pid;
			td->TargetPid=(INT32)arg0;
			td->SendLength=(INT32)arg2;
			strcpy(td->MessageSent,(char *)arg1);
			enMessageQueue(&messageQHead,td);
			*arg3=0;
			msgNum++;
			/*resume every suspended PCB if target pid is -1*/
			while(suspendQHead!=NULL){
				resumeProcess((long *)suspendQHead->pcb->pid,resumeError);
			}
			
		}
	}else{
		tempCurrent=pcbQHead;
		while(tempCurrent!=NULL){ //traverse the pcb queue
			if(tempCurrent->pcb->pid==(INT32)arg0&&tempCurrent->pcb->terminated!=1)
				break;
			tempCurrent=tempCurrent->next;
		}
		if(tempCurrent==NULL){ //illegal pid
			*arg3=1;
		}else { //legal target pid
			if((INT32)arg2>LEGAL_MESSAGE_LENGTH||msgNum==11)  //illegal message length or message number exceeds the limit, LIMIT OF MESSAGE NUMBER IS 11
				*arg3=1;
			else{
				td->SourcePid=deReadyPCB->pid;
				td->TargetPid=(INT32)arg0;
				td->SendLength=(INT32)arg2;
				strcpy(td->MessageSent,(char *)arg1);

				enMessageQueue(&messageQHead,td);
				*arg3=0;
				msgNum++;
				resumeProcess((long *)td->TargetPid,resumeError);
			}
		}	
	}
}

/***************************************************************************
Receive Message
***************************************************************************/
void ReceiveMessage(long* arg0, long *arg1, long *arg2, long *arg3, long *arg4, long *arg5){
	struct PCBNode *tempCurrent;
	struct MessageNode *msgCurrent;
	struct MessageNode *msgPrev;
	Message *td;
	INT32 y;
	long *suspendError=(long *)(&y);

	//source pid is -1
	if((INT32)arg0==-1){ 
		msgPrev=NULL;
		msgCurrent=messageQHead;
		while(msgCurrent!=NULL){
			/*try to find target pid is the current pid or target pid is -1,but the source pcb is not itself*/
			if(msgCurrent->message->TargetPid==deReadyPCB->pid
				||(msgCurrent->message->TargetPid==-1&&msgCurrent->message->SourcePid!=deReadyPCB->pid))
				break;
			msgPrev=msgCurrent;
			msgCurrent=msgCurrent->next;
		}
		/*receive the message as we found it*/
		if(msgCurrent!=NULL){
			*arg5=0;
			if(msgPrev!=NULL){
				msgPrev->next=msgCurrent->next;
				msgCurrent->next=NULL;
				td=msgCurrent->message;
			}else{
				td=deMessageQueue(&messageQHead);
			}
			strcpy((char *)arg1,td->MessageSent);
			*arg3=td->SendLength;
			*arg4=td->SourcePid;
			msgNum--;
		}else{//didn't find message, then we need to suspend ourselves and call Dispatcher
			while(msgCurrent==NULL){
				*arg5=1;    //
				suspendProcess((long *)-1,suspendError);
				enSuspendQueue(&suspendQHead,deReadyPCB);
				
				Dispatcher();
				
				/*to make sure next time when we come back, we can still try to find the message for us,
				and we will again suspend ourselves and move to another PCB ,until finally we receive a 
				message then we can exit the loop*/
				msgPrev=NULL;
				msgCurrent=messageQHead;
				while(msgCurrent!=NULL){
					if(msgCurrent->message->TargetPid==deReadyPCB->pid
						||(msgCurrent->message->TargetPid==-1&&msgCurrent->message->SourcePid!=deReadyPCB->pid))
						break;
					msgPrev=msgCurrent;
					msgCurrent=msgCurrent->next;			
				}
				if(msgCurrent!=NULL){
					//printMsgQueue(&messageQHead);
					*arg5=0;
					if(msgPrev!=NULL){
						msgPrev->next=msgCurrent->next;
						msgCurrent->next=NULL;
						td=msgCurrent->message;
					}else{
						td=deMessageQueue(&messageQHead);
					}
					strcpy((char *)arg1,td->MessageSent);
					*arg3=td->SendLength;
					*arg4=td->SourcePid;
					msgNum--;
				}
			}
		}
	}
	//sourcePID is not -1, this situation is like when sourcePID is -1 in the same way
	else{
		tempCurrent=pcbQHead;
		while(tempCurrent!=NULL){ //traverse the pcb queue
			if(tempCurrent->pcb->pid==(INT32)arg0&&tempCurrent->pcb->terminated!=1)
				break;
			tempCurrent=tempCurrent->next;
		}
		if(tempCurrent==NULL){ //illegal pid
			*arg5=1;
		}else{ //legal pid
			if((INT32)arg2>=LEGAL_MESSAGE_LENGTH){  //illegal length
				*arg5=1;
			}else{
				msgCurrent=messageQHead;  //traverse message queue
				msgPrev=NULL;
				while(msgCurrent!=NULL&&msgCurrent->message->SourcePid!=(INT32)arg0){
					msgPrev=msgCurrent;
					msgCurrent=msgCurrent->next;
				}
				if(msgCurrent!=NULL&&msgCurrent->message->TargetPid==deReadyPCB->pid){
					if(msgCurrent->message->SendLength>(INT32)arg2){   //receive length is smaller than send length
						*arg5=1;
					}else{
						arg5=0;
						if(msgPrev!=NULL){
							msgPrev->next=msgCurrent->next;
							msgCurrent->next=NULL;
							td=msgCurrent->message;
						}else{
							td=deMessageQueue(&messageQHead);
						}
						strcpy((char *)arg1,td->MessageSent);
						*arg3=td->SendLength;
						*arg4=td->SourcePid;
						msgNum--;
					}
				}
				else{ 
					while(msgCurrent==NULL){
						*arg5=1;    //
						suspendProcess((long *)-1,suspendError);
						enSuspendQueue(&suspendQHead,deReadyPCB);

						Dispatcher();
						msgPrev=NULL;
						msgCurrent=messageQHead;
						while(msgCurrent!=NULL){
							if(msgCurrent->message->TargetPid==deReadyPCB->pid
								||(msgCurrent->message->TargetPid==-1&&msgCurrent->message->SourcePid!=deReadyPCB->pid))
								break;
							msgPrev=msgCurrent;
							msgCurrent=msgCurrent->next;			
						}
						if(msgCurrent!=NULL){
							*arg5=0;
							if(msgPrev!=NULL){
								msgPrev->next=msgCurrent->next;
								msgCurrent->next=NULL;
								td=msgCurrent->message;
							}else{
								td=deMessageQueue(&messageQHead);
							}
							strcpy((char *)arg1,td->MessageSent);
							*arg3=td->SendLength;
							*arg4=td->SourcePid;
							msgNum--;
						}
					}

				}
			}
		}
	}
}

/***************************************************************************
Project 2:
implementation of virtual memory.address spaces are divided into blocks 
called pages. Processes are allowed to run with only some of the pages 
in their address spaces actually resident in the physical computer memory. 
As long as they only reference the code and data that is resident, there
is no problem. As soon as they reference a "virtual memory" location that 
is not resident in physical memory, the operating system must bring in the 
page that was referenced, replacing some other page if necessary. A reference
to a non-resident page results in a page fault.
***************************************************************************/

/***************************************************************************
Disk Write
Input: DiskID (arg0),Sector (arg1),Written data (arg2)

Write the data to disk, while implenting scheduling as disk operation consumes
time.

1.Check the target disk is free. If not, wait util it's free.
2.Put the current PCB to the corresponding disk queue (8 disk queues in total).
3.Write the data to disk with given ID and sector.
4.Call dispatcher to fetch another PCB in order to continue the scheduling.
***************************************************************************/
void DiskWrite(long* arg0, long *arg1, long *arg2){
	MEMORY_MAPPED_IO mmio;
	INT32 LockResult=0;
	INT32 UnlockResult=0;



	//check if the disk is in use
	mmio.Mode = Z502Status;
	mmio.Field1 = (long) arg0;
	mmio.Field2 = mmio.Field3 = 0;
	MEM_READ(Z502Disk, &mmio);

	while(mmio.Field2!=DEVICE_FREE){
		mmio.Mode = Z502Status;
		mmio.Field1 = (long) arg0;
		mmio.Field2 = mmio.Field3 = 0;
		MEM_READ(Z502Disk, &mmio);	
	}


	READ_MODIFY(0x7FE00004, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	switch((int)arg0){
		case 1:
			enDiskQueue(&diskQHead1,deReadyPCB);
			break;
		case 2:
			enDiskQueue(&diskQHead2,deReadyPCB);
			break;
		case 3:
			enDiskQueue(&diskQHead3,deReadyPCB);
			break;
		case 4:
			enDiskQueue(&diskQHead4,deReadyPCB);
			break;
		case 5:
			enDiskQueue(&diskQHead5,deReadyPCB);
			break;
		case 6:
			enDiskQueue(&diskQHead6,deReadyPCB);
			break;
		case 7:
			enDiskQueue(&diskQHead7,deReadyPCB);
			break;
		case 8:
			enDiskQueue(&diskQHead8,deReadyPCB);
			break;
	}
	READ_MODIFY(0x7FE00004, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);

	mmio.Mode = Z502DiskWrite;
	mmio.Field1 =(long) arg0;
	mmio.Field2 = (long) arg1;
	mmio.Field3 = (long) arg2;
	MEM_WRITE(Z502Disk, &mmio);

	Dispatcher();

}

/***************************************************************************
Disk Read
Input: DiskID (arg0),Sector (arg1),Data to be read (arg2)

Read data from disk, while implenting scheduling as disk operation consumes
time.

1.Check the target disk is free. If not, wait util it's free.
2.Put the current PCB to the corresponding disk queue (8 disk queues in total).
3.Read the data from the specific position of disk with given ID and sector.
4.Call dispatcher to fetch another PCB in order to continue the scheduling.
***************************************************************************/
void DiskRead(long* arg0, long *arg1, long *arg2){
	MEMORY_MAPPED_IO mmio;
	INT32 LockResult=0;
	INT32 UnlockResult=0;



	//check if the disk is in use
	mmio.Mode = Z502Status;
	mmio.Field1 = (long) arg0;
	mmio.Field2 = mmio.Field3 = 0;
	MEM_READ(Z502Disk, &mmio);
	
	while(mmio.Field2!=DEVICE_FREE){
		mmio.Mode = Z502Status;
		mmio.Field1 = (long) arg0;
		mmio.Field2 = mmio.Field3 = 0;
		MEM_READ(Z502Disk, &mmio);	
	}


	READ_MODIFY(0x7FE00004, DO_LOCK, SUSPEND_UNTIL_LOCKED,
	&LockResult);
	switch((int)arg0){
		case 1:
			enDiskQueue(&diskQHead1,deReadyPCB);
			break;
		case 2:
			enDiskQueue(&diskQHead2,deReadyPCB);
			break;
		case 3:
			enDiskQueue(&diskQHead3,deReadyPCB);
			break;
		case 4:
			enDiskQueue(&diskQHead4,deReadyPCB);
			break;
		case 5:
			enDiskQueue(&diskQHead5,deReadyPCB);
			break;
		case 6:
			enDiskQueue(&diskQHead6,deReadyPCB);
			break;
		case 7:
			enDiskQueue(&diskQHead7,deReadyPCB);
			break;
		case 8:
			enDiskQueue(&diskQHead8,deReadyPCB);
			break;
	}
	READ_MODIFY(0x7FE00004, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
		&UnlockResult);

	mmio.Mode = Z502DiskRead;
	mmio.Field1 =(long) arg0;
	mmio.Field2 = (long) arg1;
	mmio.Field3 = (long) arg2;
	MEM_WRITE(Z502Disk, &mmio);

	Dispatcher();
}

/***************************************************************************
Page Replacement
transfer pages from frames to disks or transfer pages from disks to frames.
Least Recently Used for selecting a victim page.
***************************************************************************/
/***************************************************************************
Swipe out
Input: position (frame to be swiped out), process ID, Page

When physical memory is full, find a victim on frames to swipe out to disk.
Victim is indicated by position. 

1.Read the victim's data and store it at a memory buffer.
2.look into shadowed page table for DiskID and Sector.If not find,create a 
new one for the page.
3.make a new time stamp for the page to accomplish LRU in the future.
4.Do a disk write to swipe it out.
5.change the valid bit and reverved bit indicating the page is on disk.
***************************************************************************/
void PageReplaceOut(UINT16 position,int processID,int Page){
	long DiskID;
	long Sector;
	char physical_memory_out[16];
	ShadowedPage *shadowedpagetable;
	ShadowedPage shadowedpage;
	UINT16 *pgtbl;

	Z502ReadPhysicalMemory(position, (char *) physical_memory_out);

	shadowedpagetable=ShadowedTable[processID];
	shadowedpage=shadowedpagetable[Page];
	if(shadowedpage.DiskID!=0){
		DiskID=shadowedpage.DiskID;
		Sector=shadowedpage.Sector;
	}

	else{

		DiskItr=DiskItr % MAX_NUMBER_OF_DISKS+1;
		if(DiskItr==1){
			SectorItr++;
		}
		DiskID=DiskItr;
		Sector=SectorItr;
		shadowedpagetable[Page].DiskID=DiskID;
	    shadowedpagetable[Page].Sector=Sector;
	}
	FrameTable[position].time=0;
	DiskWrite((long *)DiskID,(long *)Sector,(long *)(physical_memory_out));


	pgtbl=(UINT16 *)PageTable[processID];

	pgtbl[Page]=pgtbl[Page]&0x7FFF;
	pgtbl[Page]=pgtbl[Page]|0x1000;
}

/***************************************************************************
Swipe In
Input: position (frame to be swiped out), Page(Status)

Last step of page replacement. After the victim page is swiped out, swipe the 
target page in from disk.

1.Update the frame position of target page.
2.Update the state of the page.
3.Look into shadowed page table to find the target's DiskID and Sector,
then read it back from disk and write it to frames.
4.Update the frmae table which contains page number and PID.
***************************************************************************/
void PageReplaceIn(UINT16 position,int Status){
	long DiskID;
	long Sector;
	ShadowedPage* shadowedpagetable;
	ShadowedPage shadowedpage;
	char physical_memory_in[16];

	PageTable[deReadyPCB->pid][Status]=position|0x8000&0xEFFF;
	shadowedpagetable=ShadowedTable[deReadyPCB->pid];
	shadowedpage=shadowedpagetable[Status];
	DiskID=shadowedpage.DiskID;
	Sector=shadowedpage.Sector;

	DiskRead((long *)DiskID,(long *)Sector,(long *)(physical_memory_in));

	Z502WritePhysicalMemory(position, (char *) physical_memory_in);
	FrameTable[position].Page=Status;
	FrameTable[position].processID=deReadyPCB->pid;
}

/***************************************************************************
Least Recently Used
Outcome: the victim page with the largest time stamp.
***************************************************************************/
INT32 LeastRecentlyUsed(){
	int count;
	int maxTime=INT_MIN;
	int victim=-1;;
	for(count=0;count<64;count++){
		if(FrameTable[count].time>maxTime){
			maxTime=FrameTable[count].time;
			victim=count;
		}
	}
	return victim;
	
}

/***************************************************************************
Memory Printer
***************************************************************************/
void MemoryPrinter(){
	MP_INPUT_DATA MPData;
	int j;
	int state;
	UINT16 pageBit;
	UINT16 valid ,modified,referenced;
	state=0;
	memset(&MPData, 0, sizeof(MP_INPUT_DATA));

	for (j = 0; j < PHYS_MEM_PGS ; j = j + 2) {
		if(Bits<63&&flag==0){
			if(j<=Bits){
				MPData.frames[j].InUse = TRUE;
			}else{
				MPData.frames[j].InUse = FALSE;
			}
		}else{
			MPData.frames[j].InUse = TRUE;
		}
		
		MPData.frames[j].Pid=FrameTable[j].processID;
		MPData.frames[j].LogicalPage=FrameTable[j].Page;
		pageBit=PageTable[MPData.frames[j].Pid][MPData.frames[j].LogicalPage];
		valid=pageBit&0x8000;
		if(valid==0x8000){
			state=state+FRAME_VALID;
		}

		modified=pageBit&0x4000;
		if(modified==0x4000){
			state=state+FRAME_MODIFIED;
		}

		referenced=pageBit&0x2000;
		if(referenced==0x2000){
			state=state+FRAME_REFERENCED;
		}
		MPData.frames[j].State=state;
		state=0;
	}
	MPPrintLine(&MPData);
}

/***************************************************************************
Define Shared Area
Input:
	Starting Address Of Shared Area (arg0)
	Pages In Shared Area (arg1)
	Area Tag (arg2)
Output:
	ID of the shared area
	error returned
Data occupying one physical page, but pointed to by multiple logical pages.
Define the front section of frames to be shared physical memory. With start 
page number and page size passed in, define the corresponding page portion 
to be shared area.
***************************************************************************/
void DefineSharedArea(long* arg0,long* arg1,long* arg2,long* arg3,long* arg4){
	int startPageNumber;
	int sharedSize;
	int currentPID;
	int i;
	int framePos;
	startPageNumber=(int)arg0/PGSIZE;
	sharedSize=(int)arg1;

	currentPID=deReadyPCB->pid;
	for(i=0;i<sharedSize;i++){
		framePos=i;
		PageTable[currentPID][i+startPageNumber]=i|0x8000;
	}

	sharedID++;//begins with 0
	*arg3=sharedID;
	*arg4=0;

}