# Windows Internals 6ed.

##  Chapter 1 Concepts and Tools

- The Microsoft NET Framework consists of a library of classes called the **Framework Class Library (FCL)** and a **Common Language Runtime (CLR)** that provides a managed code execution environment with features such as just-in-time compilation, type verification, garbage collection, and code access security.

- Dotnet compiler converts the source code to an intermadiate form (bytecode) called **Common Intermediate Language (CIL)**, which then processed by CLR to produce native code.
- The CLR is implemented as a classic **COM server** whose code resides in a standard user-mode Windows DLL. Nothing from .NET runs in kernel mode.
- All componenets of the dotnet framework are implemented as a standard user mode windows dll (nothing runs in kernel mode.)
- Managed code (code executed by the CLR), unmanaged code (for example, native code APIs, such as Win32).

<p align="center"><img src="https://i.imgur.com/HE0hm5t.png" width="400px" height="auto"></p>

- At the highest level of abstraction, a Windows process comprises the following:
    - A private **virtual address space (VAS)**, which is a set of virtual memory addresses that the process can use.
    - An executable program, which defines initial code and data and is mapped into the process’ VAS.
    - A list of open handles to various system resources—such as semaphores, communication ports, and files—that are accessible to all threads in the process
    - A security context called an access token that identifies the user, security groups, privileges, User Account Control (UAC) virtualization state, session, and limited user account state associated with the process.
    - A unique identifier called a process ID (internally part of an identifier called a client ID)
    - At least one thread of execution (although an “empty” process is possible, it is not useful).
- Each process also points to its parent or creator process If the parent no longer exists, this information is not updated.
- Windows maintains only the creator process ID, not a link back to the creator of the creator, and so forth.
- Just as note, a process with "non responding" state, means the threat might be running or waiting for I/O or some Windows synchronization object.
- A **thread** includes the following essential components:
    - set of CPU registers representing the state of the processor
    - Two stacks—one for the thread to use while executing in kernel mode and one for executing in user mode
    - A private storage area called **thread-local storage (TLS)** for use by subsystems, run-time libraries, and DLLs
    - A unique identifier called a thread ID (part of an internal structure called a client ID—process IDs and thread IDs are generated out of the same namespace, so they never overlap)
    - Threads sometimes have their own security context, or token, that is often used by multithreaded server applications that impersonate the security context of the clients that they serve.
- The volatile registers, stacks, and private storage area are called the thread’s context (Use `GetThreadContext()` or `Wow64GetThreadContext()` to get the context.)
- Switching execution between threads involves the kernel scheduler and it can be an expensive operation, to reduce this cost, Windows implemets two mechanims: 
    - **Fibers**:
        - allow an application to schedule its own “threads” of execution rather than rely on the priority-based scheduling mechanism built into Windows.
        - They are more lightweight.
        - they’re invisible to the kernel because they’re implemented in user mode in Kernel32.dll.
        - Think of goroutines in golang.
    - **UMS (User mode scheduling)**.
        - available only on 64-bit versions of Windows.
        - provide the same basic advantages as fibers, without many of the disadvantage.
        - have their own kernel thread state and are therefore visible to the kernel, which allows multiple UMS threads to issue blocking system calls, share and contend on resources, and have per-thread state.
        - However, as long as two or more UMS threads only need to perform work in user mode, they can periodically switch execution contexts (by yielding from one thread to another) without involving the scheduler: the context switch is done in user mode.
        - From the kernel’s perspective, the same kernel thread is still running and nothing has changed When a UMS thread performs an operation that requires entering the kernel (such as a system call), it switches to its dedicated kernel-mode thread (called a directed context switch).
- All the threads in a process have full read-write access to the process virtual address space.
- Threads cannot accidentally reference the address space of another process, however, unless the other process makes available part of its private address space as a **shared memory section** (called file mapping object in the Windows API) or unless one process has the right to open another process to use cross-process memory functions such as `ReadProcessMemory()` and `WriteProcessMemory()`.
- In addition to a private address space and one or more threads, each process has:
    - a security context
    - and a list of open handles to kernel objects such as files, mutexes, events, or semaphores ...

<p align="center"><img src="https://i.imgur.com/AfczKx3.png" width="400px" height="auto"></p>

- The **virtual address descriptors (VADs)** are data structures that the memory manager uses to keep track of the virtual addresses the process is using.
- By default, threads don’t have their own access token, but they can obtain one, thus allowing individual threads to impersonate the security context of another process—including processes on a remote Windows system—without affecting other threads in the process.

### Job 

