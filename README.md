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
* Example: Access validation to files is implemented by the security reference monitor and not the NTFS security policy.
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

#### Hardware Abstraction Layer (HAL):
* Helps providing processor architecture independence on Windows (portability) by implementing platform specific differences.
* Provides a standard interface (which does not change from hardware platform to hardware platform) which all other executive level components access system resources (Like I/O architecture, DMA operations, Firmware and BIOS interfacing, Interrupt management ...).
* Examples:
    - HAL provides to device drivers routines as - READ_PORT_UCHAR() or WRITE_PORT_UCHAR() - to allow them to read/write their devices' port without worrying about the underlying architecture.
    - Processor architectures varies widely on handling priorities of hardware logical interrupts. HAL asbtract these priorities using Interrupt Request Level (IRQL) which are a set of symbolic values ranging from the IRQL_PASSIVE_LEVEL (lowest, used by user mode applications) to IRQL_HIGH_LEVEL which is the highest possible IRQL.


#### Virtual Memory
* Is a mechanism which provides a process the ability to use more than the physical memory available on the system.
* The conversion of virtual memory to physical memory are done by a lookup table called `Page Table`.
* The precise mechanism used for implementing this lookup table is acutally quite specific to the processor hardware being used, however, Windows implemts its common set of functionality uniformly across all the hardware platforms.
* A single physical page can be shared between two separate address spaces, and can have the same VA in each address space or it can have different VA in each address space.
* Paging is simply a method of dividing up the linear address space into chunks. Pages are simply the name that we give to the chunks that result. 
* The size of these sections is referred to as the Page Size. On x86 systems, the standard page size is 4-KBytes.
* A Large Page means that the page is larger than the standard size (2MB on PAE x86 or 4MB on non-PAE x86).
* With large parges, TLB misses and page faults may be reduced, and there is a lower allocation cost (used by database applications).
* Physical Address Extension (PAE) is a processor feature that enables x86 processors to access more than 4 GB of physical memory on capable versions of Windows. Certain 32-bit versions of Windows Server running on x86-based systems can use PAE to access up to 64 GB or 128 GB of physical memory, depending on the physical address size of the processor. For details, see Memory Limits for Windows Releases.
* The technology called 4-gigabyte tuning (4GT), also known as application memory tuning, or the /3GB switch, is one of two technologies that increase the amount of physical memory available to user mode applications. 4GT increases the portion of the virtual address space that is available to a process from 2 GB to up to 3 GB.
* The Intel Itanium and x64 processor architectures can access more than 4 GB of physical memory natively and therefore do not provide the equivalent of PAE. PAE is used only by 32-bit versions of Windows running on x86-based systems.
* Because pages can be reclaimed (marked as invalid), because they have not been used recently, it is possible that when the CPU is performing V-to-P translation, it may find that there is no physical page currently allocated for the given VA. This process is known as Page Fault (#PF).
* When a PF occurs, the CPU transfers control to the registered page fault handler within the OS, in Windows, to the Memory Manager.
* If the VA is valid and the process have the permissions to access that page, the memory manager must allocate a new physical page, then ask the I/O manager to read the data from disk into that new physical page. Only then, the PF is resolved.
* Windows virtual memory implementation supports a bit in each virtual-to-physical page table entry that indicates if the page can be accessed from user mode.
* Windows provides an additional bit to control access to a page called *read only* that indicates whather the page can be written.
* The file cache consists of views into various mapped files and shares physical memory with the rest of the OS.
* Virtual memory allows memory to be shared accross multiple processe, which is the capbility of two page tables to reference to the same physical memory.
* An example of memory sharing would be sharing DLLs between two applications so that only one copy of the DLL is present in memory at any time.
* Within a virtual page, there us a bit which indicates if the page is allowed to be shared.
* There is no requirements that says, if there is two virtual references to the same physical page, the two vritual page table entries must contain the same attributes.
* Copy-On-Write is a mechanism for allowing shared memory access when appropriate. 
* That means, as long as the page is not being modified, copy-on-write operates very much like any other shared memory page.
* The only time it differs is when a process attempts to modify the content of that page, which leads to a #PF. Therefore, the memory manager after handling the exception and analyzing the cause of the exception, find out that the page has copy-on-write attribute, the memory manager copy then the current physical page into a new physical page and adjust the virtual page table entry to point to the new physical page. Both copy-on-write and read only attribute are cleared for that page.
* Most of standard windows DLLs are built so they each use a separate load address. This mechanism maximizes the speed at which apps load. Otherwise, the  windows loader have to performs relocations which then breaks the sharing of the pages as they are maked as copy-on-write.
* Reclaiming pages refers to the Memory Manager changing individual page table entries so that instead of pointing to the actual physical page, it is marked as invalid.
* Memory Trashing is the condition when in which the system spends most of its time performing I/O to disk and reclaiming pages.
* A dirty bit or modified bit is a bit that is associated with a block of computer memory and indicates whether or not the corresponding block of memory has been modified.
* How various demands to the physical memory are balanced against one another:
	- Working sets: is used to balance the use of memory between competing processes and file cache.
	- Balance Set Manager: used by the kernel to periodically call into the memory manager to reclaim physical memory from processes that no longer be using it.
	- Modified Page Writer: scans all physical pages looking for dirty pages that should be written.
	- Mapped Page Writer: accepts pages from the Modified Page Writer that are part of memory-mapped files and writes them to disk.
	- Lazy Writer: scans data by the file system and writes out any dirty pages to disk.
* Virtual Address Descriptors (VAD): the memory uses the VAD to describe the complete virtual memory layout for a given address space. This allow the memory mamanger to resolve PF quickly and correctly.
* Context: Given that there are many address spaces within the system, the context is defined by the address space that is currently in use.
* In addition to the hardware definition of a page table, the memory manager also defines special-purpose type of page table entries (PTEs).
* These `special purpose` entry types store information within the page table when the entry is not marked as valid.
* Different types of PTE exists:
	- Hardware: The precise layout of the hardware PTE is specefic to the hardware platform.
	- Prototype: This PTE is used for shared memory pages.
	- Demande Zero: This PTE indicates that the page must be zeroed before it can be used.
	- Paging File: This PTE indicates that the data contents of this page are stored in the paging file.
	- Unused:  This PTE indicates that the particular entry is available for use.
* MIPS family of CPUs does not traverse the page tables, instead they rely solely upon the Translation Lookaside Buffer, which is implemented in hardware. * * However Intel uses the TLB as a way to cache V-to-P address translation to increase the performance of this process.
* When a #PF occurs, it is actually trapped by the microkernel which builds a canonical description of the fault and then passes this into the Memory Manager:
	- Weather the fault is a load or a store operation.
	- What VA is being accessed.
	- What the CPU modeis when the fault occurs.
* VAD is as binary tree data structure which describe how the entire address space for a particular process in constructed. Each VAD entry descrives one range of pages within the address and indicates where data within the region is actually located.
* An invalid pointer is the most common reason the windows OS halts:
	- IRQL_NOT_LESS_OR_EQUAL.
	- PAGE_FAULT_IN_NON_PAGED_AREA.
	- KMODE_EXCEPTION_NOT_HANDLED.
* There is three mechanisms used by the memory manager to manage the allocation and usage of physical memory:
	- Page Frame Database: is a table describing the state of each physical memory page in the system. By tracking the state of each page (active, free, modified, zeroed, standby, bad, modifiedNoWrite, transition), the memory manager can reclaim and allocate memory as needed.
	- Page Recovery Algorithm: The memory manager handles the transition of pages thourgh several states as part of page recovery. This ensure that page recovery is both efficient and inexpensive.
	- Section object: used by the memory manager to track resources that are available to be memory-mapped into various address spaces.
* Probing: is the process of ensuring that a V-to-P mapping `is valid`.
* Locking: is the process of ensing that a valid V-to-P mapping `remains valid`. See MmProbeAndLockPages().
* Memory Descriptor Lists (MDL): is a structure used by the memory manager to describe a set of physical pages that make up the user application's virtual buffer. (See MmGetSystemAddressForMDL()).

#### The Registry
* The Registry is nothing more than a database of configuration and administrative information about the operating system and related utilities.
* Although the Registry is described as if it were a single component, it is in fact constructed by combining several independent components called `hives` into a single, coherent namespace. 
* The Registry is organized into a series of different top-level keys. Each key represents a distinct type of information. In Windows NT, the standard top-level keys are as follows:
- HKEY_CLASSES_ROOT: This key indicates special handling for various file extensions.
- HKEY_CURRENT_USER: This key indicates configuration information for the current logged-on user.
- HKEY_CURRENT_CONFIG: This key indicates miscellaneous configuration state.
- HKEY_LOCAL_MACHINE: Of interest to device driver writers, this key indicates system state.
- HKEY_USERS: This key provides local information on this machine about users.
* Registry keys may in fact be links to other keys. While reading the contents of the Registry, these links point to other parts of the Registry. For example, the HKEY_CURRENT_USER key points to the correct entry in the HKEY_USERS portion of the Registry.
* One technique you can use when managing wide strings is to maintain them by using the UNICODE_STRING structure, but ensure that there is an additional wide character at the end of the Buffer pointed to by the structure. 
* In this case, the Length field in the structure indicates the size in bytes of the string stored within the Buffer , while the MaximumLength field will indicate a size of at least two bytes more than the Length {because it requires two bytes to store a single null wide character terminator).
* For device driver developers, there are only a few keys of general interest within the Registry. These keys are located within the HKEY_LOCAL_MACHINE top-level key:
- HARDWARE:  describes the current hardware configuration, including resources that have been reserved for use by a particular device by its device driver. This key is entirely dynamic and is reconstructed each time the system boots.
- SOFTWARE: describes the configuration state and information for the various software packages installed on the system.
- SYSTEM: contains all static configuration information, and is of particular interest to device drivers because it includes the static configuration information about which drivers can be loaded on this system. The actual system startup information is maintained as a `control set`. Each control set describes the parameters to use when initializing the system, the drivers and services to load, and other information essential to proper configuration of the system as it is booted.

#### Dispatching and Scheduling
* __Dispatching__ is the way the OS switches between threadsm the units of execution on Windows NT. As such, dispatching is distinct fom the act of __scheduling__, which is the determination of the next thread to run on a given CPU.
* Typical states for threads are as follows:
	- Wait. A thread in the wait state is blocked fom running until some event (or set of events) occurs.
	- Ready. A thread in the ready state is eligible to run but must wait until NT decides to schedule it.
	- Running. A thread in the running state is presently active on some CPU in the system.
* The `_ETHREAD` structure keeps track of all threads, regardless of their state. If the thread is waiting to run because it is ready, it will be tracked via the __ready queue__, which is a kernel data structure used to track threads while they await being scheduled.
* When the thread is running, the *kernel's processor control block*, the `_KPRCB` (which is referenced fom the `PCR`) identifes which thread is active at the time, as well as two other threads-the next thread to run and the idle thread.
* When the kernel switches from one thread to another thread, it stores the current thread's context, such as the contents of various CPU registers. The kernel then loads the new context, such as those CPU registers, of the next thread to run. This is done by the routine `KiSwapThread()`.
* Another routine that is called to perform dispatching is `KiSwitchToThread()`. This function dispatches to a particular thread.
* The code within the kernel that is responsible fr dispatching control to a new thread always runs at or above IRQL __DISPATCH_LEVEL__. This is necessary because there are a number of intermediate states, such as when the registers for the threads are being restored, where it is not saf to allow for arbitrary *preemption*. Thus, we typically describe the dispatcher as running at IRQL __DISPATCH_LEVEL__.
* A __priority__ is a numeric value that indicates the relative importance of a particular thread with respect to scheduling. 
* There are actually two priority fields:
	- Priority. The value for this field is the current numeric value that will actually be used for scheduling.
	- BasePriority. The value for this field indicates the minimum value for Priority. In other words, the OS can adjust the Priority of a given thread arbitrarily, as long as it is equal to or greater than the BasePriority value for that thread.
* On Windows NT, numeric priority values range between __0__ and __31__ , although the value 0 is reserved by the OS. Thus, no threads, except specially designated OS threads, may use this priority. This range is divided into two categories: __dynamic__ priorities and __real-time__ priorities.
* Dynamic priorities are values between 1 and 15. They are referred to as "dynamic" because the OS varies the priority of threads in this range. Thus, for example, it is not possible for a thread in this range to *steal* the CPU and cause starvation of other threads that are waiting to run.
* Real-time priorities are values between 16 and 31 . They are referred to as *real-time* because the OS does not vary the priority of threads in this range. Real-time range threads can continue to control the CPU, as long as no other threads of equal or higher priority are scheduled. Thus, it is possible for a real-time thread to *steal* the CPU and cause starvation of other threads that are waiting to run.
* For either dynamic or real-time priorities, the __BasePriority__ is established when the thread is first created and may be programmatically adjusted via such calls as `KeSetBasePriorityThread()`.
* For dynamic threads, the __Priority__ starts out equal to the __BasePriority__ , but may be adjusted by the OS. For example during I/O completion `IoCompleteRequest()`, `KeSetEvent()`, Quantum exhaustion.
* For real-time threads, the OS never adjusts the Priority value, although it can be changed programmatically, such as with the call `KeSetPriorityThread()`.
* `!ready 0` displays ready threads.
* `thread`
* `!pcr` describing the current state of this processor. From the PCR, you can see the value of CurrentThread (0x8058DBE0), NextThread (0x0), and IdleThread (0x80145A80)-these values were extracted from the KPRCB via the PCR.
* Windows NT is a pre-emptive, multithreaded, and multitasking OS. It employs a traditional OS technique to provide this multitasking capability by associating a __quantum__ with each thread when it starts running. This quantum is the period of time that this particular thread will execute.
* The precise value of the quantum for a given thread depends upon the particular version and type of Windows NT system. For example, on one Windows NT v4 system, the quantum for all threads on a server system was 120 milliseconds. 
* When a thread fnishes its quantum and a new thread is scheduled to run, the thread has been __pre-empted__. A thread being pre-empted moves fom the running state to the ready state. This is different fom when a thread dispatches when a thread dispatches, it moves fom the running state to the waiting state.
* When the OS pre-empts one thread so that another thread may run, the currently running thread transitions fom the running state to the ready state. For real-time threads, the OS does not adjust the Priority value. For dynamicthreads, the OS adjusts the Priority value by decreasing it by *PriorityDecrement + 1*.