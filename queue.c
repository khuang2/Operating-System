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

/***************************************************************************
 Timer Queue, which is implemented by linked list.
***************************************************************************/
void enTimerQueue(struct PCBNode **head_ref, PCB *pcb){//function to add a node with the given PCB to the timer queue by time priority;
	struct PCBNode* newNode = (struct PCBNode*) malloc(sizeof(struct PCBNode));
	struct PCBNode *current=*head_ref;
	struct PCBNode *theNext=(struct PCBNode*) malloc(sizeof(struct PCBNode));
	INT32 key;
	newNode->pcb=pcb;
	newNode->next=NULL;
	if(*head_ref==NULL){
		*head_ref=newNode;
		timerNumber++;
		return;
	}
	if(newNode->pcb->time<(*head_ref)->pcb->time){
		newNode->next=*head_ref;
		*head_ref=newNode;
		timerNumber++;
		return;
	}

	key=newNode->pcb->time;
	theNext=current->next;
	if(theNext==NULL){
		current->next=newNode;
		timerNumber++;
		return;
	}
	while(theNext!=NULL&&key>=theNext->pcb->time){
		current=theNext;
		theNext=theNext->next;
	}
	current->next=newNode;
	newNode->next=theNext;
	timerNumber++;
}


PCB *deTimerQueue(struct PCBNode **head_ref){
	struct PCBNode *temp;

	if(*head_ref==NULL)
		return NULL;
	temp=*head_ref;
	*head_ref=(*head_ref)->next;

	timerNumber--;

	return temp->pcb;
}

void printTimerQueue(struct PCBNode **head_ref){
	struct PCBNode *current=*head_ref;
	printf("@@@@timerQueue:");
	while(current!=NULL){
		printf("%d-->",current->pcb->pid);
		current=current->next;
	}
	if(*head_ref==NULL)
		printf("timer q is@ null");
}

/***************************************************************************
Ready Queue
***************************************************************************/
void enReadyQueue(struct PCBNode **head_ref, PCB *pcb){//function to add a node with the given PCB to the timer queue by time priority;
	struct PCBNode* newNode = (struct PCBNode*) malloc(sizeof(struct PCBNode));
	struct PCBNode *current=*head_ref;
	struct PCBNode *theNext=(struct PCBNode*) malloc(sizeof(struct PCBNode));
	INT32 key;
	newNode->pcb=pcb;
	newNode->next=NULL;
	if(*head_ref==NULL){
		*head_ref=newNode;
		readyNumber++;
		return;
	}
	if(newNode->pcb->priority<(*head_ref)->pcb->priority){
		newNode->next=*head_ref;
		*head_ref=newNode;
		readyNumber++;
		return;
	}

	key=newNode->pcb->priority;
	theNext=current->next;
	if(theNext==NULL){
		current->next=newNode;
		readyNumber++;
		return;
	}
	while(theNext!=NULL&&key>=theNext->pcb->priority){
		current=theNext;
		theNext=theNext->next;
	}
	current->next=newNode;
	newNode->next=theNext;
	readyNumber++;

	//struct PCBNode* newNode = (struct PCBNode*) malloc(sizeof(struct PCBNode));
	//struct PCBNode *current=*head_ref;
	//newNode->pcb=pcb;
	//newNode->next=NULL;
	//if(*head_ref==NULL){
	//	*head_ref=newNode;
	//	return;
	//}
	//while(current->next!=NULL){
	//	current=current->next;
	//}
	//current->next=newNode;
}


PCB *deReadyQueue(struct PCBNode **head_ref){
	struct PCBNode *temp;

	if(*head_ref==NULL)
		return NULL;
	temp=*head_ref;
	*head_ref=(*head_ref)->next;
	readyNumber--;
	return temp->pcb;
}

void printReadyQueue(struct PCBNode **head_ref){
	struct PCBNode *current=*head_ref;
	printf("@@@@readyQueue:");
	while(current!=NULL){
		printf("%d-->",current->pcb->pid);
		current=current->next;
	}
}
/***************************************************************************
PCB Queue
***************************************************************************/
void enPCBQueue(struct PCBNode **head_ref, PCB *pcb){
	struct PCBNode* newNode = (struct PCBNode*) malloc(sizeof(struct PCBNode));
	struct PCBNode *current=*head_ref;
	newNode->pcb=pcb;
	newNode->next=NULL;
	if(*head_ref==NULL){
		*head_ref=newNode;
		return;
	}
	while(current->next!=NULL){
		current=current->next;
	}
	current->next=newNode;
	
	
}

/***************************************************************************
Suspend Queue
***************************************************************************/
void enSuspendQueue(struct PCBNode **head_ref, PCB *pcb){
	struct PCBNode* newNode = (struct PCBNode*) malloc(sizeof(struct PCBNode));
	struct PCBNode *current=*head_ref;
	newNode->pcb=pcb;
	newNode->next=NULL;
	if(*head_ref==NULL){
		*head_ref=newNode;
		return;
	}
	while(current->next!=NULL){
		current=current->next;
	}
	current->next=newNode;
}

PCB *deSuspendQueue(struct PCBNode **head_ref){
	struct PCBNode *temp;

	if(*head_ref==NULL)
		return NULL;
	temp=*head_ref;
	*head_ref=(*head_ref)->next;

	return temp->pcb;
}

/***************************************************************************
Message Queue
***************************************************************************/
void enMessageQueue(struct MessageNode **head_ref, Message *message){
	struct MessageNode* newNode = (struct MessageNode*) malloc(sizeof(struct MessageNode));
	struct MessageNode *current=*head_ref;
	newNode->message=message;
	newNode->next=NULL;
	if(*head_ref==NULL){
		*head_ref=newNode;
		return;
	}
	while(current->next!=NULL){
		current=current->next;
	}
	current->next=newNode;
}

Message *deMessageQueue(struct MessageNode **head_ref){
	struct MessageNode *temp;

	if(*head_ref==NULL)
		return NULL;
	temp=*head_ref;
	*head_ref=(*head_ref)->next;

	return temp->message;
}

void printMsgQueue(struct MessageNode **head_ref){
	struct MessageNode *current=*head_ref;
	while(current!=NULL){
		printf("sourcepid %d\n",current->message->SourcePid);
		printf("targetpid %d\n",current->message->TargetPid);
		current=current->next;
	}
}

/***************************************************************************
Disk Queue 
***************************************************************************/
void enDiskQueue(struct PCBNode **head_ref, PCB *pcb){
	struct PCBNode* newNode = (struct PCBNode*) malloc(sizeof(struct PCBNode));
	struct PCBNode *current=*head_ref;
	newNode->pcb=pcb;
	newNode->next=NULL;
	if(*head_ref==NULL){
		*head_ref=newNode;
		return;
	}
	while(current->next!=NULL){
		current=current->next;
	}
	current->next=newNode;
}

PCB *deDiskQueue(struct PCBNode **head_ref){
	struct PCBNode *temp;

	if(*head_ref==NULL)
		return NULL;
	temp=*head_ref;
	*head_ref=(*head_ref)->next;

	return temp->pcb;
}

void printDiskQueue(struct PCBNode **head_ref){
	struct PCBNode *current=*head_ref;
	printf("@@@@diskQueue:");
	while(current!=NULL){
		printf("%d-->",current->pcb->pid);
		current=current->next;
	}
}