- Windows provides an extension to the process model called a **job**.
- A job object’s main function is to allow groups of processes to be managed and manipulated as a unit.
- In some ways, the job object compensates for the lack of a structured process tree in Windows—yet in many ways it is more powerful than a UNIX-style process tree.

### Virtual Memory

- Windows implements a virtual memory system based on a flat (linear) address space that provides each process with the illusion of having its own large, private address space.
- At run time, the memory manager, with assistance from hardware, translates, or maps, the virtual addresses into physical addresses, where the data is actually stored.
- By controlling the protection and mapping, the OS can ensure that individual processes don’t bump into one another or overwrite OS data .

<p align="center"><img src="https://i.imgur.com/46ak97H.png" width="400px" height="auto"></p>

- Because most systems have much less physical memory than the total virtual memory in use by the running processes, the memory manager transfers, or pages, some of the memory contents to disk.
- Paging data to disk frees physical memory so that it can be used for other processes or for the OS itself When a thread accesses a VA that has been paged to disk, the virtual memory manager loads the information back into memory from disk.
- Applications don’t have to be altered in any way to take advantage of paging because hardware support enables the memory manager to page without the knowledge or assistance of processes or threads.
- The size of the virtual address space varies for each hardware platform.
- On 32-bit x86, a process can address 4GB of memory space.
    -  By default, Windows allocates half this address space (the lower half of the 4-GB virtual address space, from __0x00000000 through 0x7FFFFFFF__) to processes for their unique private storage.
    - and uses the other half (the upper half, addresses __0x80000000 through 0xFFFFFFFF__) for its own protected OS memory utilization.
    - Windows supports boot-time options (the __increaseuserva__ qualifier in the Boot Configuration Database that give processes running specially marked programs (the large address space aware flag must be set in the header of the executable image) the ability to use up to 3 GB of private address space (leaving 1 GB for the kernel).
    <p align="center"><img src="https://i.imgur.com/hnF7YlL.png" width="400px" height="auto"></p>
    - Although 3 GB is better than 2 GB, it’s still not enough virtual address space to map very large (multigigabyte) databases.
    - To address this need on 32-bit systems, Windows provides a mechanism called **Address Windowing Extension (AWE)**, which allows a 32-bit application to allocate up to 64 GB of physical memory and then map views, or windows, into its 2-GB virtual address space.
    - Although using AWE puts the burden of managing mappings of virtual to physical memory on the programmer, it does address the need of being able to directly access more physical memory than can be mapped at any one time in a 32-bit process address space.
- 64-bit Windows provides a much larger address space for processes:
    - 7152 GB on IA-64 systems
    - and 8192 GB on x64 systems. 
    - Note that these sizes do not represent the architectural limits for these platforms.
    - 64 bits of address space is over 17 billion GB, but current 64-bit hardware limits this to smaller values.
    - And Windows implementation limits in the current versions of 64-bit Windows further reduce this to 8192 GB (8 TB)
    <p align="center"><img src="https://i.imgur.com/oVVFRcB.png" width="400px" height="auto"></p>


### Kernel Mode vs. User Mode

- To prevent user applications to modify critical OS data, Windows uses two processor access modes (even if the processor on which Windows is running supports more than two): **user mode** and **kernel mode**.
- User application code runs in user mode, whereas OS code (such as system services and device drivers) runs in kernel mode
- Kernel mode refers to a mode of execution in a processor that grants access to all system memory and all CPU instructions.
- Although each Windows process has its own private memory space, the kernel-mode OS and device driver code share a single virtual address space.
- Each page in virtual memory is tagged to indicate what access mode the processor must be in to read and/or write the page.
- Pages in system space can be accessed only from kernel mode, whereas all pages in the user address space are accessible from user mode.
- Read-only pages (such as those that contain static data) are not writable from any mode.
- Additionally, on processors that support no-execute memory protection, Windows marks pages containing data as nonexecutable, thus preventing inadvertent or malicious code execution in data areas.
- 32-bit Windows doesn’t provide any protection to private read/write system memory being used by components running in kernel mode.
- In other words, once in kernel mode, OS and device driver code has complete access to system space memory and can bypass Windows security to access objects.

### Terminal Services and Multiple Sessions

- Terminal Services refers to the support in Windows for multiple interactive user sessions on a single system.
- With Windows Terminal Services, a remote user can establish a session on another machine, log in, and run applications on the server
- The server transmits the graphical user interface to the client (as well as other configurable resources such as audio and clipboard), and the client transmits the user’s input back to the server.
- Similar to the __X Window System__, Windows permits running individual applications on a server system with the display remoted to the client instead of remoting the entire desktop.
- Note that _Windows Client_ support only one remote user to connect to the machine, however in _Windows Server_, it supports __2 simultaneous remote connections__.

### Objects and Handles

- In the windows OS, for example: a file, process, thread or event object are examples of __kernel objects__, they are based on low level objects that Windows creates and manages (object manager). These objects are __opaque__ meaning that you must call an object service to get / set data into it.
- Not all data structures in Windows are objects. Only data that needs to be
shared, protected, named, or made visible to user-mode programs (via system services) is placed in objects .

### Security

- Windows has three forms of access control over objects:
    - __Discretionary Access Control (DAC)__.
    - __Privileged access control (PAC)__.
    - __Mandatory Integrity Control (MAC)__.

### Registry

- Most internal Windows strings are implemented in __Unicode__, when you use the ANSI version of an API, Windows have to convert it to unicode and also when returnin back from the api, this have a small performance impact.

### Digging into Windows Internals

- __Performance Monitor__ is a valuable tool for system monitoring, it also contains performance counter logs.
- It includes hundreds of base and extensible counters for various objects
- __Ressourdce Monitor__ shows four primary system resources: CPU, Disk, Network, and Memory.
- Example of information you can grab using these tools:
    - per-process CPU usage.
    - CPU usage of each service hosting process.
    - Physical memory bar graph displays the current organization of physical memory into either hardware reserved, in use, modified, standby, and free memory.
    - Per-file information for I/Os in a way that makes it easy to identify the most accessed, written to, or read from files on the system. 
    - Active network connections and which processes own them, as well as how much data is going through them.

### Kernel debugging

- Kernel debugging means examining internal kernel data structures and/or stepping through functions in the kernel.
- User mode debugging has 2 modes to attach to a process:
- __Invasive__:
    - Unless specified otherwise, when you attach to a running process, the `DebugActiveProcess()` API is used to establish a connection between the debugger and the debugee.
    - This permits examining and/or changing process memory, setting breakpoints, and performing other debugging functions.
    - Windows allows you to stop debugging without killing the target process, as long as the debugger is detached, not killed
- __Noninvasive__:
    - The debugger simply opens the process with the `OpenProcess()` API.
    - It does not attach to the process as a debugge. - This allows you to examine and/or change memory in the target process, but you cannot set breakpoints.
- There are two debuggers that can be used for kernel debugging:
    - a command-line version **(Kd.exe)**
    - and a graphical user interface (GUI) version **(Windbg exe)**
- Both provide the same set of commands, so which one you choose is a matter of personal preference.
- **LiveKd** is a free tool from _Sysinternals_ that allows you to use the standard Microsoft kernel debuggers just described to examine the running system without booting the system in debugging mode.
    - Useful when kernel-level troubleshooting is required on a machine that wasn’t booted in debugging mode.
    - Certain issues might be hard to reproduce reliably, so a reboot with the debug option enabled might not readily exhibit the error.

### Windows Software Development Kit (SDK)

- contains the documentation, C header files, and libraries necessary to compile and link Windows applications.
- The header files contained in the Windows SDK always match the latest version ofWindows, whereas the version that comes with Visual C++ might be an older version that was current when Visual C++ was released ).
- items of interest in the Windows SDK include the Windows API header files `(\Program Files\Microsoft SDKs\Windows\v7.0A\Include)`. A few of these tools are also
shipped as sample source code in both the Windows SDK and the MSDN Library.

