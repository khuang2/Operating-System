/***************************************************************************

  global.h

      This include file is used by both the OS502 and
      the Z502 Simulator.

      Revision History:
        1.0 August 1990:        first release
        1.1 Jan    1991:        Additional disks added
        1.3 July   1992:        Make as many defines as possible
                                into shorts since PCs are happier.
        1.6 June   1995:        Minor changes
        2.0 Jan    2000:        A large number of small changes.
        2.1 May    2001:        Redefine disk layout
        2.2 July   2002:        Make code appropriate for undergrads.
        3.0 August 2004:        Modified to support memory mapped IO
        3.1 August 2004:        Implement Memory Mapped IO
        3.11 August 2004:       Implement OS level locking capability
        3.13 Nov.   2004        Change priorities of LINUX threads
        3.40 August 2008        Fix code for 64 bit addresses
        3.41 Sept.  2008        Change Z502_ARG to support 64 bit addr.
        3.50 August 2009        Minor cosmetics
        3.60 August 2012        Updates with student generated code to
                                support MACs
        4.00 August 2013        Major revision of how tests run.  There are 
                                now many threads, one for each process.  All
                                macros in the tests have disappeared.
        4.01 October 2013       Small fix in test1j.
        4.20 May 2015:          Serious changes to MemoryMappedIO.  Made atomic
        .                       StatePrinter made atomic
****************************************************************************/
#ifndef GLOBAL_H_
#define GLOBAL_H_

#define         CURRENT_REL                     "4.20"
/****************************************************************************
    Choose one of the operating systems below
****************************************************************************/
#define         NT
//#define         LINUX
//#define         MAC


/*****************************************************************
    The next five defines have special meaning.  They allow the
    Z502 processor to report information about its state.  From
    this, you can find what the hardware thinks is going on.
    The information produced when this debugging is on is NOT
    something that should be handed in with the project.
    Change FALSE to TRUE to enable a feature.
******************************************************************/

#define         DO_DEVICE_DEBUG                 FALSE
#define         DO_MEMORY_DEBUG                 FALSE
#define         DEBUG_LOCKS                     FALSE
#define         DEBUG_CONDITION                 FALSE
#define         DEBUG_USER_THREADS              FALSE


        /*      These are Portability enhancements              */

typedef         int                             INT32;
typedef         unsigned int                   UINT32;
typedef         short                           INT16;
typedef         unsigned short                  UINT16;
typedef         int                             BOOL;

#ifdef  NT
#define THREAD_PRIORITY_LOW           THREAD_PRIORITY_BELOW_NORMAL
#define THREAD_PRIORITY_HIGH          THREAD_PRIORITY_TIME_CRITICAL
#define LOCK_TYPE                     HANDLE
// Eliminates warnings of deprecated functions with Visual C++
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef NT
#define THREAD_PRIORITY_LOW                 1
#define THREAD_PRIORITY_HIGH                2
#define LOCK_TYPE                       pthread_mutex_t
#endif
#define LESS_FAVORABLE_PRIORITY             -5
#define MORE_FAVORABLE_PRIORITY              5
#define MAX_NUMBER_OF_USER_THREADS               25
#define MAX_NUMBER_OF_PROCESSORS             (short)32

#define         FALSE                           (BOOL)0
#define         TRUE                            (BOOL)1
#define         MULTIPROCESSOR_IMPLEMENTED      TRUE


#define         PHYS_MEM_PGS                    (short)64
#define         PGSIZE                          (short)16
#define         PGBITS                          (short)4
#define         VIRTUAL_MEM_PAGES               1024
#define         VMEMPGBITS                      10
#define         MEMSIZE                         PHYS_MEM_PGS * PGSIZE



        /* Meaning of locations in a page table entry          */

#define         PTBL_VALID_BIT                  0x8000
#define         PTBL_MODIFIED_BIT               0x4000
#define         PTBL_REFERENCED_BIT             0x2000
#define         PTBL_PHYS_PG_NO                 0x0FFF

        /*  The maximum number of disks we will support:        */

#define         MAX_NUMBER_OF_DISKS             (short)8


//     These are the memory mapped IO Functions

#define      Z502Halt                  Z502Idle+1
#define      Z502Idle                  Z502InterruptDevice+1
#define      Z502InterruptDevice       Z502Clock+1
#define      Z502Clock                 Z502Timer+1
#define      Z502Timer                 Z502Disk+1
#define      Z502Disk                  Z502Context+1
#define      Z502Context               Z502Processor+1
#define      Z502Processor             Z502MEM_MAPPED_MIN+1
#define      Z502MEM_MAPPED_MIN        0x7FF00000

