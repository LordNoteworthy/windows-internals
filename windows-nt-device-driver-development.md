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
    - HAL provides to device drivers routines as - `READ_PORT_UCHAR()` or `WRITE_PORT_UCHAR()` - to allow them to read/write their devices' port without worrying about the underlying architecture.
    - Processor architectures varies widely on handling priorities of hardware logical interrupts. HAL abstract these priorities using Interrupt Request Level (IRQL) which are a set of symbolic values ranging from the IRQL_PASSIVE_LEVEL (lowest, used by user mode applications) to IRQL_HIGH_LEVEL which is the highest possible IRQL.


#### Virtual Memory

* Is a mechanism which provides a process the ability to use more than the physical memory available on the system.
* The conversion of virtual memory to physical memory are done by a lookup table called `Page Table`.
* The precise mechanism used for implementing this lookup table is actually quite specific to the processor hardware being used, however, Windows implements its common set of functionality uniformly across all the hardware platforms.
* A single physical page can be shared between two separate address spaces, and can have the same VA in each address space or it can have different VA in each address space.
* Paging is simply a method of dividing up the linear address space into chunks. Pages are simply the name that we give to the chunks that result.
* The size of these sections is referred to as the Page Size. On x86 systems, the standard page size is 4-KBytes.
* A Large Page means that the page is larger than the standard size (2MB on PAE x86 or 4MB on non-PAE x86).
* With large pages, TLB misses and page faults may be reduced, and there is a lower allocation cost (used by database applications).
* Physical Address Extension (PAE) is a processor feature that enables x86 processors to access more than 4 GB of physical memory on capable versions of Windows. Certain 32-bit versions of Windows Server running on x86-based systems can use PAE to access up to 64 GB or 128 GB of physical memory, depending on the physical address size of the processor. For details, see Memory Limits for Windows Releases.
* The technology called 4-gigabyte tuning (4GT), also known as application memory tuning, or the /3GB switch, is one of two technologies that increase the amount of physical memory available to user mode applications. 4GT increases the portion of the virtual address space that is available to a process from 2 GB to up to 3 GB.
* The Intel Itanium and x64 processor architectures can access more than 4 GB of physical memory natively and therefore do not provide the equivalent of PAE. PAE is used only by 32-bit versions of Windows running on x86-based systems.
* Because pages can be __reclaimed__ (marked as invalid), because they have not been used recently, it is possible that when the CPU is performing V-to-P translation, it may find that there is no physical page currently allocated for the given VA. This process is known as Page Fault (#PF).
* When a PF occurs, the CPU transfers control to the registered page fault handler within the OS, in Windows, to the Memory Manager.
* If the VA is valid and the process have the permissions to access that page, the memory manager must allocate a new physical page, then ask the I/O manager to read the data from disk into that new physical page. Only then, the PF is resolved.
* Windows virtual memory implementation supports a bit in each virtual-to-physical page table entry that indicates if the page can be accessed from user mode.
* Windows provides an additional bit to control access to a page called *read only* that indicates weather the page can be written.
* The file cache consists of views into various mapped files and shares physical memory with the rest of the OS.
* Virtual memory allows memory to be shared across multiple process, which is the capability of two page tables to reference to the same physical memory.
* An example of memory sharing would be sharing DLLs between two applications so that only one copy of the DLL is present in memory at any time.
* Within a virtual page, there us a bit which indicates if the page is allowed to be shared.
* There is no requirements that says, if there is two virtual references to the same physical page, the two virtual page table entries must contain the same attributes.
* Copy-On-Write is a mechanism for allowing shared memory access when appropriate.
* That means, as long as the page is not being modified, copy-on-write operates very much like any other shared memory page.
* The only time it differs is when a process attempts to modify the content of that page, which leads to a #PF. Therefore, the memory manager after handling the exception and analyzing the cause of the exception, find out that the page has copy-on-write attribute, the memory manager copy then the current physical page into a new physical page and adjust the virtual page table entry to point to the new physical page. Both copy-on-write and read only attribute are cleared for that page.
* Most of standard windows DLLs are built so they each use a separate load address. This mechanism maximizes the speed at which apps load. Otherwise, the  windows loader have to performs relocations which then breaks the sharing of the pages as they are maked as copy-on-write.
* __Reclaiming pages__ refers to the Memory Manager changing individual page table entries so that instead of pointing to the actual physical page, it is marked as __invalid__.
* __Memory Trashing__ is the condition when in which the system spends most of its time performing I/O to disk and reclaiming pages.
* A __dirty bit__ or modified bit is a bit that is associated with a block of computer memory and indicates whether or not the corresponding block of memory has been modified.
* How various demands to the physical memory are balanced against one another:
	- __Working sets__: is used to balance the use of memory between competing processes and file cache.
	- __Balance Set Manager__: used by the kernel to periodically call into the memory manager to reclaim physical memory from processes that no longer be using it.
	- __Modified Page Writer__: scans all physical pages looking for dirty pages that should be written.
	- __Mapped Page Writer__: accepts pages from the Modified Page Writer that are part of memory-mapped files and writes them to disk.
	- __Lazy Writer__: scans data by the file system and writes out any dirty pages to disk.
* __Virtual Address Descriptors (VAD)__: the memory uses the VAD to describe the complete virtual memory layout for a given address space. This allow the memory mamanger to resolve PF quickly and correctly.
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
* Memory Descriptor Lists (MDL): is a structure used by the memory manager to describe a set of physical pages that make up the user application's virtual buffer. (See `MmGetSystemAddressForMDL()`).

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
* In this case, the Length field in the structure indicates the size in bytes of the string stored within the Buffer , while the MaximumLength field will indicate a size of at least two bytes more than the Length (because it requires two bytes to store a single null wide character terminator).
* For device driver developers, there are only a few keys of general interest within the Registry. These keys are located within the HKEY_LOCAL_MACHINE top-level key:
- HARDWARE:  describes the current hardware configuration, including resources that have been reserved for use by a particular device by its device driver. This key is entirely dynamic and is reconstructed each time the system boots.
- SOFTWARE: describes the configuration state and information for the various software packages installed on the system.
- SYSTEM: contains all static configuration information, and is of particular interest to device drivers because it includes the static configuration information about which drivers can be loaded on this system. The actual system startup information is maintained as a `control set`. Each control set describes the parameters to use when initializing the system, the drivers and services to load, and other information essential to proper configuration of the system as it is booted.

#### Dispatching and Scheduling
* __Dispatching__ is the way the OS switches between threads, the units of execution on Windows NT. As such, dispatching is distinct fom the act of __scheduling__, which is the determination of the next thread to run on a given CPU.
* Typical states for threads are as follows:
	- Wait. A thread in the wait state is blocked fom running until some event (or set of events) occurs.
	- Ready. A thread in the ready state is eligible to run but must wait until NT decides to schedule it.
	- Running. A thread in the running state is presently active on some CPU in the system.
* The `_ETHREAD` structure keeps track of all threads, regardless of their state. If the thread is waiting to run because it is ready, it will be tracked via the __ready queue__, which is a kernel data structure used to track threads while they await being scheduled.
* When the thread is running, the *kernel's processor control block*, the `_KPRCB` (which is referenced fom the `PCR`) identifes which thread is active at the time, as well as two other threads-the next thread to run and the idle thread.
* When the kernel switches from one thread to another thread, it stores the current thread's context, such as the contents of various CPU registers. The kernel then loads the new context, such as those CPU registers, of the next thread to run. This is done by the routine `KiSwapThread()`.
* Another routine that is called to perform dispatching is `KiSwitchToThread()`. This function dispatches to a particular thread.
* The code within the kernel that is responsible for dispatching control to a new thread always runs at or above IRQL __DISPATCH_LEVEL__. This is necessary because there are a number of intermediate states, such as when the registers for the threads are being restored, where it is not safe to allow for arbitrary *preemption*. Thus, we typically describe the dispatcher as running at IRQL __DISPATCH_LEVEL__.
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
* When the OS pre-empts one thread so that another thread may run, the currently running thread transitions fom the running state to the ready state. For real-time threads, the OS does not adjust the Priority value. For dynamicthreads, the OS adjusts the Priority value by decreasing it by *PriorityDecrement+1*.

#### Interrupt Request Levels and DPCs
* IRQLs are the chief method used for __prioritizing__ OS activities within Windows NT.
* The relative priority of an activity within the Windows NT operating system is defined by its __Interrupt Request Level (IRQL)__.
* The current processor's IRQL indicates the relative priority of the activity currently taking place on that CPU.
* IRQL values are assigned to both sofware and hardware activities, with sofware IRQLs being lower than hardware IRQLs.
* If an event occurs on a given processor that has a higher IRQL than the processor's current IRQL, the higher-priority event will interrupt the lower-priority event.
*  If an event with an IRQL lower than the processor's current IRQL occurs on that CPU, processing of that event waits until all other events at higher IRQLs have been processed.
* Thus, the processor's current IRQL functions as an interrupt mask, deferring (masking) those activities requested at the same or lower IRQLs than the processor's current IRQL.
* The lower-level IRQLs (IRQLs __PASSIVE_LEVEL__ through __DISPATCH_LEVEL__) are used internally for synchronization of the OS software. These IRQLs are modeled as __software interrupts__. IRQLs above __DISPATCH_LEVEL__, whether they have a specific mnemonic or not, reflect __hardware-interrupt__ priorities. Thus, these hardware IRQLs are often referred to as Device IRQLs (or __DIRQLs__).
* A very important point to understand is that IRQLs are not the same as Windows NT process-scheduling priorities.
* Scheduling priorities are artifcts of the Windows NT Dispatcher, which uses them to determine which thread to next make active. IRQLs, on the other hand, are best thought of as *interrupt priorities* used by the OS. An interrupt at any IRQL above __PASSIVE_LEVEL__ will interrupt even the highest-priority User mode thread in the system.
* The current IRQL is tracked on a per-CPU basis. A Kernel mode routine can determine the IRQL at which it is running by calling the function `KeGetCurrentIrql()`.
* Kernel mode routines may change the IRQL at which they are executing by calling the functions `KeRaiseIrql()` and `KeLowerIrql()`.
* Because IRQLs are a method of synchronization, most Kernel mode routines (specifically, device drivers) must __never__ lower their IRQL beyond that at which they were called.
* How the most common IRQLs are used within Windows NT:
	- IRQL PASSIVE_LEVEL is the ordinary IRQL of execution in the OS, both in User mode and Kernel mode. A routine running at IRQL PASSIVE_LEVEL is subject to interruption and pre-emption by almost anything else happening in the system.
	- IRQL APC_LEVEL is used by Kernel mode routines to control re-entrancy when processing Asynchronous Procedure Calls (APCs) in Kernel mode. Exampe: IO completion.
	- IRQL DISPATCH_LEVEL is used within Windows NT for two different activities:
		- Processing Deferred Procedure Calls (DPCs)
		- Running the Dispatcher (NT's scheduler): The Windows NT Dispatcher receives requests to perform a reschedule operation at IRQL DISPATCH_LEVEL.
* When code is executing at IRQL __DISPATCH_LEVEL__ or above, it cannot wait for any Dispatcher Objects that are not already signaled. Thus, for example, code running at IRQL DISPATCH_LEVEL or above cannot wait for an event or mutex to be set.
* When code running at IRQL __DISPATCH_LEVEL__ or above may not take any __page faults__. This means that any such code must itself be __non-paged__, and must touch only data structures that are non-paged.
* A vitally important point about __DIRQLs__ is that these IRQLs do not necessarily preserve the relative priorities that may be implied by a given bus's external interrupt signaling method. For example, the HAL has complete discretion in terms of how it maps IRQs (bus Interrupt ReQuest lines) to IRQLs.
* The relationship between two IRQs assigned to two particular devices is not necessarily *preserved* when IRQLs are assigned to those devices. Whether a device with a more important IRQ is assigned a higher (that is, more important) IRQL is totally up to the HAL. Indeed, in most standard x86 multiprocessor HALs for systems that use APIC architectures, the reltionship of IRQ to IRQL is not preserved.
* IRQL __HIGH_LEVEL__ is always defined as the highest IRQL on a Windows NT system. This IRQL is used for __NMI (Non-Maskable Interrupt)__ and other interrupts of very high priority.
* In the exceedingly rare case in which a device driver needs to disable interrupts on a particular processor for a short period, the driver may raise its IRQL to HIGH_LEVEL. However, a device driver raising to IRQL HIGH_LEVEL is considered a very drastic step, and it is almost never required in Windows NT.
* In addition to its use for running the NT Dispatcher, IRQL DISPATCH_LEVEL is also used for processing __Defrred Procedure Calls (DPCs)__.
* DPCs are callbacks to routines to be run at IRQL DISPATCH_LEVEL. DPCs are typically requested from higher IRQLs to allow more extended, non-time-critical, processing to take place.
* Use cases of DPC:
	- Windows NT device drivers perform very little processing within their ISR. Instead, when a device interrupts (at DIRQL) and its driver determines that a significant amount of processing is required, the driver requests a DPC. The DPC request results in a specified driver function being called back at IRQL DISPATCH_LEVEL to perform the remainder of the required processing. By performing this processing at IRQL DISPATCH_LEVEL, the driver takes less time at DIRQL, and therefore decreases interrupt latency for all the other devices on the system.
	- Another common use for DPCs is in timer routines. A driver may request to have a particular function be called to notify it that a certain period of time has elapsed (this is done using the `KeSetTimer()` function).
* A DPC is described by a DPC Object `_KDPC`.
* A DPC Object may be allocated by a driver fom any non-pageable space (such as nonpaged pool). DPC objects are initialized by using the function `KeInitializeDpc()`.
* A request to execute a particular DPC routine is made by placing the DPC Object that describes that DPC routine into the DPC Queue of a given CPU, and then requesting an IRQL DISPATCH_LEVEL software interrupt (this is done using `KeInsertQueueDpc()` function).
* As noted earlier in the chapter, IRQL DISPATCH_LEVEL is used both for dispatching and for processing the DPC Queue. In NT V4, when a DISPATCH_LEVEL interrupt is processed, the entire DPC Queue is __serviced first__ (by the microkernel), and then the Dispatcher is called to schedule the next thread to run. This is reasonable because the processing done within a DPC routine could change to alter the state of the thread scheduling database, for example, by making a previously waiting thread runnable.
* A single DPC routine may be actively executing on multiple processors at the same time. There is absolutely no interlocking performed by the Microkernel to prevent this hence the importance of utilizing the proper set of multiprocessor synchronization mechanisms in drivers. Specifically, __spin locks__ must be used to serialize access to any data structures that must be accessed atomically within the driver's DPC if the driver's design is such that multiple DPCs can be in progress simultaneously.
* Each DPC Object has an __importance__, which is stored in the DPC Object's Importance feld. The importance of a DPC Object affects where in the DPC Queue the DPC Object is placed when it is queued, and whether or not an IRQL DISPATCH_LEVEL interrupt is issued when the DPC Object is queued.
* `KeInitializeDpc()` initializes DPC Objects with __Medium__ importance. The Importance of a DPC Object can be set by using the function `KeSetImportanceDpc()`.
* In addition to an importance, each DPC Object has a *target processor*. This target processor is stored in the `Number` field of the DPC Object. The target processor indicates whether or not a DPC is restricted to execute on a given processor on the system, and, if so, on which processor.
* By default, `KeInitializeDpc()` does not specify a target processor. Consequently, by default, DPCs will run on the processor on which they were requested (that is, the DPC will be invoked on the processor on which `KeInsertQueueDpc()` was called).
* A DPC may be restricted to executing on a specific processor using the `KeSetTargetProcessorDpc()` function.
* To make it easy for device drivers to request DPCs for ISR completion fom their ISRs, the IO Manager defines a specific DPC that may be used for this purpose. This DPC is called the __DpcForIsr__.
* Because all device drivers have Device Objects, and all drivers that utilize interrupts also utilize DPCs, using the IO Manager's DpcForIsr mechanism is very convenient. In fact, most device drivers in Windows NT never directly call `KeInitializeDpc()` or `KeInsertQueueDpc()` , but call `IoInitializeDpcRequest()` and
`IoRequestDpc()` instead.


#### Multiprocessor Issues
* Because Windows NT supports multiprocessing, all Kernel mode code must be multiprocessor-safe. Multiprocessor safety involves maintaining cache coherency among processors, virtual memory issues, and even interrupt handling. However, driver writers must be __careful__ to properly synchronize access to shared data structures.
* An appropriate Dispatcher Object, such as a __mutex__, can be used for synchronization to guard the shared data structure. This works fine, as long as all the threads that modify the data being shared execute only at __IRQL PASSIVE_LEVEL__ or __IRQL APC_LEVEL__. Thus, using a mutex would be a perfect solution for synchronizing access to data that is shared between two user threads because Usermode threads always execute at IRQL PASSIVE_LEVEL.
* However, using a Dispatcher Object such as a mutex would not be possible if any thread that modifies the shared data is running at IRQL DISPATCH_LEVEL or above.
* This is due to the fact that running at IRQL DISPATCH_LEVEL or higher blocks recognition of the DISPTACH_LEVEL interrupt that is used to trigger the Dispatcher. Thus, it is impossible for a thread running at IRQL DISPATCH_LEVEL or above to yield control of the processor to wait, in case the Dispatcher Object is not available.
* Fortunately, there is a simple solution to sharing data when one or more of the modifying threads may be running at IRQL DISPATCH_LEVEL or above. The solution is to use a __spin locks__.
* Spin locks are standard Windows NT data structures, located in nonpageable memory. Every spin lock has an IRQL implicitly associated with it. That IRQL is at least IRQL DISPATCH_LEVEL, and it is the highest IRQL fom which the lock may ever be acquired.
* The reason spin locks have their name is that if the lock is not available, the thread that attempts to acquire the lock simply spins (or _busy waits_ as it is often called), repeatedly trying to acquire the lock until the lock is free. Of course, because this spinning occurs at IRQL DISPATCH_LEVEL or above, the processor on which the lock is being acquired is not dispatchable. Thus, even when the currently executing thread's quantum expires, the thread will continue running.
* There are two kinds of spin locks: __Executive Spin Locks__ and __Interrupt Spin Locks__.
* Executive Spin Locks are the type of spin lock most frequently used in an NT device driver. They are defined as data structure of type __KSPIN_LOCK__.
* Executive Spin Locks operate at IRQL DISPATCH_LEVEL, and they are allocated from nonpaged pool. Then, they are initalized using `KeInitializeSpinLock()`.
* Executive Spin Locks may be acquired by callers running at less than or equal to IRQL DISPATCH_LEVEL by calling `KeAcquireSpinlock()`.
* Windows NT provides an optimized version of `KeAcquireSpinLock()` for use when the caller is already running at DISPATCH_LEVEL. This function is called `KeAcquireSpinLockAtDpcLevel()`.
* Executive Spin Locks that were acquired with `KeAcquireSpinLock()` must be released using the function `KeReleaseSpinLock()`.
* Executive Spin Locks that were acquired with the function `KeAcquireSpinLockAtDpcLevel()` must be released using the function `KeReleaseSpinLockFromDpcLevel()`.
* Interrupt Spin Locks, which are sometimes referred to as ISR spin locks, are the rarer of the two types of spin locks on Windows NT.
* Interrupt Spin Locks operate at a DIRQL, specifically the _SynchronizeIrql_ that is specified when a driver calls `IoConnectInterrupt()`.
* The Interrupt Spin Lock for a particular interrupt service routine is always acquired by the Microkernel prior to its calling the interrupt service routine.
* Driver routines other than the interrupt service routine may acquire a particular Interrupt Spin Lock by calling `KeSynchronizeExecution()`.

#### The IO Manager
* The major design characteristics of the Windows NT I/O Subsystem:
	- Consistent and highly structured
	- Portable across processor architectures
	- Configurable
	- As frequently pre-emptible and interruptible as possible
	- Multiprocessor safe on MP systems
	- Object-based
	- Asynchronous
	- Packet-driven
	- Layered
* The Windows NT I/O Subsystem is based on a collection of "objects." These objects are defined by the Microkernel, HAL, and the I/O Manager and exported to other Kernel mode modules, including device drivers.
* The NT operating system in general, and the I/O Subsystem in particular, is _object-based_, but not necessarily _object-oriented_.
*  Most objects used within the I/O Subsystem are considered __partially opaque__. This means that a subset of fields within the object can be directly manipulated by kernel modules, including drivers. Examples of partially opaque objects include Device Objects and Driver Objects.
* A few objects used within the I/O Subsystem (such as DPC Objects or Interrupt Objects) are considered __fully opaque__. This means that Kernel mode modules (other than the creating module) must call functions that understand and manipulate the fields within the object.
* Structures commonly used within the I/O subsystem: <p align="center"><img src="https://i.snag.gy/p6OI49.jpg"></p>
* The File Object is defined by the NT structure `FILE_OBJECT`. A File Object represents a single open instance of a file, device, directory, socket, named pipe, mail slot, or other similar entity.
* The Driver Object describes where a driver is loaded in physical memory, the driver's size, and its main entry points. The format of a Driver Object is defined by the NT structure `DRIVER_OBJECT`.
* The Device Object represents a physical or logical device that can be the target of an I/O operation. The format of the Device Object is defined by the NT structure `DEVICE_OBJECT`.
* While a Device Object may be created at any time by a driver by calling `IoCreateDevice()` , Device Objects are normally created when a driver is first loaded.
* When a driver creates a Device Object, it also specifies the size of the __Device Extension__ to be created.
* The Device Extension is a per-device area that is private to the device driver. The driver can use this area to store anything it wants, including device statistics, queues of requests, or other such data. The Device Extension is typically the main global data storage area.
* Both the Device Object and Device Extension are created in non-paged pool.
* The Interrupt Object is created by the I/O Manager, and is used to connect a driver's interrupt service routine to a given interrupt vector. The structure, `KINTERRUPT`, is one of the few fully opaque structures used in the I/O Subsystem.
* The Adapter Object is used by all OMA drivers. It contains a description of the DMA device, and represents a set of shared resources. These resources may be a DMA channel or a set of DMA map registers.

### I/O Architectures
* Devices utilize different mechanisms to move data between the device and host memory. Windows NT places devices, and hence their drivers, into one of three major categories depending on their capabilities:
* __Programmed I/O__: (PIO) devices are usually the simplest of the three main categories of devices. The driver for a PIO device is responsible for moving the
	data between host memory and the device under program control. This characteristic is, in fact, what gives this category its name.
	- A driver may access a PIO device via shared memory, memory space registers, or port I/O space registers. How the device is accessed depends on the device's design.
	- _Accessing a shared memory buffer on a PIO device_: During initialization, the driver maps the device's memory buffer into kernel virtual address space via calls to the Memory Manager. To transfer data to the device, the driver moves the data under program control to a location within the device's memory buffer. The move would most likely be performed by using the HAL function `WRITE_REGISTER_BUFFER_ULONG ()`.
	- _Accessing a memory space register on a PIO device_: During initialization, the driver will map the physical addresses that the device's memory space registers occupy into kernel virtual address space. The driver then accesses the device using the HAL routines `READ_REGISTER_ULONG ()` and `WRITE_REGISTER_ULONG ()`.
	- _Accessing a port I/O space register on a PIO device_: The interface to the device in question is via a longword register in port I/O space. Because this register is not in memory space, the driver does not need to (and, in fact, cannot) map the register into kernel virtual address space. Instead, to access the port I/O space register, the driver uses the HAL `READ_PORT_ULONG ()` and `WRITE_PORT_ULONG ()` functions.
	- Although each of the three aforementioned devices is accessed in a slightly different way, they all share one common attribute. To get data to the device or retrieve data from the device, the driver is required to __manually__ move the data under program control. This data movement consumes CPU cycles. So, while the driver is moving data between a requestor's buffer and a peripheral, the CPU is not being used to do other useful work, like processing a user's spreadsheet. This is the primary disadvantage of a PIO device.
* __Busmaster DMA Devices__: The single characteristic that these devices have in common is that a Busmaster DMA device __autonomously__ transfers data between itself and host memory.
	- The device driver for a Busmaster DMA device gives the device the starting logical address of the data to be transferred, plus the length and direction of the transfer; and the device moves the data itself without help from the host CPU.
	- Windows NT categorizes Busmaster DMA devices as being one of two types. The specific functioning of a device's hardware determines into which category a given device falls. The two categories are:
	- __Packet-Based DMA devices__ which are the most common type of Busmaster DMA device. Packet-Based DMA devices typically transfer data to/from __different logical addresses__ for each transfer, and __Common-Buffer DMA devices__ which typically utilize __the same buffer__ for all transfer operations. Many network interface cards are Common-Buffer DMA devices.
	- __Packet-Based DMA devices__ which are the most common type of Busmaster DMA device. Packet-Based DMA devices typically transfer data to/from __different logical addresses__ for each transfer, and __Common-Buffer DMA devices__ which typically utilize __the same buffer__ for all transfer operations. Many network interface cards are Common-Buffer DMA devices.
* DMA operations from devices on a Windows NT system are performed to __logical addresses__. These logical addresses are managed by the HAL, and correlate to physical host memory addresses in a hardware-specific and HAL-specific manner.
* Logical addresses are translated to host memory physical addresses by the HAL through the use of _map registers_.
* A device bus has a logical address space, managed by the HAL, which is different from the physical address space used for host memory. When processing a DMA transfer request, a device driver calls the I/O Manager and HAL (using the function `IoMapTransfer ()`) to allocate a set of map registers, and program them appropriately to perform the DMA data transfer.
* s a logical address space, managed by the HAL, which is different from the physical address space used for host memory. When processing a DMA transfer request, a device driver calls the I/O Manager and HAL (using the function `IoMapTransfer ()`) to allocate a set of map registers, and program them appropriately to perform the DMA data transfer.
* It is important to understand that map registers are part of the HAL's standard abstraction of system facilities. How the logical addresses used in DMA operations are implemented, including how these logical addresses are translated to physical addresses and thus even how map registers themselves are implemented, is entirely a function of how a particular HAL is implemented on a given platform.
 * Because Windows NT uses virtual memory, the physical memory pages that comprise a requestor's data buffer need not be contiguous in host memory.
* Simple DMA devices are capable of transferring data by using only a __single logical base address and length pair__. Therefore, drivers for such devices must reprogram the device for each logical buffer fragment in the requestor's buffer. This can require both _extra overhead and latency_.
* More-sophisticated DMA devices support an optional feature called __scatter/gather__. This feature, also known as _DMA chaining_ allows the device to be programmed with multiple pairs of base addresses and lengths simultaneously. Thus, even a logically fragmented requestor's buffer can be described to the DMA device by its driver in one operation.
* To help reduce the overhead required to support devices that do not implement scatter/gather, the HAL implements a facility known as __system scatter/gather__. To implement this feature, the HAL utilizes its map registers to create a single, contiguous, logical address range that maps to the requestor's noncontiguous buffer in physical memory. This contiguous logical address range can then be addressed by a device that does not support scatter/gather with a single logical base address and length.
* __System DMA Devices__ provides the capability for a device on the system to use a common DMA controller to perform transfers between itself and host memory.
	- This capability results in a device that is inexpensive (like a PIO device), but that can move data without using host CPU cycles (like Busmaster DMA).
	- System DMA, as it is supported in Windows NT, is very much like Busmaster DMA, with the following exceptions:
		- System DMA devices share a DMA controller that is provided as part of the system, whereas Busmaster DMA devices have a dedicated DMA controller built into their devices.
		- System DMA devices do not support scatter/gather.
		- The HAL programs the System DMA controller; the device then utilizes the functionality of the System DMA controller to transfer data between itself and host memory.

#### How I/O Requests Are Described
* Windows NT describes I/O requests by using a packet-based architecture. In this approach, each I/O request to be performed can be described by using a single __I/O Request Packet__ (IRP).
* When an I/O system service is issued (such as a request to create or read froma file), the I/O Manager services that request by building an IRP describing the request, and then passes a pointer to that IRP to a device driver to begin processing the request.
*  The IRP is allocated fom nonpaged space, using either a preallocated IRP in one of the I/O Manager's lookaside lists, or by allocating the IRP directly fom nonpaged pool.
* An IRP contains all the information necessary to fully describe an I/O request to the I/O Manager and device drivers. The IRP is a standard NT structure of type "IRP.": <p align="center"><img src="https://i.snag.gy/cnjJym.jpg"  width="300px" height="auto"></p>
* As you can see in the figure above, each I/O Request Packet may be thought of as having two parts: A __fixed__ part and an I/O Stack.
* The fixed part of the IRP contains information about the request that either does not vary from driver to driver, or it does not need to be preserved when the IRP is passed from one driver to another.
* The I/O Stack contains a set of __I/O Stack locations__, each of which holds information specific to each driver that may handle the request.
* Each I/O Stack location in an IRP contains information for a specific driver about the I/O request. The I/O Stack location is defined by the structure `IO_STACK_LOCATION`.
* To locate the current I/O Stack location within a given IRP, a driver calls the function `IoGetCurrentIrpStackLocation()`.
* The I/O Manager initializes the fixed portion of the IRP (in the format indicated by the driver) with the description of the requestor's buffer. The I/O Manager then initializes the first I/O Stack location in the IRP with the fnction codes and parameters for this request. The I/O Manager then calls the first driver in the driver "stack" to begin processing the request.
* Windows NT provides driver writers with the following three different options for describing the requestor's data buffer associated with an I/O operation:
	- __Direct I/O__: The buffer may be described in its original location in the requestor's physical address space by a structure called a Memory Descriptor List (MDL), which describes the physical addresses of the requestor's user mode virtual addresses.
	- __Buffered I/O__: The data from the requestor's buffer may be copied from the requestor's address space into an intermediate location in system address space (by the I/O Manager before the driver gets the IRP), and the driver is provided a pointer to this copy of the data.
	- __Neither I/O__: The driver is provided with the requestor's virtual address of the buffer.
* _Describing Data Buffers with Direct I/O_:
	- If a driver chooses Direct I/O, any data buffer associated with read or write I/O requests will be described by the I/O Manager by using an opaque structure called a Memory Descriptor List (MDL).
	- MDL is capable of describing a single data buffer that is contiguous in virtual memory, but is not necessarily physically contiguous.
	- An MDL is designed to make it particularly fast and easy to get the physical base addresses and lengths of the fragments that comprise the data buffer: <p align="center"><img src="https://i.snag.gy/9OXvp4.jpg"  width="400px" height="auto"></p>
	- The I/O and Memory Managers provide functions for getting information about a data buffer using an MDL as `MmGetSystemAddressForMdl()`, `MmMapLockedPages()`, `MmGetMdlVirtualAddress`, `MmGetMdlByteCount` and `MmGetMdlByteOffset`.
* _Describing Data Buﬀers with Buffered I/O_:
	- In this scheme, an intermediate buffer in system space is used as the data buffer. The I/O Manager is responsible for moving the data between the intermediate buffer and the requestor's original data buffer: <p align="center"><img src="https://i.snag.gy/LnWbZo.jpg"  width="400px" height="auto"></p>.
	- To prepare a Buffered  request, the I/O Manager checks to ensure that the caller has appropriate access to the entire length of the data buffer, just as it did for Direct I/O.
	- The  Manager next allocates a system buﬀer fom the nonpaged pool with a size that is (at least) equal to that of the data buffer.
	- Because the address of the intermediate buffer corresponds to a location in the system's nonpaged pool, the address is usable by the driver in an arbitrary thread context.
	- Buﬀered I/O is most ofen used by drivers controlling programmed I/O devices that use small data transfers. In this case, it is usually very convenient to have a requestor's data described by using a system virtual address.
* _Describing Data Buffers with Neither I/O_:
	- This option is called Neither I/O because the driver does not request either Buffered I/O or Direct I/O.
	- In this scheme, the I/O Manager provides the driver with the __requestor's virtual address__ of the data buﬀer. The buffer is not locked into memory, no intermediate buffering of the data takes place.
	- Obviously, the requestor's virtual address is only useful in the context of the calling process. As a result, the only drivers that can make use of Neither I/O are drivers that are entered directly fom the I/O Manager, with no drivers above them, and can process (and, typically, complete) the I/O operation in the context of the calling process.
	- Most typical device drivers cannot use Neither I/O because the I/O requests in these drivers are ofen started fom their __DpcForlsr__ routine, and are thus called in an arbitrary thread context.
* If you are writing an Intermediate driver that will be layered above another driver, you must use the same buffering method that the device below you uses.
* Drivers that transfer at least a page of data or more at a time usually perform best when they use Direct I/O. Although the I/O Manager locks the pages in memory for the duration of the transfer, Direct I/O avoids the overhead of recopying the data to an intermediate buffer. Using Direct I/O for large transfers also prevents tying up large amounts of system pool.
* Most DMA drivers want to use Direct I/O. Drivers for packet-based DMA devices want to use it because this allows them to easily get the physical base address and length of the fragments that comprise the data buﬀer. Drivers for "common buffer" OMA devices want to use it to avoid the overhead of an additional copy operation.
* Characteristics of Direct I/O, Buffered IO, and Neither IO: <p align="center"><img src="https://i.imgur.com/MonNsxh.png"  width="500px" height="auto"></p>
* Windows NT uses I/O function codes to identify the specific I/O operation that will take place on a particular file object. Like most operating systems,
* Windows NT I/O function codes are divided into major and minor I/O functions. Both appear in the IRP in the driver's I/O Stack location. Major function codes are defined with symbols starting __IRP_MJ__ . Some of the more common major I/O function codes include the following:
	- `IRP_MJ_CREATE`: creates a new file object by accessing an existing device or file, or by creating a new file =>  `CreateFile ()`.
	- `IRP_MJ_CLOSE`: closes a previously opened file object => `CloseHandle()`.
	- `IRP_MJ_READ`: performs a read operation on an existing file object => `ReadFile()`.
	- `IRP_MJ_WRITE`: performs a write operation on an existing file object => `WriteFile()`.
	- `IRP_MJ_DEVICE_CONTROL`: performs a driver defined function on an existing file object => `DeviceIoControl()`.
	- `IRP_MJ_INTERNAL_DEVICE_CONTROL`: as same the one before, except that yhere are no user-level APis that correspond with this function. This function is typically used for inter-driver communication purposes.
* Minor IO function codes in Windows N are defined with symbols that start with `IRP_MN_`. Windows NT mostly avoids using minor function codes to overload major functions for device drivers, fvoring instead the use of IO Control codes. For example, one file system-specific minor IO function code is `IRP_MN_COMPRESSED`, indicating that the data should be written to the volume in compressed format.
* The major and minor IO function codes associated with a particular IRP are stored in the MajorFunction and MinorFunction fields of the current IO Stack location in the IRP.
	```
	IoStack = IoGetCurrentirpStacklocation (Irp) ;
	If ( IoStack->MajorFunction == IRP_MJ_READ ) {
		If ( IoStack->MinorFunction ! = IRP_MN_NORMAL {
			// do something
		}
	}
	```
* Windows NT provides a macro that defines custom control codes, saving us fom having to manually pack bits into the I/O Control Code longword. This macro is named, appropriately, `CTL_CODE`:
	```
	CTL_CODE (DeviceType , Function, Method, Access)
	```
* The __DeviceType__ argument for the CTL_CODE macro is a value (of type DEVICE_TYPE) that indicates the category of device to which a given I/O control code belongs. Standard NT devices have standard N device types (FILE_DEVICE_DISK for disk drives, FILE_DEVICE_TAPE for tapes, and so on) that are defned in the same .H files as the CTL_CODE macro. Custom device types; for devices such as our toaster that don't correspond to any standard NT device, may be chosen from the range of 32768-65535. These values are reserved for use by Microsof customers.
* The __Function__ argument to the CTL_CODE macro is a value, unique within your driver, which is associated with a particular function to be performed. For example, we would need to choose a particular function code that represents the "set toast brownness level" function implemented by our toaster driver. Custom function codes may be chosen from the range of values between 2048 (0x800)-> 4095.
* The __Method__ argument indicates to the I/O Manager how the data buﬀers supplied with this request are to be described (METHOD_BUFFERED, METHOD_IN_DIRECT and METHOD_OUT_DIRECT, METHOD_NEITHER).
* The __Access__ argument to the CTL_CODE macro indicates the type of access that must have been requested (and granted) when the file object was opened for a given I/O control code to be passed on to the driver by the I/O Manager. The possible values for this argument: FILE_ANY_ACCESS, FILE_READ_ACCESS, FILE_WRITE_ACCESS.
* To retrieve the IOCTL code:
	```
	Code = IoStack->Parameters.DeviceloControl.IoControlCode ;
	```
<p align="center"><img src="https://i.imgur.com/VAqyZHA.png"  width="600px" height="auto"></p>

### The Layered Driver Model
* At the highest level, the types of drivers may be divided into two categories: User mode drivers and Kernel mode drivers.
* User mode drivers ofen provide a subsystem-specific interface to a standard Kernel mode driver. In the Win32 Environment Subsystem, User mode drivers are implemented as Dynamic Linked Libraries (DLLs).
* As an example, most Audio Compression Manager (ACM) drivers, which implement audio compression algorithms, are User mode, sofware-only drivers. On the other hand, Multimedia Control Interface (MCI) drivers are User mode drivers that typically'interact with underlying hardware through the use of a collaborating Kernel mode driver. <p align="center"><img src="https://i.imgur.com/bT2MmO2.png"  width="400px" height="auto"></p>
* Kernel mode drivers form part of the Windows NT Executive layer and run in Kernel mode, as their name implies. Kernel mode drivers are accessed and supported by the I/O Manager.
* The four types of Kernel mode drivers are as follows:
	- File System drivers
	- Intermediate drivers
	- Device drivers
	- Mini-drivers. <p align="center"><img src="https://i.imgur.com/crRu51r.png"  width="400px" height="auto"></p>
* __File System drivers__ exist at the top of the NT Kernel mode driver stack. File System drivers play a special role in Windows NT because they are tightly coupled with the NT Memory and Cache Manager subsystems.
* File System drivers may implement a physical fle system, such as NTFS or FAT; however, they may also implement a distributed or networked facility.
* __Intermediate drivers__ form the middle layer of the NT driver hierarchy, sitting below File System drivers and above Device drivers.
* Intermediate drivers provide either a "value-added" feature (such as mirroring or disk-level encryption) or class processing for devices. In either case, Intermediate drivers rely upon the Device drivers below them in the NT driver hierarchy for access to a physical device.
* The most common type of Intermediate driver is the _Class driver_. A Class driver typically performs processing for a category of device, having common attributes, which is physically accessed via a separately addressable shared bus. For example, the Disk Class driver performs processing for disk-type devices that are located on a SCSI bus.
* __Device drivers__ interface to hardware via the Hardware Abstraction Layer (HAL). In general, device drivers control one or more peripheral devices, in
response to a user request.
* Device drivers may receive and process interrupts fom their hardware. device drivers may exist alone or may be located under an Intermediate driver in a driver stack. If a device driver exists in a driver stack, it is always at the bottom of the stack. An example of a device driver in a driver stack is the NT serial port driver. Our Toaster example driver mentioned in previous chapters, would also be a device driver. The Toaster driver would probably exist on its own, without an Intermediate or File System driver above it.
* What distinguishes a __Mini-Driver__ fom other Device drivers is that the Mini-Driver exists within a "wrapper." The Mini-Driver's interfaces are typically restricted to those provided by the wrapper, which dictates the structure of the Mini-Driver. <p align="center"><img src="https://i.imgur.com/ZWGwZ6H.png"  width="300px" height="auto"></p>
* The purpose of the Mini-Driver approach is to make it relatively easy to write drivers for common peripherals, such as video cards, net cards, and SCSI adapters. All common processing is done in the wrapper; the only work done by the Mini-Driver is the actual interfacing with the hardware.
* Perhaps the best-known example of a Mini-Driver is the SCSI Miniport driver. The Miniport driver exists inside the wrapper provided by the SCSI Port driver. The SCSI Miniport driver's structure is dictated by the SCSI Port driver. The SCSI Port driver handles all the work common to queuing and processing SCSI requests, including building an appropriate SCSI Command Data Block. The Miniport driver's job is restricted to placing the request on the hardware in a manner that is specific to its particular SCSI adapter.
* So, now that you know the different types of drivers that exist, how are they organized into stacks? In NT 4, driver stacks are mostly static, being created when the system is first started. As an example, Let's look how the FAT File System driver that uses the services of the Intermediate Disk Class driver, which in turn uses the services of the SCSI Port/MiniPort Device driver:
	1. The frst driver to start in the example is the SCSI Miniport driver. When the Miniport driver is started, it causes its wrapper the SCSI port driver, to start. The SCSI Port/Miniport driver searches the bus and finds SCSI adapters that it will control. For each adapter found, the SCSI Port driver creates a Device Object named _\Device\ScsiPortX_, where x is an ordinal number representing a particular SCSI adapter.
	2. After all the SCSI Miniport drivers configured in the system have started, the Class drivers are started, one at a time. The Class driver looks for Device Objects that represent SCSI Port devices, since these are the devices over which it will layer. The Class driver does this by calling the `IoGetDeviceObjectPointer ()`. This resuls on the refrence count of the Device Object to which the File Object belongs to get incredemented. It gets  decremented when the Class driver calls `ObDereferenceObject ()` on the File Object.
	3. Next, the Disk Class driver enumerates the device units on each SCSI Port. For each disk fund, the Disk Class driver creates a Device Object named _\Device\HardDiskX\Partition0_ (where x is the ordinal number of the disk). This Device Object represents the (entire raw) disk volume itself. In addition, the Disk Class driver creates one Device Object for each logical partition on the disk with a format that it can identify as supportable under Windows NT. These Device Objects are named `\Device\HardDiskX\PartitionY`, where x is the ordinal disk number and Y is the ordinal partition number starting at one on that hard disk. The system later assigns actual drive letters (such as C:, D:, and so on) to these devices in the form of symbolic links.<p align="center"><img src="https://i.imgur.com/qvnEEWZ.png"  width="400px" height="auto"></p>
* For each Device Object created, the Class driver stores away the Device Object pointer for the underlying device to which its device is linked. That is, for each Disk Device Object it creates, the Disk Class driver stores (in the disk __Device Object's Device Extension__) the pointer to the SCSI Port Device Object on which that disk unit resides.
* Whenever a Device Object is created for a device that is to be layered above another device, the high-level Device Object must be initialized carefully to reﬂect the attributes of the lower-layer device. The information about the lower-layer device comes from its Device Object, or even fom interrogating the lower-layer physical device itself. See _Characteristics_, _StackSize_ and _AlignmentRequirement_ of `DEVICE_OBJECT` struct.
* __File System drivers (FSDs)__ are at the top of the Windows NT driver stack. FSDs are added dynamically to the driver stack, as opposed to the way Intermediate drivers are added.
* The I/O Manager recognizes file-structured devices through the existence of a __Volume Parameter Block (VPB)__ for the device. The VPB links the Device Object that represents the partition (created by the Class driver) with a Device Object that represents a mounted instance of a file system on that partition (created by an FSD). <p align="center"><img src="https://i.imgur.com/SRg20zB.png"  width="400px" height="auto"></p>
* Using the previous example driver stack, the first time an I/O operation is directed to device `\Device\HardDisk0\Partition1` , the I/O Manager notices that this is a file-structured device and that it does not presently have a fle system associated with it. As a result, it will pass a Mount request to registered disk type file systems, one at a time, asking them if they recognize the file structure on the partition as a type that they support.
* Disk file systems ordinarily ncountered on Windows NT 4 include NTFS and FAT. The frst file system that recognizes the format of the data on the partition will mount the device successfully. As part of the mount process, the FSD creates an (unnamed) Device Object that represents the instance of the mounted file system on that partition.
* With the creation of the File System Device Object, the driver stack is complete. In our example picture illustrated above, we have a File System driver over an Intermediate (Class) driver, which is layered over a Port/Mini-Port Device driver.
* Of course, not every IO request starts at a File System driver. Requests are ordinarily directed to the driver that owns the Device Object named in the Create operation.
* Windows NT uses a layered driver model to process IO requests. In this model, drivers are organized into stacks.
* Each driver in a stack is responsible for processing the part of the request that it can handle, if any.
* If the driver's processing of the request results in its completion, the driver calls `IoCompleteRequest ()` to complete the request.
* If the request cannot be completed, information for the next lower-level driver in the stack is set up and the request is then passed along to that driver.
* When the I/O Manager receives an I/O system services call, it allocates an IRP with at least as many I/O Stack locations as there are drivers in the driver stack. The I/O Manager determines this quantity by examining the StackSize field of the top Device Object in the stack. The I/O Manager then initializes both the fxed part of the IRP and the IRP's first I/O Stack location. The I/O Manager then calls the frst driver in the stack at its appropriate Dispatch routine to start processing the request.
* If the driver can complete the IRP itself, either immediately or by queuing the IRP for later processing, it will do so. If a driver decides that it cannot completely handle a particular request itself, it can decide to pass that request on to the next lower-level driver in its driver stack. The driver may do this either immediately upon receiving the request, or afer partially processing a request.
* In order to pass an IRP to another driver, the driver must set up the next I/O Stack location in the IRP for the underlying driver to which the request will be passed. The driver calls the `IoGetNextIrpStackLocation ()` function to get a pointer to the next I/O Stack location.
* Using this pointer, the driver fills in the parameters of the request that need to be passed to the next-lowest-level driver. The request is then passed to a specific lower-level driver by calling the `IoCallDriver ()` function.
* The call to the `IoCallDriver ()` function causes the I/O Manager to "push" the I/O Stack, resulting in the I/O Stack location that had been "next" becoming "current.".
* The `IoCallDriver ()` function call also causes the I/O Manager to find the driver associated with the target Device Object and to call that driver's Dispatch routine that corresponds to the Major Function code in the now current I/O Stack location.
* It is important to understand that the call to `IoCallDriver ()` causes the I/O Manager to directly call the target driver's Dispatch routine afer performing a minimal amount of processing; the I/O Manager does not delay, queue, or schedule this call in any way. Thus, a higher-level driver's call to `IoCallDriver ()` does not return until the Dispatch routine of the called driver performs a return operation. <p align="center"><img src="https://i.imgur.com/DIgAERv.png"  width="500px" height="auto"></p>
* A variation on passing an IRP to a lower-level driver is when a driver chooses to process an IRP by creating one or more additional (new) IRPs and passes these IRPs to a lower-level driver. IRPs may be created by a driver using a variety of methods, the most common of which is to call the `IoAllocateIrp ()` function.
* A slightly different approach to calling `IoAllocateIrp ()` is for a driver to call the `IoMakeAssociatedIrp ()` function.
* `IoMakeAssociatedIrp ()` allows the creation of IRPs that are "associated" with a "master" IRP. The driver that calls `IoMakeAssociatedIrp ()` must manually initialize the __AssociatedIrp.Irpcount__ field of the master IRP to the count of associated IRPs that are created prior to calling `IoMakeAssociatedIrp ()`. Associated IRPs may only be used by the topmost driver in a stack.
* A driver may wish to be informed when a request that it has passed to a lower level driver is completed. It can do this by calling the `IoSetCompletionRoutine ()` function prior to passing the IRP to an underlying driver. Invoking this function causes a pointer to a completion routine and the supplied completion routine's context argument to be stored in the next I/O Stack location in the IRP.
* When `IoCompleteRequest ()` is called, the I/O Manager starts at the current I/O Stack location and walks backward up the stack, calling completion routines as it goes. Completion routines are called serially, one after another. Figure above illustrates this process. Again, these calls are made directly by the I/O Manager fom within the `IoCompleteRequest ()` function: there is no queuing or scheduling involved. Additionally, note that the Completion routine is called at the same IRQL at which `IoCompleteRequest ()` was called. This may be any IRQL, up to and including IRQL __DISPATCH_LEVEL__. <p align="center"><img src="https://i.imgur.com/DIgAERv.png"  width="500px" height="auto"></p>
* Because the current driver's Completion routine is stored in the next I/O Stack location, the Completion routine of any driver calling `IoCompleteRequest ()` never gets called.
* Another implication of storing the Completion routine information in the next I/O Stack location is that a driver that is the lowest driver in the stack, typically the device driver, must never attempt to set a Completion routine to be called.
* The contents of the IRP are still valid when the Completion routine is called. Thus, a driver may get a pointer to its I/O Stack location by calling `IoGetCurrentlrpStackLocation ()` . The current IRP Stack location may thus be used to pass information from the Dispatch routine to the Completion routine (the contents of all lower I/O Stack Locations are cleared by the I/O Manager
before the Completion routine is called)
* One particularly useful technique in a Completion routine is the capability of a driver to reclaim ownership of the IRP passed to it. This can be accomplished by the Completion routine returning with the status __STATUS_MORE_PROCESSING_REQUIRED__. When a driver returns __STATUS_MORE_PROCESSING_REQUIRED__ from its Completion routine, the I/O Manager immediately stops Completion processing of the IRP.
* Completion routines are used for a wide variety of purposes. For example, a Class or File System driver may want to know the Completion status of a disk read, so that if the read fails, it can reissue the request. Alternatively, an Intermediate disk block encryption driver may need to know when read operations are completed on a disk volume, so that it can decrypt the read data.
* **!!Important!!** A common mistake is made in implementing Completion routines that wish to resubmit an IRP to a lower-layer driver. The mistake is that the driver calls `IoCallDriver ()` from its Completion routine to pass the IRP to the underlying driver. Although this might initially sound like a good idea, this can result in stack overfiow problems. Also recall that Completion routines may be called at any IRQL <= DISPATCH_LEVEL . Because `IoCallDriver ()` results in a target driver's Dispatch routine being called directly, this also results in the target driver's Dispatch routine being called at IRQL __DISPATCH_LEVEL__ . This is likely to be a fatal error, resulting in a system crash, because most drivers expect their dispatch routines to be called at IRQL __PASSIVE_LEVEL__ or __IRQL APC_LEVEL__ . To avoid these problems, Completion routines should send the IRP to a worker thread running at IRQL __PASSIVE_LEVEL__. The worker thread can then call `IoCallDriver ()` to resubmit the IRP to the underlying driver.
* The simplest way that two drivers can communicate is to simply call each other's routines directly. Because all drivers reside within the system process' address space. On loading, each driver creates and initializes a structure into which it places pointers to the entry points it wishes to export to the other driver, and calls a function in a DLL to store this information. When one driver wishes to call the other, it simply does so through a set of functions provided by the DLL, which calls the appropriate function through one of the previously provided pointers.
* This is precisely how TDI (Intermediate layer) drivers interface with NDIS drivers. The NDIS wrapper is the common library DLL. When the TDI has a message that it wants the NDIS driver to send, it calls a function in the NDIS wrapper, which in turn directly calls the NDIS driver. The status returned by the NDIS driver is returned by the NDIS wrapper as the status of the TDI's call. Likewise, when the NDIS driver receives a message, it passes that message to the TDI by calling the TDl's Receive function through a function provided by the NDIS wrapper. <p align="center"> <img src="https://i.imgur.com/Cph16Rr.png"  width="500px" height="auto"></p>
* The Class drivers and SCSI Port drivers use yet another _special understanding_ to facilitate their communication. When it receives an IRP, the Class driver builds an auxiliary data structure called a __SCSI Request Block (SRB)__ in a __nonpaged pool__. A pointer to this structure is stored in a prearranged field in the SCSI Port driver's J/O Stack location. When the SCSI Port driver receives the IRP, it primarily looks to the SRB for information to describe the request.
* The Windows NT J/O Manager includes the capability for one Kernel mode driver to "attach" one of its Device Objects to a Device Object
created by a different driver. The result of this is that IRPs destined for the driver associated with the original Device Object will be sent to the driver associated with the "attached" Device Object. This attached driver is a __Filter driver__.
* There are a couple of different mechanisms that allow a Filter driver to attach its Device Object to that of another driver. One way is for the Filter driver to first find the Device Object for the device it wants to attach using `IoGetDeviceObjectPointer ()`. The Filter driver then attaches its Device Object to the found Device Object using the `IoAttachDeviceToDeviceStack ()` function.
* Every Device Object has a field named _AttachedDevice_, which points to the Device Object of the first Filter driver that has attached this Device Object. If the _AttachedDevice_ field of the Device Object is NULL, there are no attached devices. If the AttachedDevice field is not-zero, it points to a Filter driver's Device Object.
* `IoAttachDeviceToDeviceStack ()` finds the end of the _AttachedDevice_ list for the Device Object pointed to by TargetDevice, and points the AttachedDevice field of this final Device Object to the Filter driver's Device Object. <p align="center"> <img src="https://i.imgur.com/XbaA6aS.png"  width="500px" height="auto"></p>
* Another way for a Filter driver to attach its Device Object to that of another device is to call the `IoAttachDevice ()` function. This function simply combines the functionality provided by `IoGetDeviceObjectPointer ()` and `IoAttachDeviceToDeviceStack ()`.
* It is very important to remember that the I/O Manager only runs the AttachedDevice list to find the last attached Device Object as a result of processing a Create request. No redirection occurs when `IoCallDriver ()` is called. Therefore, for a Filter driver to be successful, it must attach any device in which it is interested before a higher-layered device calls `IoGetDeviceObjectPointer ()` (which, as previously discussed, issues a Create) to find its Device Object.
* Filter Driver Usage Guidelines:
  * If inserting a Filter driver into the device stack causes anything to break, it's automatically the Filter driver's fault. The onus is thus on the Filter driver to adapt itself and to make sure that everything is working correctly after it is introduced. This is true, even if the Filter driver needs to compensate for an error in a lower-layered driver!
  * It is up to the Filter driver to understand how the device to which it attaches works. If the Filter driver can't "understand" the requests that it receives, it is not the fault of the device to which it attaches.
  * Filter drivers must appear to any drivers that layer above them as close to the original device as possible.
  * Filter drivers must plan to work well with other Filter drivers. These include Filter drivers that attach a device before them and Filter drivers that attach the device after they have attached it.
* As we mentioned at the start of this chapter, File System drivers are very closely coupled with the __NT Memory__ and __Cache Manager__ subsystems. Thus, requests to read or write to a File Object often result in nothing more than a cached operation in the File System. None of the other driver layers are involved. Because these cached operations can be very fast, the I/O Manager implements an optimized method for dealing with these requests. This method is called __Fast__ or __Turbo__ I/O.
* A driver that supports fast I/O creates what is called a __Fast I/O Dispatch Table__. A driver that supports Fast I/O points the _FastioDispatch_ field of its Driver Object to this table.
* When the I/O Manager receives an I/O request, for certain drivers and functions, it checks to see if the driver supports Fast I/O for this function before it builds an IRP. If the driver supports Fast I/O for this function, the I/O Manager calls the driver at its Fast I/O entry point, in the context of the requesting thread, with the parameters supplied with the request. If the driver can completely handle the request in its Fast I/O routine, it does so, and returns TRUE as the result of the call to its Fast I/O entry point. If the driver cannot handle the request in its Fast I/O routine, it returns FALSE.
* When a driver returns FALSE as a result of a call to one of its Fast I/O entry points, the I/O Manager proceeds as if no Fast I/O entry point had been supplied. That is, the I/O Manager builds an IRP in the normal way and calls the driver with that IRP.
* Except for TDI devices, Fast I/O is usable by only small number of drivers:
  * Drivers that are the top driver stack.
  * I/O Manager restricts most operations, including read and write operations, to File System drivers only.
  * However, one operation for which the I/O Manager does support Fast I/O for non-File System drivers is IRP_MJ_DEVICE_CONTROL.
  * Driver must be able to completely process the request in the context of the calling thread.

### Driver Structure

- The Windows NT driver architecture uses an entry point model, in which the 1/0 Manager calls a particular routine in a driver when it wants the driver to perform a particular function.