### Windows Driver Kit (WDK)

- Aimed at device driver developers, it is an abundant source of Windows internals information.
- Contains a comprehensive description of all the Windows kernel support functions and mechanisms used by device drivers in both a tutorial and reference form.
- Contains header files (in particular, ntddk.h, ntifs.h, and wdm.h) that define key internal data structures and constants as well as interfaces to many internal system routines.

### Sysinternals  Tools

- Various tools which helps diagnose, troubleshoot, and monitor the windows OS.
- The most popular tools include Process Explorer and Process Monitor.
- Heavily used in malware analysis.

## Chapter 2: System Architecture

### Operating System Model

- Windows is similar to most UNIX systems in that it’s a **monolithic** OS in the sense that the bulk of the OS and device driver code shares the same kernel-mode protected memory space.
- Here is a simplified version of this architecture:

<p align="center"><img src="https://i.imgur.com/NcPJBiS.png" width="500px" height="auto"></p>

- The four basic types of user-mode processes are described as follows:
    - Fixed (or hardwired) **system support processes**, such as the logon process and the Session Manager, that are not Windows services (That is, they are not started by the service control manager)
    - **Service processes** that host Windows services, such as the _Task Scheduler_ and _Print Spooler_ services. Services generally have the requirement that they run independently of user logons.
    - **User applications**, which can be one of the following types: Windows 32-bit or 64-bit, Windows 3 1 16-bit, MS-DOS 16-bit, or POSIX 32-bit or 64-bit. Note that 16-bit applications can be run only on 32-bit Windows.
    - **Environment subsystem server processes**, which implement part of the support for the oOS environment, or personality, presented to the user and programmer. Windows NT originally shipped with three environment subsystems: Windows, POSIX, and OS/2 However, the POSIX and OS/2 subsystems last shipped with Windows 2k. The Ultimate and Enterprise editions of Windows client as well as all of the server versions include support for an enhanced POSIX subsystem called __Subsystem for Unix-based Applications (SUA)__.
