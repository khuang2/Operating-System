/************************************************************************

 This code forms the base of the operating system you will
 build.  It has only the barest rudiments of what you will
 eventually construct; yet it contains the interfaces that
 allow test.c and z502.c to be successfully built together.

 Revision History:
 1.0 August 1990
 1.1 December 1990: Portability attempted.
 1.3 July     1992: More Portability enhancements.
 Add call to SampleCode.
 1.4 December 1992: Limit (temporarily) printout in
 interrupt handler.  More portability.
 2.0 January  2000: A number of small changes.
 2.1 May      2001: Bug fixes and clear STAT_VECTOR
 2.2 July     2002: Make code appropriate for undergrads.
 Default program start is in test0.
 3.0 August   2004: Modified to support memory mapped IO
 3.1 August   2004: hardware interrupt runs on separate thread
 3.11 August  2004: Support for OS level locking
 4.0  July    2013: Major portions rewritten to support multiple threads
 4.20 Jan     2015: Thread safe code - prepare for multiprocessors
 ************************************************************************/

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

//  Allows the OS and the hardware to agree on where faults occur
extern void *TO_VECTOR[];

char *call_names[] = { "mem_read ", "mem_write", "read_mod ", "get_time ",
		"sleep    ", "get_pid  ", "create   ", "term_proc", "suspend  ",
		"resume   ", "ch_prior ", "send     ", "receive  ", "disk_read",
		"disk_wrt ", "def_sh_ar" };


/************************************************************************
 INTERRUPT_HANDLER
 When the Z502 gets a hardware interrupt, it transfers control to
 this routine in the OS.Catch valid interrupts as much as possible (Using 
 While Loop).
 If it's a disk interrupt,remove the PCB from its disk queue (8 disk queues 
 in total) and add it to ready queue.
 If it's a timer interrupt,remove the PCB from timer queue and add it to 
 ready queue.
 ************************************************************************/