// These are the Memory Mapped IO Modes

#define      Z502Action                    0
#define      Z502GetInterruptInfo          1
#define      Z502ClearInterruptStatus      2
#define      Z502ReturnValue               3
#define      Z502Start                     4
#define      Z502Status                    5
#define      Z502DiskRead                  6
#define      Z502DiskWrite                 7
#define      Z502InitializeContext         8
#define      Z502StartContext              9
#define      Z502GetPageTable             10
#define      Z502GetCurrentContext        11
#define      Z502SetProcessorNumber       12
#define      Z502GetProcessorNumber       13

// This is the memory Mapped IO Data Structure.  It is an integral
// part of all Mapped IO.  It's required that this be filled in by
// the OS before making a call to the hardware.

typedef struct  {
	int          Mode;
	long         Field1;
	long         Field2;
	long         Field3;
	long         Field4;
} MEMORY_MAPPED_IO;

/*  These are the allowable locations for hardware synchronization support */

#define      MEMORY_INTERLOCK_BASE     0x7FE00000
#define      MEMORY_INTERLOCK_SIZE     0x00000100

/*  These are the device IDs that are produced when an interrupt
    or fault occurs.                                            */
        /* Definition of trap types.                            */

#define         SOFTWARE_TRAP                   (short)0

        /* Definition of fault types.                           */

#define         CPU_ERROR                       (short)1
#define         INVALID_MEMORY                  (short)2
#define         INVALID_PHYSICAL_MEMORY         (short)3
#define         PRIVILEGED_INSTRUCTION          (short)4

        /* Definition of interrupt types.                       */

#define         TIMER_INTERRUPT                 (short)4
#define         DISK_INTERRUPT                  (short)5
#define         DISK_INTERRUPT_DISK1            (short)5
#define         DISK_INTERRUPT_DISK2            (short)6

/*new defined*/
#define         DISK_INTERRUPT_DISK3            (short)7
#define         DISK_INTERRUPT_DISK4            (short)8
#define         DISK_INTERRUPT_DISK5            (short)9
#define         DISK_INTERRUPT_DISK6            (short)10
#define         DISK_INTERRUPT_DISK7            (short)11
#define         DISK_INTERRUPT_DISK8            (short)12

/*      ... we could define other explicit names here           */

#define         LARGEST_STAT_VECTOR_INDEX       DISK_INTERRUPT + \
                                                MAX_NUMBER_OF_DISKS - 1


/*      Definition of the TO_VECTOR array.  The TO_VECTOR
        contains pointers to the routines which will handle
        hardware exceptions.  The pointers are accessed withhttp://www.behr.com/consumer/products/compare-paint-sheenshttp://www.behr.com/consumer/products/compare-paint-sheenshttp://www.behr.com/consumer/products/compare-paint-sheenshttp://www.behr.com/consumer/products/compare-paint-sheens	if ( mmio.Field4 != ERR_SUCCESS ) {
		printf("In StartDisk: Took error on IO\n");
	}	if ( mmio.Field4 != ERR_SUCCESS ) {
		printf("In StartDisk: Took error on IO\n");
	}
        these indices:                                          */

#define         TO_VECTOR_INT_HANDLER_ADDR              (short)0
#define         TO_VECTOR_FAULT_HANDLER_ADDR            (short)1
#define         TO_VECTOR_TRAP_HANDLER_ADDR             (short)2
#define         TO_VECTOR_TYPES                         (short)3

        /* Definition of return codes.                           */

#define         ERR_SUCCESS                             0L
#define         ERR_BAD_PARAM                           1L
#define         ERR_NO_PREVIOUS_WRITE                   2L
#define         ERR_ILLEGAL_ADDRESS                     3L
#define         ERR_DISK_IN_USE                         4L
#define         ERR_BAD_DEVICE_ID                       5L
#define         ERR_NO_DEVICE_FOUND                     6L
#define         DEVICE_IN_USE                           7L
#define         DEVICE_FREE                             8L
#define         ERR_Z502_INTERNAL_BUG                   20L
#define         ERR_OS502_GENERATED_BUG                 21L

        // Miscellaneous

#define         NUM_LOGICAL_SECTORS                     (short)1600

#define         SUSPEND_CURRENT_CONTEXT_ONLY            (short)0
#define         START_NEW_CONTEXT_ONLY                  (short)1
#define         START_NEW_CONTEXT_AND_SUSPEND           (short)2

#define         USER_MODE                               (short)0
#define         KERNEL_MODE                             (short)1



#endif /* GLOBAL_H_ */