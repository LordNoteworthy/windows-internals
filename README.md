# windows-internals

##### Windows NT Key features:
* Multi-threading.
* Pre-emtive Multitasking.
* Virtual memory.
* Multiprocessing
* Processor-independent architecture.
* Multiple Operating System Emulation.

##### Windows is divided into three major groups:
* The executive: responsible for system service dispatching and managing the paged and non paged system pools. In addition to the executive module itself.
* The microkernel: processor independent, deliberately small OS module which handle routing interrupts, context switching and multiprocessor synchronization.
* The Hardware Abstraction Layer (HAL): responsible for providing a standard interface to processor-specific resources.

##### The major components within the executive-level are:
* I/O Manager
* Object Manager
* Security Reference Monitor
* Process manager
* LPC facility
* Memory manager
* Cache manager
* Win32 support (Window manager / Graphic device drivers).

##### The I/O manager:
* Responsible for managing the I/O subsystem of the OS.
* It implements a packet-based async I/O subsystem that uses IRPs to describe I/O operations.

##### The Object manager:
* Responsible for maintaining a single namespace for all named objects on the system.
* Also, responsible for creating, deleting and management of system objects (like, Processes, Threads, Sections, ...).
* Example: CreateFile() -> NtCreateFile() -> I/O Manager -> Object Manager (to parse the file path and create a file object) -> IRP to File system for parsing and validation -> Handle to our file.

##### The Security Reference Monitor
* Responsible for implementing Windows security access policy (ACLs and SIDs).
* Example: Access validation to files is implemented by the security reference monitor and not  the NTFS security policy.
* Impersonation is the ability to allow one thread to pass along to another thread the right for the second thread to use the first thread's security credentials.

##### Process Manager:
* Responsible for process and thread management. 
* It accomplishes this by working with the object manager to build process and thread objects amd with the memory manager to allocate virtual address space for the process.

##### LPC Facility
* Provides a local implementation of the RPC interprocess communication service.

##### Memory Manager and Cache Manager
* The memory manager and cache manager together form what we refer to as Windows virtual memory subsystem.
* Supports sharing of physical pages among multiple processes.


* Threads are the unit of execution and scheduling on Windows, and each thread is autonomous in terms of scheduling (The OS does not care about which progress it belongs to when making scheduling decisions.)
* Quantum indicates the max length of time the thread will be allowed to run before another thread is scheduled.
* Environment subsystem is an an independent user mode process that export a set of APIs used by applications and communicate with the system on behalf of those applications. (Implemented as client-side DLLs).

##### Major changes:
- Prior to Windows NT 4.0, all requests (except for file and device I/O) were sent to the Win32 environment subsystem.
- The Window manager, Graphics Device Interface (GDI) and display drivers were moved from user to kernel mode.

#### Microkernel
* Microkernel exports two different types of kernel objects (distinct from the Object Manager objects):
    - Dispatcher objects: used for scheduling and synchronization. (events, semaphores, mutexes and  timers ...).
    - Control objects: control specific aspects of system operation (APCs, DPCs, Interrupts).
    