void InterruptHandler(void) {
	INT32 DeviceID;
	PCB *tempPCB;

	INT32 currTime;  //current time

	INT32 newDelay;


	MEMORY_MAPPED_IO mmio;       // Enables communication with hardware

	INT32 LockResult=0;
	INT32 UnlockResult=0;
	
	static BOOL remove_this_in_your_code = TRUE; /** TEMP **/
	static INT32 how_many_interrupt_entries = 0; /** TEMP **/

	mmio.Mode = Z502GetInterruptInfo;
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	MEM_READ(Z502InterruptDevice, &mmio);
	//DeviceID = mmio.Field1;


	while  ( mmio.Field4 == ERR_SUCCESS ) {               //a valid interrupt
			DeviceID = mmio.Field1;

			//Disk Interrupt

			switch(DeviceID){
				case DISK_INTERRUPT_DISK1:
					READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					READ_MODIFY(0x7FE00004, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					tempPCB=deDiskQueue(&diskQHead1);
					enReadyQueue(&readyQHead,tempPCB);
					READ_MODIFY(0x7FE00004, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					break;

				case DISK_INTERRUPT_DISK2:
					READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					READ_MODIFY(0x7FE00004, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					tempPCB=deDiskQueue(&diskQHead2);
					enReadyQueue(&readyQHead,tempPCB);
					READ_MODIFY(0x7FE00004, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					break;

				case DISK_INTERRUPT_DISK3:
					READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					READ_MODIFY(0x7FE00004, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					tempPCB=deDiskQueue(&diskQHead3);
					enReadyQueue(&readyQHead,tempPCB);
					READ_MODIFY(0x7FE00004, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					break;

				case DISK_INTERRUPT_DISK4:
					READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					READ_MODIFY(0x7FE00004, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					tempPCB=deDiskQueue(&diskQHead4);
					enReadyQueue(&readyQHead,tempPCB);
					READ_MODIFY(0x7FE00004, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					break;

				case DISK_INTERRUPT_DISK5:
					READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					READ_MODIFY(0x7FE00004, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					tempPCB=deDiskQueue(&diskQHead5);
					enReadyQueue(&readyQHead,tempPCB);
					READ_MODIFY(0x7FE00004, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					break;

				case DISK_INTERRUPT_DISK6:
					READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					READ_MODIFY(0x7FE00004, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					tempPCB=deDiskQueue(&diskQHead6);
					enReadyQueue(&readyQHead,tempPCB);
					READ_MODIFY(0x7FE00004, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					break;

				case DISK_INTERRUPT_DISK7:
					READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					READ_MODIFY(0x7FE00004, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					tempPCB=deDiskQueue(&diskQHead7);
					enReadyQueue(&readyQHead,tempPCB);
					READ_MODIFY(0x7FE00004, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					break;

				case DISK_INTERRUPT_DISK8:
					READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					READ_MODIFY(0x7FE00004, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					tempPCB=deDiskQueue(&diskQHead8);
					enReadyQueue(&readyQHead,tempPCB);
					READ_MODIFY(0x7FE00004, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					break;

				case TIMER_INTERRUPT:
					//get current time 
					mmio.Mode = Z502ReturnValue;
					mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
					MEM_READ(Z502Clock, &mmio);//hardware call
					currTime = mmio.Field1;

					READ_MODIFY(0x7FE00001, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					READ_MODIFY(0x7FE00002, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					READ_MODIFY(0x7FE00003, DO_LOCK, SUSPEND_UNTIL_LOCKED,
					&LockResult);
					while(timerQHead!=NULL&&timerQHead->pcb->time<=currTime){


						tempPCB=deTimerQueue(&timerQHead);
		

						if(tempPCB->suspend==1){
							enSuspendQueue(&suspendQHead, tempPCB);
						}else{

							enReadyQueue(&readyQHead,tempPCB);
		
						}

						mmio.Mode = Z502ReturnValue;
						mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
						MEM_READ(Z502Clock, &mmio);//hardware call
						currTime = mmio.Field1;
					}


						mmio.Mode = Z502ReturnValue;
						mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
						MEM_READ(Z502Clock, &mmio);//hardware call
						currTime = mmio.Field1;




					//reset the timer with the new delay time.
					if(timerQHead!=NULL){
						newDelay=timerQHead->pcb->time-currTime;
						mmio.Mode = Z502Start;
						mmio.Field1 =(long) newDelay;
						mmio.Field2 = mmio.Field3 = 0;
						MEM_WRITE(Z502Timer, &mmio);


					}

					READ_MODIFY(0x7FE00003, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					READ_MODIFY(0x7FE00002, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
					&UnlockResult);
					READ_MODIFY(0x7FE00001, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
						&UnlockResult);
					break;

			}

			

			 
			mmio.Mode = Z502ClearInterruptStatus;            // Now clear that interrupt
			mmio.Field1 = DeviceID;
			mmio.Field2 = mmio.Field3  = mmio.Field4 = 0;
			MEM_WRITE( Z502InterruptDevice, &mmio );
			mmio.Mode = Z502GetInterruptInfo;               // See if there's another Interrupt
			mmio.Field1 = mmio.Field2 = mmio.Field3  = mmio.Field4 = 0;
			MEM_READ( Z502InterruptDevice, &mmio );
	}   // End of while
	
}           // End of InterruptHandler

/************************************************************************
 FAULT_HANDLER
 The beginning of the OS502.  Used to receive hardware faults.
 First, check whether the demand page is beyond the maximum pages of PCB.
 Then, fault handler checks if the demand page resides on disks,and this
 is using the reserved bit of the page. 

 If it is on disks, then we select a victim page on the frame to replace 
 the demand page. The selection is using LRU mechanism. After we selected 
 the victim, we swipe out the victim page, and swipe in the demand page at
 the same position of the frames, thus accomplish a page replacement.

 If the demand page is not on disks, then this is a new page, or else we 
 wouldn¡¯t come to fault handler. If the frames are full, again we need to
 select a victim page on the frames to swipe it out to disks. Finally, fault
 handler mark the valid bit.

 ************************************************************************/

void FaultHandler(void) {
	INT32 DeviceID;
	INT32 Status;
	INT32 Page;  //the page that resides in the frame
	INT32 P; //the pcb that the page belongs to
	INT32 k;
	UINT16 i;//bits of pages
	UINT16 r;//to judge if the page resides on frame or disk
	INT32 count;
	MEMORY_MAPPED_IO mmio;       // Enables communication with hardware
	UINT16 frame_Position;  //use a local variable to indicate the frame position, because the global variable varies each time.

	// Get cause of interrupt
	mmio.Mode = Z502GetInterruptInfo;
	MEM_READ(Z502InterruptDevice, &mmio);
	DeviceID = mmio.Field1;
	Status = mmio.Field2; //page number

	if(Status>=1024){ //page number is from 0 to 1023
		printf("Fault_handler: Found vector type %d with value %d\n", DeviceID,
			Status);
		terminateSysProcess((long *)-1,(long *)&k);
	}

	/*Frame number is from 0 to 63, firstFault indicates if it's the first page fault.
	If it is,use the default page number which is 0. Otherwise, increment the page number
	every time a page fault occurs.*/
	if(firstFault==1){ 
		firstFault=0;
	}else{
		Bits++;
	}
	
	/*flag indicates the occurance of page faults.*/
	if(flag==0){ //if no page fault occurs,which means frames is not full yet, increment the time stamp of the occupied frames.
		for(count=0;count<=Bits;count++){
			FrameTable[count].time++;
		}
	}
	if(flag==1){// if page faults has occurred, which means all the frames are occupied, increment the time stamp of all the frames.
		for(count=0;count<64;count++){
			FrameTable[count].time++;
		}
	}

	if(Bits<=63&&flag==0){ //frames are not full yet
		frame_Position=Bits;
	}
	else if(Bits>63&&flag==0){   //the first time page replacement will be occuring
		
		frame_Position=LeastRecentlyUsed(); //use LRU to select a victim page
		flag=1;
	}else if(flag==1){ //page replacement has occured, then each time a page fault occurs, a page replacement should be implemented.
		frame_Position=LeastRecentlyUsed();
	}

	r=PageTable[deReadyPCB->pid][Status]&PTBL_DISK_RESIDENT_BIT;

	if(r==0x1000){ //the page resides in disk

		Page= FrameTable[frame_Position].Page;  //page to be swipe out
		P=FrameTable[frame_Position].processID;
		
		PageReplaceOut(frame_Position,P,Page);
		PageReplaceIn(frame_Position,Status); //using shadowedTable

	}else{
		if(flag==1){ //if page replacement has occurred, look into frame table for the occuping page.
			Page=FrameTable[frame_Position].Page;
			P=FrameTable[frame_Position].processID;
			PageReplaceOut(frame_Position,P,Page);
		}

		i=frame_Position|PTBL_VALID_BIT;  //change the valid bit
		PageTable[deReadyPCB->pid][Status]= (UINT16)i;

		/*update the frame table of the new page*/
		FrameTable[frame_Position].Page=Status;
		FrameTable[frame_Position].processID=deReadyPCB->pid;
				
	}
	
	MemoryPrinter();

	// Clear out this device - we're done with it
	mmio.Mode = Z502ClearInterruptStatus;
	mmio.Field1 = DeviceID;
	MEM_WRITE(Z502InterruptDevice, &mmio);
} // End of FaultHandler

/************************************************************************
 SVC
 The beginning of the OS502.  Used to receive software interrupts.
 All system calls come to this point in the code and are to be
 handled by the student written code here.
 The variable do_print is designed to print out the data for the
 incoming calls, but does so only for the first ten calls.  This
 allows the user to see what's happening, but doesn't overwhelm
 with the amount of data.
 ************************************************************************/


INT32 n=0;//the number of calling clock

void svc(SYSTEM_CALL_DATA *SystemCallData) {
	MEMORY_MAPPED_IO mmio; 
	short call_type;
	static short do_print = 10;
	INT32 Time;
	INT32 Status;
	short i;
	long* arg0=SystemCallData->Argument[0];
	long* arg1=SystemCallData->Argument[1];
	long* arg2=SystemCallData->Argument[2];
	long* arg3=SystemCallData->Argument[3];
	long* arg4=SystemCallData->Argument[4];
	long *arg5=SystemCallData->Argument[5];
	call_type = (short) SystemCallData->SystemCallNumber;
	if (do_print > 0) {
		printf("SVC handler: %s\n", call_names[call_type]);
		for (i = 0; i < SystemCallData->NumberOfArguments - 1; i++) {
			printf("Arg %d: Contents = (Decimal) %8ld,  (Hex) %8lX\n", i,
					(unsigned long) SystemCallData->Argument[i],
					(unsigned long) SystemCallData->Argument[i]);
		} 
		do_print--;
	}

	switch (call_type) {
		// Get time service call
		case SYSNUM_GET_TIME_OF_DAY:
			mmio.Mode = Z502ReturnValue;
			mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
			MEM_READ(Z502Clock, &mmio);//hardware call
			Time = mmio.Field1;
			(SystemCallData->Argument[0])=(long*)&Time;

			//statePrinter(call_type,arg0, arg1, arg2, arg3, arg4);
			break;
				// get the Timer start and generate idle.
		case SYSNUM_SLEEP:

			startTimer(arg0);

			//statePrinter(call_type,arg0, arg1, arg2, arg3, arg4);

			Dispatcher();//call dispatcher	

			break;

		// create process
		case SYSNUM_CREATE_PROCESS:
			OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
			//statePrinter(call_type,arg0, arg1, arg2, arg3, arg4);
			break;

		// get process id
		case SYSNUM_GET_PROCESS_ID:
			getProcessID(arg0,arg1,arg2);
			//statePrinter(call_type,arg0, arg1, arg2, arg3, arg4);
			break;

		// suspend process
		case SYSNUM_SUSPEND_PROCESS:
			suspendProcess(arg0, arg1);
			//statePrinter(call_type,arg0, arg1, arg2, arg3, arg4);
			break;
		
		// resume the process
		case SYSNUM_RESUME_PROCESS:
			resumeProcess(arg0, arg1);
			//statePrinter(call_type,arg0, arg1, arg2, arg3, arg4);
			break;
		
		// change priority;
		case SYSNUM_CHANGE_PRIORITY:
			changePriority(arg0,arg1,arg2);
			//statePrinter(call_type,arg0, arg1, arg2, arg3, arg4);
			break;

		// terminate system call
		case SYSNUM_TERMINATE_PROCESS:
			terminateSysProcess(arg0,arg1);
			break;

		case SYSNUM_SEND_MESSAGE:
			Send_message(arg0, arg1, arg2, arg3);

			break;

		case SYSNUM_RECEIVE_MESSAGE:
			ReceiveMessage(arg0, arg1, arg2, arg3, arg4, arg5);
			break;
		
		case SYSNUM_DISK_WRITE:
			DiskWrite(arg0, arg1, arg2);
			break;

		case SYSNUM_DISK_READ:
			DiskRead(arg0, arg1, arg2);
			break;

		case SYSNUM_DEFINE_SHARED_AREA:
			DefineSharedArea(arg0, arg1, arg2,arg3,arg4);
			break;

		default:
			printf( "ERROR!  call_type not recognized!\n" ); 
			printf( "Call_type is - %i\n", call_type);

	}
}                                               // End of svc

/************************************************************************
 osInit (together with OSCreateProcess which is a subroutine.)
 This is the first routine called after the simulation begins.  This
 is equivalent to boot code.  All the initial OS components can be
 defined and initialized here.
 ************************************************************************/

void osInit(int argc, char *argv[]) {
	INT32 i;
	MEMORY_MAPPED_IO mmio;
	long a;
	long b;
	long* arg0=(long*)"test2a"; //default ruuning test
	long* arg1=(long*)test2a;
	long* arg2=(long*)10;
	long* arg3=& a;
	long* arg4=& b;

  // Demonstrates how calling arguments are passed thru to here       

    printf( "Program called with %d arguments:", argc );
    for ( i = 0; i < argc; i++ )
        printf( " %s", argv[i] );
    printf( "\n" );
    printf( "Calling with argument 'sample' executes the sample program.\n" );
    // Here we check if a second argument is present on the command line.
    // If so, run in multiprocessor mode
    if ( argc > 2 ){
    	printf("Simulation is running as a MultProcessor\n\n");
		mmio.Mode = Z502SetProcessorNumber;
		mmio.Field1 = MAX_NUMBER_OF_PROCESSORS;
		mmio.Field2 = (long) 0;
		mmio.Field3 = (long) 0;
		mmio.Field4 = (long) 0;

		MEM_WRITE(Z502Processor, &mmio);   // Set the number of processors
    }
    else {
    	printf("Simulation is running as a UniProcessor\n");
    	printf("Add an 'M' to the command line to invoke multiprocessor operation.\n\n");
    }

	//          Setup so handlers will come to code in base.c   

	TO_VECTOR[TO_VECTOR_INT_HANDLER_ADDR ] = (void *) InterruptHandler;
	TO_VECTOR[TO_VECTOR_FAULT_HANDLER_ADDR ] = (void *) FaultHandler;
	TO_VECTOR[TO_VECTOR_TRAP_HANDLER_ADDR ] = (void *) svc;


	if((argc > 1) && (strcmp(argv[1], "test1a") == 0)){
		arg0=(long*)"test1a";
		arg1=(long*)test1a;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test1b") == 0)){
		arg0=(long*)"test1b";
		arg1=(long*)test1b;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test1c") == 0)){
		arg0=(long*)"test1c";
		arg1=(long*)test1c;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test1d") == 0)){
		arg0=(long*)"test1d";
		arg1=(long*)test1d;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test1e") == 0)){
		arg0=(long*)"test1e";
		arg1=(long*)test1e;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test1f") == 0)){
		arg0=(long*)"test1f";
		arg1=(long*)test1f;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test1g") == 0)){
		arg0=(long*)"test1g";
		arg1=(long*)test1g;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test1h") == 0)){
		arg0=(long*)"test1h";
		arg1=(long*)test1h;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test1i") == 0)){
		arg0=(long*)"test1i";
		arg1=(long*)test1i;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test1j") == 0)){
		arg0=(long*)"test1j";
		arg1=(long*)test1j;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test1k") == 0)){
		arg0=(long*)"test1k";
		arg1=(long*)test1k;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}

	/*project2*/
	if((argc > 1) && (strcmp(argv[1], "test2a") == 0)){
		arg0=(long*)"test2a";
		arg1=(long*)test2a;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test2b") == 0)){
		arg0=(long*)"test2b";
		arg1=(long*)test2b;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test2c") == 0)){
		arg0=(long*)"test2c";
		arg1=(long*)test2c;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test2d") == 0)){
		arg0=(long*)"test2d";
		arg1=(long*)test2d;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test2e") == 0)){
		arg0=(long*)"test2e";
		arg1=(long*)test2e;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test2f") == 0)){
		arg0=(long*)"test2f";
		arg1=(long*)test2f;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test2g") == 0)){
		arg0=(long*)"test2g";
		arg1=(long*)test2g;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}
	if((argc > 1) && (strcmp(argv[1], "test2h") == 0)){
		arg0=(long*)"test2h";
		arg1=(long*)test2h;
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}

	else{
		OSCreateProcess(arg0,arg1,arg2,arg3,arg4);
		Dispatcher();
	}


}                                               // End of osInit