- User applications don’t call the native Windows OS services directly; rather, they go through one or more **subsystem dynamic-link libraries (DLLs)**.
- The role of the subsystem DLLs is to translate a documented function into the appropriate internal (and generally undocumented) native system service calls. This translation might or might not involve sending a message to the environment subsystem process that is serving the user application.
- The kernel-mode components of Windows include the following:
    - The Windows executive.
    - The Windows kernel 
    - Device drivers
    - The hardware abstraction layer (HAL).
    - The windowing and graphics system
-  List of important file names of the core Windows OS components:
<p align="center"><img src="https://i.imgur.com/DiTqz7s.png" width="600px" height="auto"></p>

### Portability

- Windows was designed with a variety of hardware architectures. Back in time, _MIPS, Alpha AXP_ and _PowerPC_ were supported but got dropped later due to changing market demands which left only _Intel x86/x64_ the only one kept being supported.
- Windows has a **layered design**, with low-level portions of the system that are processor architecture-specific or platform-specific **isolated** into separate modules so that upper layers of the system can be shielded from the differences between architectures and among hardware platforms.
- The vast majority of Windows is written in **C**, with **some portions in C++**. **Assembly** language is used only for those parts of the OS that need to communicate directly with system hardware (such as the interrupt trap handler) or that are extremely performance-sensitive (such as context switching).

### Symmetric Multiprocessing

- **Multitasking** is the OS technique for sharing a single processor among multiple threads of execution.
- When a computer has more than one processor, however, it can execute multiple threads simultaneously.
- Thus, whereas a **multitasking** OS only appears to execute multiple threads at the same time, a **multiprocessing** OS actually does it, executing one thread on each of its processors.
- Windows is a **symmetric multiprocessing (SMP)** OS.
    - There is no master processor
    - the OS as well as user threads can be scheduled to run on any processor 
    - Also, all the processors share just one memory space
- This model contrasts with **asymmetric multiprocessing (ASMP)**, in which the OS typically selects one processor to execute OS kernel code while other processors run only user code.
- Windows also supports three modern types of multiprocessor systems: **multicore, Hyper-Threading enabled, and NUMA** (non-uniform memory architecture).
<p align="center"><img src="https://i.imgur.com/LRIa4ot.png" width="500px" height="auto"></p>

### Scalability

- One of the key issues with multiprocessor systems is scalability .
- Windows incorporates several features that are crucial to its success as a multiprocessor OS:
    - The ability to run OS code on any available processor and on multiple processors at the same time.
    - Multiple threads of execution within a single process, each of which can execute simultaneously on different processors
    - Fine-grained synchronization within the kernel (such as spinlocks, queued spinlocks, and pushlocks,)
    - Programming mechanisms such as I/O completion ports (that facilitate the efficient implementation of multithreaded server processes that can scale well on multiprocessor systems).

### Differences Between Client and Server Versions

- Windows ships in both client and server retail packages.
- The client version differs by (Pro, Ultimate, Home, ...):
    - The number of processors supported (in terms of sockets, not cores or threads).
    - The amount of physical memory supported.
    - The number of concurrent network connections supported 
    - And features like: Multi-Touch, Aero, Desktop Compositing, etc...
- The server systems are optimized by default for **system throughput** as high performance application servers, whereas the client version (although it has server capabilities) is optimized for **response time** for **interactive desktop use**. 
- Look for `GetVersion(Ex)` or `RtlGetVersion` to determine which editions of Windows you are running on. Worth checkig as well: `VerifyVersionInfo()` or `RtlVerifyVersionInfo()`.
- Windows supports more than 100 different features that can be enabled through the software licensing mechanism.
    - You can use the SlPolicy tool available [here](https://github.com/zodiacon/WindowsInternals/tree/master/SlPolicy) to display these policy values"
<p align="center"><img src="https://i.imgur.com/Snj8dgy.png" width="600px" height="auto"></p>

