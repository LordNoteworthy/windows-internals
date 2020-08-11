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
    - Windows supports boot-time options (the __increaseuserva__ qualifier in the Boot Configuration Database that give processes running specially marked programs (the large address space aware flag must be set in the header of the executable image) the ability to use up to 3 GB of private address space (leaving 1 GB for the kernel).<p align="center"><img src="https://i.imgur.com/hnF7YlL.png" width="400px" height="auto"></p>
    - Although 3 GB is better than 2 GB, it’s still not enough virtual address space to map very large (multigigabyte) databases.
    - To address this need on 32-bit systems, Windows provides a mechanism called **Address Windowing Extension (AWE)**, which allows a 32-bit application to allocate up to __64 GB__ of physical memory and then map views, or windows, into its 2-GB virtual address space.
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
- User application code runs in user mode, whereas OS code (such as system services and device drivers) runs in kernel mode.
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
    - **Environment subsystem server processes**, which implement part of the support for the OS environment, or personality, presented to the user and programmer. Windows NT originally shipped with three environment subsystems: Windows, POSIX, and OS/2 However, the POSIX and OS/2 subsystems last shipped with Windows 2k. The Ultimate and Enterprise editions of Windows client as well as all of the server versions include support for an enhanced POSIX subsystem called __Subsystem for Unix-based Applications (SUA)__.
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

### Checked vs Free builds

- **checked build** is a recompilation of the Windows source code with a compile time flag defined called `DBG`:
    - easier to understand the machine code, the post-processing of the Windows binaries to optimize code layout for faster execution is not performed
    - aid device driver developers because it performs more stringent error checking on kernel-mode functions called by device drivers or other system code
    - additional detailed informational tracing that can be enabled for certain components.
- **free build** is used in production environments.
    - is built with full compiler optimization.

### Windows Architecture

<p align="center"><img src="https://i.imgur.com/gz5AHxH.png" width="600px" height="auto"></p>

### Environment Subsystems and Subsystem DLLs

- The role of an environment subsystem is to expose some subset of the base Windows executive system services to application programs.
- User applications don’t call Windows system services **directly**. Instead, they go through one or more subsystem DLLs.
- These libraries export the documented interface that the programs linked to that subsystem can call For example, the Windows subsystem DLLs (such as Kernel32 dll, Advapi32 dll, User32 dll, and Gdi32 dll) implement the Windows API functions. The **SUA** subsystem DLL (Psxdll.dll) implements the **SUA API** functions.
- When an application calls a function in a subsystem DLL, one of three things can occur:
    - The function is __entirely implemented in user mode__ inside the subsystem DLL. Examples of such functions include `GetCurrentProcess` (which always returns –1, a value that is defined to refer to the current process in all process-related functions) and `GetCurrentProcessId` (The process ID doesn’t change for a running process, so this ID is retrieved from a cached location, thus avoiding the need to call into the kernel).
    - The function requires __one or more calls__ to the Windows executive For example, the Windows `ReadFile` and `WriteFile` functions involve calling the underlying internal (and undocumented) Windows I/O system services `NtReadFile` and `NtWriteFile`, respectively.
    - The function requires some work to be done in the __environment subsystem process__ (The environment subsystem processes, running in user mode, are responsible for maintaining the state of the client applications running under their control ). In this case, a client/server request is made to the environment subsystem via a message sent to the subsystem to perform some operation. The subsystem DLL then waits for a reply before returning to the caller. Some functions can be a combination of the second and third items just listed, such as the Windows `CreateProcess` and `CreateThread` functions.
- The Windows subsystem consists of the following major components:
    - For each session, an instance of the __environment subsystem process (Csrss exe)__ loads three DLLs (Basesrv.dll, Winsrv.dll, and Csrsrv.dll).
    - A __kernel-mode device driver (Win32k.sys)__ includes (Window manager, GDI & wrappers for DirectX).
    - The __console host process (Conhost exe)__, which provides support for console (character cell) applications.
    - __Subsystem DLLs__ (such as Kernel32.dll, Advapi32.dll, User32.dll, and Gdi32.dll) that translate documented Windows API functions into the appropriate and mostly undocumented kernel-mode system service calls in Ntoskrnl exe and Win32k sys.

### Subsystem Startup

- Subsystems are __started__ by the __Session Manager (Smss exe)__ process.
- The Required value lists the subsystems that load when the system boots.
- The Windows value contains the file specification of the Windows subsystem, __Csrss.exe__, which stands for _Client/Server Run-Time Subsystem_. 

### Windows Subsystem

- To avoid duplicating code, the SUA subsystem calls services in the Windows subsystem to perform display I/O.
- The Windows subsystem consists of the following major components:
    - For each session, an instance of the environment subsystem process (Csrss.exe) loads three DLLs (Basesrv.dll, Winsrv.dll, and Csrsrv.dll)
    - A kernel-mode device driver (Win32k.sys): Window manager + GDI + Wrappers for DirectX.
    - The console host process (Conhost.exe), which provides support for console (character cell) applications.
    - Subsystem DLLs: the bridge to Ntoskrnl exe and Win32k.sys.
    - Graphics device drivers for hardware-dependent graphics display drivers, printer drivers, and video miniport drivers.

#### Console Window Host

- In the original Windows subsystem design, the subsystem process (Csrss exe) was responsible for the managing of console windows and each console application (such as Cmd exe, the command prompt) communicated with Csrss.
- Windows now uses a separate process, the __console window host (Conhost exe)__, for each console window on the system (A single console window can be shared by multiple console applications, such as when you launch a command prompt from the command prompt By default, the second command prompt shares the console window of the first.)

### Subsystem for Unix-based Applications

- SUA enables compiling and running custom UNIX-based applications on a computer running Windows Server or the Enterprise or Ultimate editions of Windows client.

### Ntdll.dll

- special system support library primarily for the use of subsystem DLLs It contains two types of functions:
    - System service dispatch stubs to Windows executive system services
    - Internal support functions used by subsystems, subsystem DLLs, and other native images:
        - image loader functions (Ldr) + the heap manager
        - Windows subsystem process communication functions (Csr)
        - run-time library routines (Rtl) + user-mode debugging (DbgUi)
        - Event Tracing for Windows (Etw)
        - user-mode asynchronous procedure call (APC) dispatcher and exception dispatcher.
        - a small subset of the C Run-Time (CRT) routines.

### Executive

- is the upper layer of Ntoskrnl exe (The kernel is the lower layer, it includes the following types of functions:
    - __System services__: Functions that are exported and callable from user mode.
    - Device driver functions that are called through the use of the `DeviceIoControl()`.
    - Functions that are exported and callable from kernel mode but are __not documented__ in the WDK (like functions called by the boot video driver `Inbv`)
    - Functions that are defined as __global symbols__ but are not exported.
        -  `Iop` (internal I/O manager support functions) or `Mi` (internal memory management support functions)
    - Functions that are internal to a module that are not defined as global symbols.
- The executive contains the following major components, each of which is covered in detail in a
subsequent chapter of this book:
    - __configuration manager__: responsible for implementing and managing the system registry.
    - __process manager__ creates and terminates processes and threads. The underlying support for processes and threads is implemented in the Windows kernel; the executive adds additional semantics and functions to these lower-level objects.
    - __security reference monitor__ enforces security policies on the local computer. It guards operating system resources, performing run-time object protection and auditing.
    - __I/O manager__ implements device-independent I/O and is responsible for dispatching to the appropriate device drivers for further processing.
    - __Plug and Play (PnP)__ manager determines which drivers are required to support a particular device and loads those drivers.
    - __power manager__ coordinates power events and generates power management I/O notifications to device drivers.
    - __Windows Driver Model Windows Management Instrumentation routines__  enable device drivers to publish performance and configuration information and receive commands from the user-mode WMI service.
    - __cache manager__ improves the performance of file-based I/O by causing recently referenced disk data to reside in main memory for quick access.
    - __memory manager__ implements virtual memory, a memory management scheme that provides a large, private address space for each process that can exceed available physical memory.
    - __logical prefetcher__ and __Superfetch__ accelerate system and process startup by optimizing the loading of data referenced during the startup of the system or a process.
- Additionally, the executive contains four main groups of support functions that are used by the executive components just listed. About a third of these support functions are documented in the WDK because device drivers also use them. These are the four categories of support functions:
    - __object manager__ which creates, manages, and deletes Windows executive objects and abstract data types that are used to represent operating system resources such as processes, threads, and the various synchronization objects.
    - __advanced LPC facility__ (ALPC): passes messages between a client process and a server process on the same computer. Among other things, ALPC is used as a local transport for remote procedure call (RPC), an industry-standard communication facility for client and server processes across a network.
    - A broad set of __common run-time library__ functions, such as string processing, arithmetic operations, data type conversion, and security structure processing.
    - __executive support routines__, such as system memory allocation (paged and nonpaged pool), interlocked memory access, as well as three special types of synchronization objects: resources, fast mutexes, and pushlocks.
- The executive also contains a variety of other infrastructure routines:
    -  __kernel debugger library__, which allows debugging of the kernel from a debugger supporting KD.
    - user-mode debugging framework.
    - __kernel transaction manager__, which provides a common, two-phase commit mechanism to resource managers, such as the _transactional registry (TxR)_ and _transactional NTFS (TxF)_.
    - __hypervisor library__ provides kernel support for the virtual machine environment and optimizes certain parts of the code when the system knows it’s running in a client partition (virtual environment)
    - __errata manager__ provides workarounds for nonstandard or noncompliant hardware devices.
    - __driver verifier__ implements optional integrity checks of kernel-mode drivers and code
    _ __event tracing for windows (ETW)__ provides helper routines for systemwide event tracing for kernel-mode and user-mode components
    - __Windows diagnostic infrastructure__ enables intelligent tracing of system activity based on diagnostic scenarios
    - __Windows hardware error architecture__ support routines provide a common framework for reporting hardware errors
    - __file-system runtime library__ provides common support routines for file system drivers.

### Kernel

- consists of a set of functions in `Ntoskrnl.exe` that provides fundamental mechanisms such as:
    - __thread scheduling__ and __synchronization services__ used by the executive components, as well as low-level hardware architecture–dependent support (such as __interrupt and exception dispatching__ that is different on each processor architecture.
    - written mostly in C + assembly for specialized processor instructions.
    - expose many functions documented in the WDK (Ke).

### Kernel Objects

- Outside the kernel, the executive represents threads and other shareable resources as objects.
- These objects require some __policy overhead__, such as object handles to manipulate them, security checks to protect them, and resource quotas to be deducted when they are created.
-  This overhead is eliminated in the kernel, which implements a set of simpler objects, called __kernel objects__, that help the kernel control central processing and support the creation of executive objects.
- Most executive-level objects encapsulate one or more kernel objects, incorporating their kernel-defined attributes.
- The executive uses kernel functions to create instances of kernel objects, to manipulate them, and to construct the more complex objects it provides to user mode 

### Kernel Processor Control Region and Control Block (KPCR and KPRCB)

- The kernel uses a data structure called the __kernel processor control region__, or KPCR, to store processor-specific data:
    - IDT, TSS, GDT.
    - interrupt controller state, which it shares with other modules, such as the ACPI driver and the HAL.
    - can be accessed via fs/gs register on 32bit/64bits Winsows system correspondingly.
    - On IA64 systems, the KPCR is always located at `0xe0000000ffff0000`
- KPCR also contains an embedded data structure called the __kernel processor control block__ or KPRCB.
    - is a private structure used only by the kernel code in Ntoskrnl.exe
    - contains scheduling information such as the current, next, and idle threads scheduled for execution on the processor
    - the dispatcher database for the processor (which includes the ready queues for each priority level)
    - the DPC queue
    - CPU vendor and identifier information (model, stepping, speed, feature bits);
    - CPU and NUMA topology (node information, cores per package, logical processors per core, and so on)
    - cache sizes
    - time accounting information (such as the DPC and interrupt time); and more
    - also contains all the statistics for the processor, such as I/O statistics, cache manager statistics, DPC statistics, and memory manager statistics 
    - sometimes used to store cache-aligned, per-processor structures to optimize memory access, especially on NUMA systems
        - For example, the nonpaged and paged-pool system look-aside lists are stored in the KPRCB.
```c
0: kd> !pcr
KPCR for Processor 0 at fffff80002c4a000:
    Major 1 Minor 1
	NtTib.ExceptionList: fffff80000b95000
	    NtTib.StackBase: fffff80000b94000
	   NtTib.StackLimit: 0000000000000000
	 NtTib.SubSystemTib: fffff80002c4a000
	      NtTib.Version: 0000000002c4a180
	  NtTib.UserPointer: fffff80002c4a7f0
	      NtTib.SelfTib: 000000007efdb000

	            SelfPcr: 0000000000000000
	               Prcb: fffff80002c4a180
	               Irql: 0000000000000000
	                IRR: 0000000000000000
	                IDR: 0000000000000000
	      InterruptMode: 0000000000000000
	                IDT: 0000000000000000
	                GDT: 0000000000000000
	                TSS: 0000000000000000

	      CurrentThread: fffff80002c5a1c0
	         NextThread: fffffa800586bb50
	         IdleThread: fffff80002c5a1c0

	          DpcQueue: 

0: kd> !prcb
PRCB for Processor 0 at fffff80002c4a180:
Current IRQL -- 13
Threads--  Current fffff80002c5a1c0 Next fffffa800586bb50 Idle fffff80002c5a1c0
Processor Index 0 Number (0, 0) GroupSetMember 1
Interrupt Count -- 000174c7
Times -- Dpc    00000012 Interrupt 00000012 
         Kernel 00000574 User      00000072 

0: kd> dt nt!_KPCR
   +0x000 NtTib            : _NT_TIB
   +0x000 GdtBase          : Ptr64 _KGDTENTRY64
   +0x008 TssBase          : Ptr64 _KTSS64
   +0x010 UserRsp          : Uint8B
   +0x018 Self             : Ptr64 _KPCR
   +0x020 CurrentPrcb      : Ptr64 _KPRCB
   +0x028 LockArray        : Ptr64 _KSPIN_LOCK_QUEUE
   +0x030 Used_Self        : Ptr64 Void
   +0x038 IdtBase          : Ptr64 _KIDTENTRY64
   +0x040 Unused           : [2] Uint8B
   +0x050 Irql             : UChar
   +0x051 SecondLevelCacheAssociativity : UChar
   +0x052 ObsoleteNumber   : UChar
   +0x053 Fill0            : UChar
   +0x054 Unused0          : [3] Uint4B
   +0x060 MajorVersion     : Uint2B
   +0x062 MinorVersion     : Uint2B
   +0x064 StallScaleFactor : Uint4B
   +0x068 Unused1          : [3] Ptr64 Void
   +0x080 KernelReserved   : [15] Uint4B
   +0x0bc SecondLevelCacheSize : Uint4B
   +0x0c0 HalReserved      : [16] Uint4B
   +0x100 Unused2          : Uint4B
   +0x108 KdVersionBlock   : Ptr64 Void
   +0x110 Unused3          : Ptr64 Void
   +0x118 PcrAlign1        : [24] Uint4B
   +0x180 Prcb             : _KPRCB

0: kd> dt nt!_KPRCB
   +0x000 MxCsr            : Uint4B
   +0x004 LegacyNumber     : UChar
   +0x005 ReservedMustBeZero : UChar
   +0x006 InterruptRequest : UChar
   +0x007 IdleHalt         : UChar
   +0x008 CurrentThread    : Ptr64 _KTHREAD
   +0x010 NextThread       : Ptr64 _KTHREAD
   +0x018 IdleThread       : Ptr64 _KTHREAD
   +0x020 NestingLevel     : UChar
   +0x021 PrcbPad00        : [3] UChar

```

### Hardware Support

- The other major job of the kernel is to __abstract__ or __isolate__ the executive and device drivers from variations between the hardware architectures supported by Windows:
    - includes handling variations in functions such as __interrupt handling__, __exception dispatching__, and __multiprocessor synchronization__.
- Some kernel interfaces (such as _spinlock_ routines) are actually implemented in the HAL because their implementation can vary for systems within the same architecture family.
- Other examples of architecture-specific code in the kernel include the interfaces to provide __translation buffer and CPU cache__ support. This support requires different code for the different architectures because of the way caches are implemented.
- Another example is __context switching__:
    - Although at a high level the same algorithm is used for thread selection and context switching
    - there are architectural differences among the implementations on different processors
    - because the context is described by the processor state (registers and so on), what is saved and loaded varies depending on the architecture.

### Hardware Abstraction Layer

- HAL is a loadable kernel-mode module `Hal.dll` that provides the __low-level interface__ to the hardware platform on which Windows is running.
- hides hardware-dependent details such as __I/O interfaces__, __interrupt controllers__, and __multiprocessor communication__ mechanisms.

### Device Drivers

- are loadable kernel-mode modules (typically ending in sys) that interface between the __I/O manager__ and the relevant __hardware__.
- they run in kernel mode in one of three contexts:
    - In the context of the user thread that initiated an I/O function
    - In the context of a kernel-mode system thread
    - As a result of an interrupt (and therefore not in the context of any particular process or thread—whichever process or thread was current when the interrupt occurred)
- they don’t manipulate __hardware directly__, but rather they call functions in the HAL to interface with the hardware.
There are several types of device drivers:
    - __Hardware device drivers__ manipulate hardware (using the HAL) to write output to or retrieve input from a physical device or network There are many types of hardware device drivers, such as _bus drivers_, _human interface drivers_, _mass storage drivers_, and so on
    - __File system drivers__ are Windows drivers that accept file-oriented I/O requests and translate them into I/O requests bound for a particular device
    - __File system filter drivers__, such as those that perform disk mirroring and encryption, intercept I/Os, and perform some added-value processing before passing the I/O to the next layer
    - __Network redirectors and servers__ are file system drivers that transmit file system I/O requests to a machine on the network and receive such requests, respectively
    - __Protocol drivers__ implement a networking protocol such as _TCP/IP_, _NetBEUI_, and _IPX/SPX_.
    - __Kernel streaming filter drivers__ are chained together to perform signal processing on data streams, such as recording or displaying audio and video.

#### Windows Driver Model (WDM)

- _Windows 2000_ added support for __Plug and Play__, __Power Options__, and an extension to the Windows NT driver_ model called the __Windows Driver Model (WDM)__.
- From the WDM perspective, there are three kinds of drivers:
    - __bus driver__ services a bus controller, adapter, bridge, or any device that has child devices.
        - are required drivers, and Microsoft generally provides them; each type of bus (such as __PCI__, __PCMCIA__, and __USB__) on a system has one bus driver.
        - third parties can write bus drivers to provide support for new buses, such as __VMEbus__, __Multibus__, and __Futurebus__
    - __function driver__ is the main device driver and provides the operational interface for its device.
        - It is a required driver unless the device is used raw (an implementation in which I/O is done by the bus driver and any bus filter drivers, such as _SCSI PassThru_).
        - A function driver is by definition the driver that knows the most about a particular device, and it is usually the only driver that accesses device-specific registers.
    - __filter driver__ is used to add functionality to a device (or existing driver) or to modify I/O requests or responses from   other drivers (and is often used to fix hardware that provides incorrect information about its hardware resource requirements) 
        - are optional and can exist in any number, placed above or below a function driver and above a bus driver.
        - usually, system original equipment manufacturers (OEMs) or independent hardware vendors (IHVs) supply filter drivers 

#### Windows Driver Foundation (WDF)

- simplifies Windows driver development by providing two frameworks:
    - the __Kernel-Mode Driver Framework (KMDF)__ and the __User-Mode Driver Framework (UMDF__. 
    - developers can use KMDF to write drivers for Windows 2000 SP4 and later, while UMDF supports Windows XP and later.
- provides a simple interface to WDM.
- hides its complexity from the driver writer without modifying the underlying bus/function/filter model.
- KMDF drivers respond to events that they can register and call into the KMDF library to perform work that isn’t specific to the hardware they are managing, such as __generic power management__ or __synchronization__ .
    - Previously, each driver had to implement this on its own.
    - In some cases, more than 200 lines of WDM code can be replaced by a single KMDF function call.
- UMDF enables certain classes of drivers (mostly USB-based or other high-latency protocol buses) such as those for video cameras, MP3 players, cell phones, PDAs, and printers—to be implemented as user-mode drivers.
    - UMDF runs each user-mode driver in what is essentially a __usermode service__, and it uses __ALPC__ to communicate to a kernel-mode wrapper driver that provides actual access to hardware.
    - If a UMDF driver crashes, the process dies and usually restarts, so the system doesn’t become unstable. The device simply becomes unavailable while the service hosting the driver restarts. 
    - UMDF drivers are written in C++ using COM-like classes and semantics, further lowering the bar for programmers to write device drivers

#### Peering into Undocumented Interfaces

- looking at the list of functions in __ntdll.dll__ gives you the list of all the system services that Windows provides to user-mode subsystem DLLs versus the subset that each subsystem exposes.
- although many of these functions map clearly to documented and supported Windows functions, several are not exposed via the Windows API.
- another interesting image to dump is __Ntoskrnl.exe__, although many of the exported routines that kernel-mode device drivers use are documented in the Windows Driver Kit, quite a few are not.
- a common convention in the major executive components function naming:
    - the first letter of the prefix followed by an i (for __internal__) or ;
    - the full prefix followed by a p (for __private__) .
    - for example, _Ki_ represents internal kernel functions, and _Psp_ refers to internal process support functions.
- commonly Used Prefixes:
    |Prefix | Component|
    |-------|---------|
    |Alpc | Advanced Local Inter-Process Communication|
    |Cc | Common Cache|
    |Cm |Configuration manager|
    |Dbgk | Debugging Framework for User-Mode|
    |Em | Errata Manager|
    |Etw | Event Tracing for Windows|
    |Ex | Executive support routines|
    |FsRtl | File system driver run-time library|
    |Hvl | Hypervisor Library|
    |Io | I/O manager|
    |Kd | Kernel Debugger|
    |Ke | Kernel|
    |Lsa | Local Security Authority|
    |Mm | Memory manager|
    |Nt | NT system services (most of which are exported as Windows functions)|
    |Ob | Object manager|
    |Pf | Prefetcher|
    |Po | Power manager|
    |Pp | PnP manager|
    |Ps | Process support|
    |Rtl | Run-time library|
    |Se | Security|
    |Sm | Store Manager|
    |Tm | Transaction Manager|
    |Vf | Verifier|
    |Wdi |Windows Diagnostic Infrastructure|
    |Whea | Windows Hardware Error Architecture|
    |Wmi | Windows Management Instrumentation|
    |Zw | Mirror entry point for system services (beginning with Nt) that sets previous access mode to kernel, which eliminates parameter validation, because Nt system services validate parameters only if previous access mode is user|
- the general format: `<Prefix><Operation><Object>`

### System Processes

- the following system processes appear on every Windows system.
- two of these `Idle and System` are not full processes because they are not running a user-mode executable.
    - Idle process (contains one thread per CPU to account for idle CPU time).
    - System process (contains the majority of the kernel-mode system threads).
    - Session manager (__Smss.exe__).
    - Local session manager (__Lsm.exe__).
    - Windows subsystem (__Csrss.exe__).
    - Session 0 initialization (__Wininit.exe__).
    - Logon process (__Winlogon.exe__).
    - Service control manager (__Services.exe__) and the child service processes it creates (such as the system-supplied generic service-host process, Svchost exe).
    - Local security authentication server (__Lsass.exe__).

<p align="center"><img src="https://i.imgur.com/FjXtjCq.png" width="400px" height="auto"></p>

#### System Idle Process

- this process (as well as the process named System) isn’t running a real user-mode image.
- its name differs from utility to utility (because of implementation details)

#### System Process and System Threads

- the System process (__PID 4__) is the home for a special kind of thread that runs only in kernel mode: a __kernel-mode system thread__.
- differs from user-mode trheats in that they run only in kernel-mode executing code loaded in system space, whether that is in Ntoskrnl exe or in any other loaded device driver.
- don’t have a user process address space and hence must allocate any dynamic storage from operating system memory heaps, such as a paged or nonpaged pool.
- are created by the `PsCreateSystemThread()`

#### Session Manager (Smss)

- first user-mode process created in the system. 
- by creating multiple instances of itself during boot-up and Terminal Services session creation,
Smss can create multiple sessions at the same time (at maximum, four concurrent sessions, plus one
more for each extra CPU beyond one)
    - enhances logon performance on Terminal Server systems where multiple users connect at the same time.
- the master Smss performs the following one-time initialization, among them:
    - creates named pipes and mailslots used for communication between Smss, Csrss, and Lsm.
    - initializes paging file(s)
    - init the registry
    - creates the Smss to initialize session 0 (noninteractive session)
    - creates the Smss to initialize session 1 (interactive session)
- a session startup instance of Smss does the following:
    - calls `NtSetSystemInformation` with a request to set up kernel-mode session data structures
    - creates the subsystem process(es) for the session (by default: __Csrss.exe__)
- creates an instance of __Winlogon__ (interactive sessions) or __Wininit__ (for session 0)
- then this intermediate Smss process exits (leaving the subsystem processes and Winlogon or Wininit as parent-less processes)

#### Windows Initialization Process (Wininit.exe)

- performs the following system initialization functions:
    - initializes the user-mode scheduling infrastructure
    - creates the %windir%\temp folder.
    - creates a window station (Winsta0) and two desktops (Winlogon and Default) for processes to run on in session 0
    - creates __Services.exe__ (Service Control Manager or SCM) 
    - starts __Lsass.exe__ (Local Security Authentication Subsystem Server) 
    - starts __Lsm.exe__ (Local Session Manager)
    - waits forever for system shutdown

#### Service Control Manager (SCM)

- services are like UNIX _daemon processes_ or VMS _detached processes_ in that they can be
configured to start automatically at system boot time without requiring an interactive logon.
- can be started manually, most often, does not interact with the logged-on user.
- service control manager is a special system process running the image `%SystemRoot% \System32\Services.exe` that is responsible for starting, stopping, and interacting with service processes.
- there isn’t always one-to-one mapping between service processes and running services.

#### Local Session Manager (Lsm.exe)

- manages the state of terminal server sessions on the local machine.
- sends requests to Smss through the ALPC port _SmSsWinStationApiPort_ to start new sessions (for example, creating the Csrss and Winlogon processes) such as when a user selects _Switch User from Explorer_
- Lsm also communicates with Winlogon and Csrss (using a local system RPC)
- It notifies Csrss of events such as connect, disconnect, terminate, and broadcast system message. It receives notification from Winlogon for the following events:
    - Logon and logoff
    - Shell start and termination
    - Connect to a session
    - Disconnect from a session
    - Lock or unlock desktop

#### Winlogon, LogonUI, and Userinit

- the Windows logon process `%SystemRoot%\System32\Winlogon.exe` handles interactive user logons and logoffs.
- Winlogon is notified of a user logon request when the __secure attention sequence (SAS)__ keystroke combination is entered `CTRL+ALT+DEL`.
- identification and authentication aspects of the logon process are implemented through DLLs called __credential providers__.
- the standard Windows credential providers implement the default Windows authentication interfaces: password and smartcard. However, developers can provide their own credential providers.
- the logon dialog box run inside a child process of Winlogon called __LogonU__.
- once the user name and password have been captured, they are sent to the local security
authentication server process `%SystemRoot%\System32\Lsass.exe` to be authenticated. LSASS calls the appropriate authentication package (implemented as a DLL) to perform the actual verification, such as checking whether a password matches what is stored in the Active Directory or the SAM.
- __Userinit__ performs some initialization of the user environment (such as running the login script and applying group policies) and then looks in the registry at the Shell value (under the same Winlogon key referred to previously) and creates a process to run the system-defined shell (by default, __Explorer.exe__). Then Userinit exits leaving Explorer.exe with no parent.

## Chapter 3 System Mechanisms

### Trap Dispatching

- __interrupts__ and __exceptions__ are OS conditions that divert the processor to code outside the normal flow of control.
- the term __trap__ refers to a processor’s mechanism for capturing an executing thread when an exception or an interrupt occurs and transferring control to a fixed location in the OS.
- the processor transfers control to a __trap handler__, which is a function specific to a particular interrupt or exception.
- the kernel distinguishes between interrupts and exceptions in the following way:
    - an interrupt is an __asynchronous__ event (one that can occur at any time) that is unrelated to what the processor is executing.
        - are generated primarily by __I/O devices, processor clocks, or timers__, and they can be enabled or disabled. 
    - an exception, in contrast, is a __synchronous__ condition that usually results from the execution of a particular instruction.
        - running a program a second time with the same data under the same conditions can reproduce exceptions.
        - examples of exceptions include __memory-access violations__, __certain debugger instructions__, and __divideby-zero__ errors.
        - the kernel also regards system service calls as exceptions (although technically they’re system traps).
    <p align="center"><img src="https://i.imgur.com/UFazASp.png" width="400px" height="auto"></p>
- either hardware or software can generate exceptions and interrupts:
    - a __bus error__ exception is caused by a hardware problem, whereas a __divide-by-zero__ exception is the result of a software bug.
    - an __I/O device__ can generate an interrupt, or the kernel itself can issue a software interrupt such as an __APC or DPC__.
- when a hardware exception or interrupt is generated, the processor records enough machine state on the __kernel stack of the thread__ that’s interrupted to return to that point in the control flow and continue execution as if nothing had happened. If the thread was executing in user mode, Windows __switches to the thread’s kernel-mode stack__.
- Windows then creates a __trap frame__ on the kernel stack of the interrupted thread into which it stores the execution state of the thread
    ```c
    0: kd> dt nt!_ktrap_frame 
    +0x000 P1Home           : Uint8B
    +0x008 P2Home           : Uint8B
    +0x010 P3Home           : Uint8B
    +0x018 P4Home           : Uint8B
    +0x020 P5               : Uint8B
    +0x028 PreviousMode     : Char
    +0x029 PreviousIrql     : UChar
    +0x02a FaultIndicator   : UChar
    +0x02a NmiMsrIbrs       : UChar
    +0x02b ExceptionActive  : UChar
    +0x02c MxCsr            : Uint4B
    +0x030 Rax              : Uint8B
    ...
    ```

#### Interrupt Dispatching

- hardware-generated interrupts typically originate from I/O devices that must notify the processor when they need service.
    - interrupt-driven devices allow the OS to get the __maximum use out of the processor__ by overlapping central processing with I/O operations.
    - a thread starts an I/O transfer to or from a device and then can execute other useful work while the device completes the transfer.
    - when the device is finished, it interrupts the processor for service.
    - pointing devices, printers, keyboards, disk drives, and network cards are generally __interrupt driven__.
- the kernel installs interrupt trap handlers to respond to device interrupts.
    - those handlers transfer control either to an external routine (__Interrupt Service Routine ISR__) that handles the interrupt or ;
    - to an internal kernel routine that responds to the interrupt.
    - device drivers supply ISRs to service device interrupts, and the kernel provides interrupt-handling routines for other types of interrupts.

#### Hardware Interrupt Processing

- on the hardware platforms supported by Windows, external I/O interrupts come into one of the lines on an __interrupt controller__.
- the controller, in turn, interrupts the processor on a single line.
- once the processor is interrupted, it queries the controller to get the __interrupt request (IRQ)__.
- the interrupt controller translates the IRQ to an interrupt number, uses this number as an index into a structure called the __interrupt dispatch table (IDT)__, and transfers control to the appropriate interrupt dispatch routine.
- at system boot time, Windows fills in the IDT with pointers to the kernel routines that handle each interrupt and exception.
- Windows maps hardware IRQs to interrupt numbers in the IDT, and the system also uses the IDT to configure trap handlers for exceptions.
```c
kd> !idt

Dumping IDT: fffff8000a0d1000

00:	fffff80002be9100 nt!KiDivideErrorFaultShadow
01:	fffff80002be9180 nt!KiDebugTrapOrFaultShadow	Stack = 0xFFFFF8000A0D49E0
02:	fffff80002be9200 nt!KiNmiInterruptShadow	Stack = 0xFFFFF8000A0D47E0
03:	fffff80002be9280 nt!KiBreakpointTrapShadow
04:	fffff80002be9300 nt!KiOverflowTrapShadow
05:	fffff80002be9380 nt!KiBoundFaultShadow
06:	fffff80002be9400 nt!KiInvalidOpcodeFaultShadow
07:	fffff80002be9480 nt!KiNpxNotAvailableFaultShadow
08:	fffff80002be9500 nt!KiDoubleFaultAbortShadow	Stack = 0xFFFFF8000A0D43E0
09:	fffff80002be9580 nt!KiNpxSegmentOverrunAbortShadow
0a:	fffff80002be9600 nt!KiInvalidTssFaultShadow
0b:	fffff80002be9680 nt!KiSegmentNotPresentFaultShadow
0c:	fffff80002be9700 nt!KiStackFaultShadow
0d:	fffff80002be9780 nt!KiGeneralProtectionFaultShadow
0e:	fffff80002be9800 nt!KiPageFaultShadow
...
```
- each processor has a separate IDT so that different processors can run different ISRs, if appropriate.

#### x86 Interrupt Controllers

- most x86 systems rely on either the __i8259A Programmable Interrupt Controller (PIC)__ or a variant of the __i82489 Advanced Programmable Interrupt Controller (APIC)__
- today’s computers include an APIC.
- PIC:
    - works only on __uni-processors__ systems.
    - has only __eight__ interrupt lines
    - ibm pc arch extended it to __15__ interrupt lines (7 on master + 8 on slave).
- APIC:
    - work with __multiprocessor__ systems.
    - have __256__ interrupt lines. I
    - for compatibility, apic supports a pic mode.
    - consists of several components: 
        - an __I/O APIC__ that receives interrupts from devices
        - __local APICs__ that receive interrupts from the I/O APIC on the bus and that interrupt the CPU they are associated with
        - an i8259A-compatible interrupt controller that translates APIC input into PIC-equivalent signals.
        <p align="center"><img src="https://i.imgur.com/w14WT37.png" width="300px" height="auto"></p>
- x64 Windows will not run on systems that do not have an APIC because they use the APIC for interrupt control.
- IA64 architecture relies on the __Streamlined Advanced Programmable Interrupt Controller (SAPIC)__, which is an evolution of the APIC. Even if __load balancing__ and __routing__ are present in the firmware, Windows does not take advantage of it; instead, it __statically__ assigns interrupts to processors in a __round-robin__ manner.
```c
0: kd> !pic
----- IRQ Number ----- 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
Physically in service:  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
Physically masked:      Y  Y  Y  Y  Y  Y  Y  Y  Y  Y  Y  Y  Y  Y  Y  Y
Physically requested:   Y  .  .  .  .  .  .  .  .  .  .  .  Y  .  .  .
Level Triggered:        .  .  .  .  .  .  .  Y  .  Y  Y  Y  .  .  .  .


0: kd> !apic
Apic @ fffe0000  ID:0 (60015)  LogDesc:01000000  DestFmt:ffffffff  TPR F0
TimeCnt: 00000000clk  SpurVec:3f  FaultVec:e3  error:0
Ipi Cmd: 02000000`00000c00  Vec:00  NMI       Lg:02000000      edg high       
Timer..: 00000000`000300fd  Vec:FD  FixedDel    Dest=Self      edg high      m
Linti0.: 00000000`0001003f  Vec:3F  FixedDel    Dest=Self      edg high      m
Linti1.: 00000000`000004ff  Vec:FF  NMI         Dest=Self      edg high       
TMR: 66, 76, 86, 96, B1
IRR: 2F, 96, D1
ISR: D1

0: kd> !ioapic
IoApic @ FEC00000  ID:2 (20)  Arb:2000000
Inti00.: 00000000`000100ff  Vec:FF  FixedDel  Ph:00000000      edg high      m
Inti01.: 03000000`00000981  Vec:81  LowestDl  Lg:03000000      edg high       
Inti02.: 01000000`000008d1  Vec:D1  FixedDel  Lg:01000000      edg high       
Inti03.: 00000000`000100ff  Vec:FF  FixedDel  Ph:00000000      edg high      m
Inti04.: 03000000`00000961  Vec:61  LowestDl  Lg:03000000      edg high       
Inti05.: 00000000`000100ff  Vec:FF  FixedDel  Ph:00000000      edg high      m
Inti06.: 00000000`000100ff  Vec:FF  FixedDel  Ph:00000000      edg high      m
Inti07.: 00000000`000100ff  Vec:FF  FixedDel  Ph:00000000      edg high      m
Inti08.: 01000000`000009d2  Vec:D2  LowestDl  Lg:01000000      edg high       
Inti09.: 03000000`0000a9b1  Vec:B1  LowestDl  Lg:03000000      lvl low        
Inti0A.: 00000000`000100ff  Vec:FF  FixedDel  Ph:00000000      edg high      m
Inti0B.: 00000000`000100ff  Vec:FF  FixedDel  Ph:00000000      edg high      m
Inti0C.: 03000000`00000971  Vec:71  LowestDl  Lg:03000000      edg high       
Inti0D.: 00000000`000100ff  Vec:FF  FixedDel  Ph:00000000      edg high      m
Inti0E.: 03000000`00000975  Vec:75  LowestDl  Lg:03000000      edg high       
Inti0F.: 03000000`00000965  Vec:65  LowestDl  Lg:03000000      edg high       
Inti10.: 03000000`0000a976  Vec:76  LowestDl  Lg:03000000      lvl low        
Inti11.: 03000000`0000a986  Vec:86  LowestDl  Lg:03000000      lvl low        
Inti12.: 03000000`0000e966  Vec:66  LowestDl  Lg:03000000      lvl low  rirr  
Inti13.: 03000000`0000e996  Vec:96  LowestDl  Lg:03000000      lvl low  rirr  
Inti14.: 00000000`000100ff  Vec:FF  FixedDel  Ph:00000000      edg high      m
Inti15.: 00000000`000100ff  Vec:FF  FixedDel  Ph:00000000      edg high      m
Inti16.: 00000000`000100ff  Vec:FF  FixedDel  Ph:00000000      edg high      m
Inti17.: 00000000`000100ff  Vec:FF  FixedDel  Ph:00000000      edg high      m
```

#### Software Interrupt Request Levels (IRQLs)

- although interrupt controllers perform interrupt prioritization, Windows __imposes__ its own interrupt __priority__ scheme known as __interrupt request levels (IRQLs)__.


## Chapter 10 Memory Management

## Introduction to the Memory Manager

- because the VAS might be larger or smaller than the PM on the machine, the memory manager has two primary tasks:
    - __translating, or mapping__, a process’s VAS into PM so that when a thread running in the context of that process reads or writes to the VAS, the correct physical address is referenced. (The subset of a process’s virtual address space that is physically resident is called the __working set__).
    - __paging__ some of the contents of memory to disk when it becomes overcommitted—that is, when running threads or system code try to use more PM than is currently available—and bringing the contents back into PM when needed.

### Memory Manager Components

- memory manager consists of the following components:
    - a set of executive system services for allocating, deallocating, and managing virtual memory, most of which are exposed through the Windows API or kernel-mode device driver interfaces.
    - __translation-not-valid__ and access fault trap handler for resolving hardware-detected memory management exceptions and making virtual pages resident on behalf of a process.
    - Six __key top-level routines__, each running in one of six different kernel-mode threads in the System process:
        - __balance set manager__ (`KeBalanceSetManager`, priority 16). It calls an inner routine, the __working set manager__ (`MmWorkingSetManager`), once per second as well as when free memory falls below a certain threshold. The working set manager drives the overall memory management policies, such as __working set trimming, aging, and modified page writing__. 
        - __process/stack swapper__ (`KeSwapProcessOrStack`, priority 23) performs both process and kernel thread stack inswapping and outswapping. The balance set manager and the thread-scheduling code in the kernel awaken this thread when an inswap or outswap operation needs to take place.
        - __modified page writer__ (`MiModifiedPageWriter`, priority 17) writes __dirty pages__ on the modified list back to the appropriate paging files. This thread is awakened when the size of the modified list needs to be reduced. 
        - __mapped page writer__ (`MiMappedPageWriter`, priority 17) writes dirty pages in mapped files to disk (or remote storage). It is awakened when the size of the modified list needs to be reduced or if pages for mapped files have been on the modified list for more than 5 minutes. This second modified page writer thread is necessary because it can generate page faults that result in requests for free pages. If there were no free pages and there was only one modified page writer thread, the system could deadlock waiting for free pages.
        - __segment dereference thread__ (`MiDereferenceSegmentThread`, priority 18) is responsible for cache reduction as well as for page file growth and shrinkage. (For example, if there is no VAS for paged pool growth, this thread trims the page cache so that the paged pool used to anchor it can be freed for reuse.)
        - __zero page thread__ (`MmZeroPageThread`, base priority 0) zeroes out pages on the free list so that a cache of zero pages is available to satisfy future demand-zero page faults. Unlike the other routines described here, this routine is not a top-level thread function but is called by the top-level thread routine Phase1Initialization. MmZeroPageThread never returns to its caller, so in effect the Phase 1 Initialization thread becomes the zero page thread by calling this routine. Memory zeroing in some cases is done by a faster function called `MiZeroInParallel`.

#### Internal Synchronization

- the memory manager is __fully reentrant__ and supports simultaneous execution on multiprocessor systems.
- achieved by using several different internal synchronization mechanisms, such as spinlocks, to control access to its own internal data structures. Some of theses accesses are:
    - Dynamically allocated portions of the system VAS
    - System working sets
    - Kernel memory pools
    - The list of loaded drivers
    - The list of paging files
    - Physical memory lists
    - Image base randomization (ASLR) structures
    - Each individual entry in the page frame number (PFN) database.

#### Examining Memory Usage

- __vmmap__, __rammap__ or __process explorer__ are nice tools to examine memory usage.
    ```c
        0: kd> !vm
    Page File: \??\C:\pagefile.sys
    Current:   4193784 Kb  Free Space:   4193780 Kb
    Minimum:   4193784 Kb  Maximum:     12581352 Kb

    Physical Memory:          1048446 (    4193784 Kb)
    Available Pages:           820027 (    3280108 Kb)
    ResAvail Pages:            973277 (    3893108 Kb)
    Locked IO Pages:                0 (          0 Kb)
    Free System PTEs:        33494979 (  133979916 Kb)
    Modified Pages:              4801 (      19204 Kb)
    Modified PF Pages:           4771 (      19084 Kb)
    Modified No Write Pages:        0 (          0 Kb)
    NonPagedPool Usage:          9217 (      36868 Kb)
    NonPagedPool Max:          774140 (    3096560 Kb)
    PagedPool  0:               32876 (     131504 Kb)
    PagedPool  1:                4124 (      16496 Kb)
    PagedPool  2:                   0 (          0 Kb)
    PagedPool  3:                   0 (          0 Kb)
    PagedPool  4:                  57 (        228 Kb)
    PagedPool Usage:            37057 (     148228 Kb)
    PagedPool Maximum:       33554432 (  134217728 Kb)
    Processor Commit:             274 (       1096 Kb)
    Session Commit:              7647 (      30588 Kb)
    Syspart SharedCommit 0
    Shared Commit:               9386 (      37544 Kb)
    Special Pool:                   0 (          0 Kb)
    Kernel Stacks:               8220 (      32880 Kb)
    Pages For MDLs:              2331 (       9324 Kb)
    Pages For AWE:                  0 (          0 Kb)
    NonPagedPool Commit:            0 (          0 Kb)
    PagedPool Commit:           37108 (     148432 Kb)
    Driver Commit:               3082 (      12328 Kb)
    Boot Commit:                    0 (          0 Kb)
    System PageTables:              0 (          0 Kb)
    VAD/PageTable Bitmaps:       2214 (       8856 Kb)
    ProcessLockedFilePages:         0 (          0 Kb)
    Pagefile Hash Pages:            0 (          0 Kb)
    Sum System Commit:          70262 (     281048 Kb)
    Total Private:             107649 (     430596 Kb)
    Misc/Transient Commit:      55573 (     222292 Kb)
    Committed pages:           233484 (     933936 Kb)
    Commit limit:             2096416 (    8385664 Kb)
    ```

### Services Provided by the Memory Manager

- the Windows API has three groups of functions for managing memory in applications: 
    - __heap functions__ (_Heapxxx_ and the older interfaces _Localxxx_ and _Globalxxx_, which internally make use of the _Heapxxx_ APIs), which may be used for allocations __smaller than a page__;
    - virtual memory functions, which operate with page granularity (_Virtualxxx_);
    - and __memory mapped file functions__ (`CreateFileMapping`, `CreateFileMappingNuma`, `MapViewOfFile`, `MapViewOfFileEx`, and `MapViewOfFileExNuma`).

#### Large and Small Pages

- page size:

    | Architecture | Small Page  Size| Large Page Size | Small Pages per Large Page|
    |--------------|-----------------|-----------------|---------------------------|
    |x86 | 4 KB|  4 MB (2 MB if PAE enabled) |1,024 (512 with PAE) |
    |x64 | 4 KB  | 2 MB |  512 |
    |IA64 | 8 KB | 16 MB  |2,048 |
- primary advantage of large pages is __speed of address translation__ for references to other data within the large page.
- first reference to any byte within a large page will cause the hardware’s __translation look-aside buffer TLB__  to have in its cache the information necessary to translate references to any other byte within the large page.
- To take advantage of large pages on systems with more than 2 GB of RAM, Windows maps with large pages:
    - the core OS images (`Ntoskrnl.exe` and `Hal.dll`) 
    - as well as core OS data (such as the initial part of nonpaged pool and the data structures that describe the state of each physical memory page).
    - also automatically maps I/O space requests (calls by device drivers to `MmMapIoSpace`) with large pages if the request is of satisfactory large page length and alignment.
    - user mode apps can use `MEM_LARGE_PAGE` during mem alloc.
    - drivers can set _LargePageDrivers_.
- few notes regarding large pages:
    - allocating large pages could fail as freeing physical memory does become fragmented as the system runs.
    - tt is not possible to specify anything but __read/write__ access to large pages. 
    - the memory is also always __nonpageable__, because the page file system does not support large page.
    - if a large page contains, for example, both __read-only code__ and __read/write data__, the page must be marked as __read/write__, which means that the code will be writable. This means that device drivers or other kernel-mode code could, as a result of a bug, modify what is supposed to be __read-only__ OS or driver code without causing a memory access violation.

#### Reserving and Committing Pages

- pages in a process VAS are __free, reserved, committed, or shareable__. 
- __committed__ and __shareable__ pages are pages that, when accessed, ultimately translate to valid pages in physical memory.
- __shared pages__ are usually mapped to a view of a section, which in turn is part or all of a file, but may instead represent a portion of page file space.
    - all shared pages can potentially be shared with other processes.
    - sections are exposed in the Windows API as __file mapping objects__.

#### Commit Limit

- __commitment or commit charge__; this is the first of the two numbers, which represents the total of all committed virtual memory in the system.
- __system commit limit__ or simply the commit limit, on the amount of committed virtual memory that can exist at any one time.
    - corresponds to the current total size of all paging files, plus the amount of RAM that is usable by the OS.
    <p align="center"><img src="https://i.imgur.com/WwRPq6Y.png" width="400px" height="auto"></p>

#### Locking Memory

- pages can be locked in memory in two ways:
    - Windows applications can call the `VirtualLock` function to lock pages in their process working set.
        - pages locked using this mechanism remain in memory until explicitly unlocked or until the process that locked them terminates. 
        - the number of pages a process can lock __can’t exceed its minimum working set size minus eight pages__.
        - therefore, if a process needs to lock more pages, it can increase its working set minimum with the `SetProcessWorkingSetSizeEx`.
    - device drivers can call the kernel-mode functions `MmProbeAndLockPages`, `MmLockPagableCodeSection`, `MmLockPagableDataSection`, or `MmLockPagableSectionByHandle`.
        - pages locked using this mechanism remain in memory until explicitly unlocked. 
        - the last three of these APIs __enforce no quota__ on the number of pages that can be locked in memory because the resident available page charge is obtained when the driver first loads; this ensures that it can never cause a system crash due to overlocking.
        - for the first API, quota charges must be obtained or the API will return a failure status.

#### Allocation Granularity

- Windows aligns __each region__ of reserved process address space to begin on an integral boundary defined by the value of the system allocation granularity, which can be retrieved from the Windows `GetSystemInfo` or `GetNativeSystemInfo` function.
- this value is __64 KB__, a granularity that is used by the memory manager to efficiently allocate metadata (for example, VADs, bitmaps, and so on) to support various process operations.
- Windows kernel-mode code isn’t subject to the same restrictions; it can reserve memory on a __single-page granularity__ (although this is not exposed to device drivers for the reasons detailed earlier).
- Windows ensures that the __size and base__ of the region is a __multiple__ of the system page size, whatever that might be.

#### Shared Memory and Mapped Files

- __shared memory__ can be defined as memory that is visible to more than one process or that is present in more than one process VAS.
-  for example, if two processes use the same DLL, it would make sense to load the referenced code pages for that DLL into physical memory __only once__ and share those pages between all processes that map the DLL. <p align="center"><img src="https://i.imgur.com/ORc9BlC.png" width="300px" height="auto"></p>
- a __section object__ can be connected to an __open file on disk (called a mapped file)__ or __to committed memory (to provide shared memory)__.
- sections mapped to committed memory are called __page-file-backed sections__ because the pages are written to the paging file (__as opposed to a mapped file__) if demands on physical memory require it.
- as with any other empty page that is made visible to user mode (such as private committed pages), shared committed pages are __always zero-filled__ when they are first accessed to ensure that no sensitive data is ever leaked.
- to create a section object, call the Windows `CreateFileMapping` or `CreateFileMappingNuma` function, specifying the file handle to map it to (or __INVALID_HANDLE_VALUE__ for a page-file-backed section).
- a section object can refer to files that are __much larger__ than can fit in the address space of a process.
- to access a very large section object, a process can map only the portion of the section object that it requires (called a view of the section) by calling the `MapViewOfFile`, `MapViewOfFileEx`, or `MapViewOfFileExNuma` function and then specifying the range to map.
- mapping views permits processes to __conserve address space__ because only the views of the section object needed at the time must be mapped into memory.

#### Protecting Memory

- Windows provides memory protection in four primary ways.
    1. all systemwide data structures and memory pools used by kernel-mode system components can be accessed only while in kernel mode.
        - user-mode threads can’t access these pages
        - if they attempt to do so, the hardware generates a fault, which in turn the memory manager reports to the thread as an access violation.
    2. each process has a separate, __private address space__, protected from being accessed by any thread belonging to another process. 
        - even shared memory is not really an exception to this because each process accesses the shared regions using addresses that are part of its __own VAS__.
    3. in addition to the implicit protection virtual-to-physical address translation offers, all processors supported by Windows provide some form of __hardware-controlled memory protection__ (such as read/write, read-only, and so on).
    4. shared memory section objects have standard Windows __ACLs__ that are checked when processes attempt to open them, thus limiting access of shared memory to those processes with the proper rights.

#### No Execute Page Protection

- __No execute__ page protection (also referred to as __data execution prevention (DEP)__) causes an attempt to transfer control to an instruction in a page marked as “no execute” to generate an access fault.
    - prevent stack-based overflows
    - catch poorly written programs that don’t correctly set permissions on pages from which they intend to execute code.
- in a illegal reference:
    - in kernel, fails with `ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY` bugcheck code.
    - in user mode, fails with `STATUS_ACCESS_VIOLATION` exception.
- on 32-bit x86 systems that support DEP, bit 63 in the page table entry (PTE) is used to mark a page as nonexecutable.
    - DEP feature is available only when the processor is running in __Physical Address Extension (PAE)__ mode.
    - requires loading the PAE kernel `%SystemRoot%\System32\Ntkrnlpa.exe`.
    - the OS loader automatically loads the PAE kernel on 32-bit systems that support hardware DEP.
    - applied only to __thread stacks__ and __usermode pages__, not to paged pool and session pool.
    - processes depends on the value of the BCD nx option (OptIn, OptOut, AlwaysOn, AlwaysOff)
- on 64-bit versions of Windows,:
    - execution protection is __always applied to all 64-bit processes and device drivers__.
    - can be disabled only by setting the nx BCD option to _AlwaysOff_.
    - applied to __thread stacks__ (both user and kernel mode), __usermode pages__ not specifically marked as __executable__, __kernel paged pool__, and __kernel session pool__.
- on Windows client versions (both 64-bit and 32-bit) execution protection for 32-bit processes is configured by default to apply only to __core Windows operating system executables (OptIn)__.
- even if you force DEP to be enabled, there are still other methods through which applications can disable DEP for their own image:
    - image loader will verify the signature of the executable against known __copy-protection mechanisms__ (such as SafeDisc and SecuROM) and disable execution protection to provide compatibility with older copy protected software such as computer games.
- if the system is in _OptIn_ or _OptOut_ mode and executing a 32-bit process, the `SetProcessDEPPolicy` function allows a process to dynamically disable DEP or to permanently enable it.

#### Software Data Execution Prevention

- for older processors that do not support hardware no execute protection, Windows supports limited __software DEP__.
    - reduces SEH exploits.
    - if image is not built with __/SAFESEH__, software DEP guards against SEH exploits with __Structured Exception Handler Overwrite Protection (SEHOP)__.
    - __ALSR__ make it harder to know the location of the function pointed to by the symbolic exception registration record.
- to futher mitiguate again SEH exploits when /SAFESEH is not present:
    - a mechanism called __Image Dispatch Mitigation__ ensures that the SEH handler is located within the same image section as the function that raised an exception, which is normally the case for most programs (although not necessarily, since some DLLs might have exception handlers that were set up by the main executable, which is why this mitigation is off by default).
    - Executable Dispatch Mitigation further makes sure that the SEH handler is located within an executable page—a less strong requirement than Image Dispatch Mitigation, but one with fewer compatibility issues.
- two other methods for software DEP that the system implements are:
    - __stack cookies__: relies on the compiler to insert special code at the beginning and end of each potentially exploitable function.
        - the code saves a special numerical value (the cookie) on the stack on entry and validates the cookie’s value before returning to the caller saved on the stack.
        - the cookie value is computed for each boot when executing the first user-mode thread, and it is saved in the `KUSER_SHARED_DATA` structure. The image loader reads this value and initializes it when a process starts executing in user mode.
    - the cookie value that is calculated is also saved for use with the `EncodeSystemPointer` and `DecodeSystemPointer` APIs, which implement pointer encoding
        - static pointers that are dynamically called runs the risk of having malicious code __overwrite the pointer values__ with code that the malware controls. 
        -  These APIs provide similar protection but with a __per-process cookie__ (created on demand) instead of a __per-system cookie__.

#### Copy-on-Write

- copy-on-write page protection is an optimization the memory manager uses to conserve physical memory.
- defers making a copy of the pages until the page is written to.
- two processes are sharing three pages, each marked copy-on-write, but neither of the two processes has attempted to modify any data on the pages.
 <p align="center"><img src="https://i.imgur.com/qbiqSC3.png" width="400px" height="auto"></p>
- it a thread in either process writes to a page, a memory management fault is generated. 
    - memory manager sees that the write is to a copy-on-write page,
    - it allocates a new read/write page in physical memory,
    - copies the contents of the original page to the new page
    - updates the corresponding page-mapping information in this process to point to the new location,
    - the newly copied page is now private to the process that did the writing and isn’t visible to the other process still sharing the copy-on-write page.
    <p align="center"><img src="https://i.imgur.com/Hx0IPIE.png" width="400px" height="auto"></p>

#### Address Windowing Extensions

- an app that needs to make more than 2 GB (or 3 GB) of data easily available in a single process could do so via __file mapping__, remapping a part of its address space into various portions of a large file.
- Windows provides a set of functions called __Address Windowing Extensions (AWE)__ to allow a 32-bits process to allocate more physical memory than can be represented in its VAS.
- it then can access the physical memory by mapping a portion of its VAS into selected portions of the physical memory at various times.
- allocating and using memory via the AWE functions is done in three steps:
    1. allocating the physical memory to be used:  `AllocateUserPhysicalPages` or `AllocateUserPhysicalPagesNuma`.
    2. creating one or more regions of VAS to act as windows to map views of the physical memory. The application uses the Win32 `VirtualAlloc(Ex)`, or `VirtualAllocExNuma` function with the __MEM_PHYSICAL__ flag. 
    3. to actually use the memory, the app uses `MapUserPhysicalPages` or `MapUserPhysicalPagesScatter` to map a portion of the physical region allocated in step 1 into one of the virtual regions, or windows, allocated in step 2.
    <p align="center"><img src="https://i.imgur.com/UD6vywq.png" width="450px" height="auto"></p>

### Kernel-Mode Heaps (System Memory Pools)

- at system initialization, the memory manager creates two dynamically sized memory pools, or heaps, that most kernel-mode components use to allocate system memory:
    - __nonpaged pool__:  consists of ranges of system virtual addresses that are guaranteed to reside in physical memory at all times + can be accessed from any IRQL.
    - __paged pool__:  a region of virtual memory in system space that can be paged into and out of the system. Device drivers that don’t need to access the memory from DPC/dispatch level or above can use paged pool. It is accessible from any process context.
- systems start with __four paged pools__ and __one nonpaged pool__; more are created, up to a maximum of __64__, depending on the number of NUMA nodes on the system.
- in addition to paged/nonpaged pool, there is a pool region in __session space__, which is used for data that is common to all processes in the session.
- also, a __special pool__:  allocations from special pool are surrounded by pages marked as no-access to help isolate problems in code that accesses memory before or after the region of pool it allocated.

#### Pool Sizes

- initial size: 3% of ram, if less than 40 MB, use 40 MB insted as long as 10% of RAM results in more than 40MB; otherwise 10 percent of RAM is chosen as a minimum.

|Pool Type | Maximum on 32-Bit | Maximum on 64-Bit Systems |
|----------|-------------------|---------------------------|
|Nonpaged | 75% of physical memory or 2 GB, whichever is smaller | 75% of physical memory or 128 GB, whichever is smaller|
| Paged | 2 GB | 128 GB |

```c
Page File: \??\C:\pagefile.sys
  Current:   4193784 Kb  Free Space:   4193780 Kb
  Minimum:   4193784 Kb  Maximum:     12581352 Kb

Physical Memory:          1048446 (    4193784 Kb)
Available Pages:           826358 (    3305432 Kb)
ResAvail Pages:            974026 (    3896104 Kb)
Locked IO Pages:                0 (          0 Kb)
Free System PTEs:        33496170 (  133984680 Kb)
Modified Pages:              4825 (      19300 Kb)
Modified PF Pages:           4808 (      19232 Kb)
Modified No Write Pages:        0 (          0 Kb)
NonPagedPool Usage:          9111 (      36444 Kb)
NonPagedPool Max:          774140 (    3096560 Kb)
PagedPool  0:               32829 (     131316 Kb)
PagedPool  1:                4109 (      16436 Kb)
PagedPool  2:                   0 (          0 Kb)
PagedPool  3:                   0 (          0 Kb)
PagedPool  4:                  54 (        216 Kb)
PagedPool Usage:            36992 (     147968 Kb)
PagedPool Maximum:       33554432 (  134217728 Kb)
Processor Commit:             176 (        704 Kb)
Session Commit:              7651 (      30604 Kb)
Syspart SharedCommit 0
Shared Commit:               9231 (      36924 Kb)
Special Pool:                   0 (          0 Kb)
Kernel Stacks:               7707 (      30828 Kb)
Pages For MDLs:              2331 (       9324 Kb)
Pages For AWE:                  0 (          0 Kb)
NonPagedPool Commit:            0 (          0 Kb)
PagedPool Commit:           36992 (     147968 Kb)
Driver Commit:               3082 (      12328 Kb)
Boot Commit:                    0 (          0 Kb)
System PageTables:              0 (          0 Kb)
VAD/PageTable Bitmaps:       1993 (       7972 Kb)
ProcessLockedFilePages:         0 (          0 Kb)
Pagefile Hash Pages:            0 (          0 Kb)
Sum System Commit:          69163 (     276652 Kb)
Total Private:             102655 (     410620 Kb)
Misc/Transient Commit:      55148 (     220592 Kb)
Committed pages:           226966 (     907864 Kb)
Commit limit:             2096416 (    8385664 Kb)
```

- use _poolmon_ to monitor pool usage; or:
```c
 kd> !poolused 2

*** CacheSize too low - increasing to 128 MB

Max cache size is       : 134217728 bytes (0x20000 KB) 
Total memory in cache   : 157892 bytes (0x9b KB) 
Number of regions cached: 292
2257 full reads broken into 2626 partial reads
    counts: 2241 cached/385 uncached, 85.34% cached
    bytes : 371524 cached/139204 uncached, 72.74% cached
** Transition PTEs are implicitly decoded
** Prototype PTEs are implicitly decoded
..
 Sorting by NonPaged Pool Consumed

               NonPaged                  Paged
 Tag     Allocs         Used     Allocs         Used

 Cont      1094      6525216          0            0	Contiguous physical memory allocations for device drivers 
 EtwB       128      5041136          4       163840	Etw Buffer , Binary: nt!etw
 AmlC        31      2031616          0            0	ACPI AMLI Pooltags 
 Pool         5      1705552          0            0	Pool tables, etc. 
 AmlH         3      1572864          0            0	ACPI AMLI Pooltags 
 ...

 kd> !poolused 4
..
 Sorting by Paged Pool Consumed

               NonPaged                  Paged
 Tag     Allocs         Used     Allocs         Used

 CM31         0            0      19797     91705344	Internal Configuration manager allocations , Binary: nt!cm
 CM25         0            0       2764     12546048	Internal Configuration manager allocations , Binary: nt!cm
 MmRe         0            0        958     11071552	ASLR relocation blocks , Binary: nt!mm
 MmSt         0            0       2808      6790672	Mm section object prototype ptes , Binary: nt!mm
 CcPD         0            0          8      4314976	Prefetcher trace dump , Binary: nt!ccpf
 CIcr         0            0         89      2976080	Code Integrity allocations for image integrity checking , Binary: ci.dll
 Ntff         5         1600       2109      2598288	FCB_DATA , Binary: ntfs.sys
 ...
```

### Look-Aside Lists

- Windows provides a fast memory allocation mechanism called __look-aside lists__.
- the difference between pools and look-aside lists:
    - general pool allocations can __vary__ in size, a look-aside list contains only __fixed-sized__ blocks. 
    - although the general pools are more flexible in terms of what they can supply, look-aside lists are __faster__ because they don’t use any __spinlocks__.
- contents and sizes of the various system look-aside lists:
```c
1: kd> !lookaside

Lookaside "nt!CcTwilightLookasideList" @ 0xfffff80002c49280  Tag(hex): 0x6b576343 "CcWk"
    Type           =       0000  NonPagedPool
    Current Depth  =          0  Max Depth  =         34
    Size           =         32  Max Alloc  =       1088
    AllocateMisses =        534  FreeMisses =        467
    TotalAllocates =        759  TotalFrees =        715
    Hit Rate       =         29% Hit Rate   =         34%

Lookaside "nt!IopSmallIrpLookasideList" @ 0xfffff80002c49d00  Tag(hex): 0x73707249 "Irps"
    Type           =       0000  NonPagedPool
    Current Depth  =          0  Max Depth  =          4
    Size           =        280  Max Alloc  =       1120
    AllocateMisses =        217  FreeMisses =         66
    TotalAllocates =        263  TotalFrees =        113
    Hit Rate       =         17% Hit Rate   =         41%
...

Total NonPaged currently allocated for above lists =        0
Total NonPaged potential for above lists           =    75624
Total Paged currently allocated for above lists    =        0
Total Paged potential for above lists              =   367832
```

### Heap Manager

- manages allocations inside __larger memory areas__ reserved using the page granularity memory allocation functions.
- the allocation granularity in the heap manager is relatively small: __8 bytes__ on 32-bit systems, and __16 bytes__ on 64-bit. systems.
- legacy APIs (prefixed with either __Local__ or __Global__) are provided to support older Windows applications, which also internally call the heap manager, using some of its specialized interfaces to support legacy behavior.

#### Types of Heaps

- each process has at least one heap: the __default process heap__.
- defaults to 1 MB in size, but it can be made bigger by using the `/HEAP` linker flag
- this size is just the initial reserve, however—it will expand automatically as needed.
- processes can also create additional __private heaps__ with the `HeapCreate`.

#### Heap Manager Structure

- the heap manager is structured in two layers: an __optional front-end layer__ and the __core heap__.
- the core heap handles the basic functionality and is mostly common across the usermode and kernel-mode heap implementations. 
- the core functionality includes the management of blocks inside segments, the management of the segments, policies for extending the heap, committing and decommitting memory, and management of the large blocks. <p align="center"><img src="https://i.imgur.com/pIABTFh.png" width="450px" height="auto"></p>
- for user-mode heaps only, an optional front-end heap layer can exist on top of the existing core functionality. 
- the only front-end supported on Windows is the __Low Fragmentation Heap (LFH)__. Only one front-end layer can be used for one heap at one time.

#### Heap Synchronization

- the heap manager supports __concurrent access__ from multiple threads by default.
- however, if a process is __single threaded__ or uses an external mechanism for __synchronization__, it can tell the heap manager to avoid the overhead of synchronization by specifying `HEAP_NO_SERIALIZE` either at heap creation or on a per-allocation basis.

#### The Low Fragmentation Heap

- many Windows apps have relatively small heap memory usage (~ less than 1 MB).
    - for this class of apps, the heap manager’s best-fit policy helps keep a low memory footprint for each process.
    - however, this strategy does not scale for __large processes__ and __multiprocessor__ machines.
    - in these cases, memory available for heap usage might be reduced as a result of __heap fragmentation__.
    - performance can suffer in scenarios where only certain sizes are often used concurrently from different threads scheduled to run on different processors.
    - this happens because several processors need to modify the same memory location (i.e the head of the look-aside list for that particular size) at the same time, thus causing significant contention for the corresponding cache line.
- LFH avoids fragmentation by managing allocated blocks in predetermined different block-size ranges called __buckets__. 
- LFH chooses the bucket that maps to the __smallest block__ large enough to hold the required size.

|Buckets |Granularity | Range |
|--------|------------|-------|
|1–32 | 8 | 1–256 |
|33–48 | 16 | 257–512 |
|49–64 |32 | 513–1,024 |
|65–80 | 64  |1,025–2,048 |
|81–96 | 128 | 2,049–4,096 |
|97–112  |256 | 4,097–8,194 |
|113–128  |512 | 8,195–16,384 |

#### Heap Security Features

- heap manager have some techniques to minimize heap-based exploits:
    - metadata used by the heap for internal management is packed with high degree of randomization
    - blocks are subjects to integrity check to the header to detect buffer overruns.
    - randomization of the base address (or handle).
- by using `HeapSetInformation` with _HeapEnableTerminationOnCorruption_ class, a process can opt in for an automatic termination in case if anomalies.
- dump metadata fields from a heap block:
```c
0:000> !heap -i 001a0000
Heap context set to the heap 0x001a0000
0:000> !heap -i 1e2570
Detailed information for block entry 001e2570
Assumed heap : 0x001a0000 (Use !heap -i NewHeapHandle to change)
Header content : 0x1570F4EC 0x0C0015BE (decoded : 0x07010006 0x0C00000D)
Owning segment : 0x001a0000 (offset 0)
Block flags : 0x1 (busy )
Total block size : 0x6 units (0x30 bytes)
Requested size : 0x24 bytes (unused 0xc bytes)
Previous block size: 0xd units (0x68 bytes)
Block CRC : OK - 0x7
Previous block : 0x001e2508
Next block : 0x001e25a0
```

#### Heap Debugging Features

- the heap manager leverages the 8 bytes used to store internal metadata as a consistency checkpoint which helps detecting potential heap errors and bugs:
    - __enable tail checking__: the end of the each block carries a signature that is checked when the block is released.
    - __enable free checking__: free block is filled with a pattern that is checked at various points when the heap manager needs to access the block.
    - __parameter checking__: extensive checking of the parameteres passed to the heap functions.
    - __heap validation__ the entire heap is validated at each heap call.
    - __heap tagging and strack traces support__: supports specifying tags for allocation and/or captures user-mode stack traces for the heap calls to help narrow the possible causes of a heap error.

#### Pageheap

- because the tail and free checking might be discovering corruptions that occurred well before the problem was detected, an additional heap debugging capability, called __pageheap__, is provided that directs all or part of the heap calls to a different heap manager.

#### Fault Tolerant Heap

- Windows includes a feature called the __fault tolerant heap (FTH)__ in an attempt to mitigate heap corruption problems.
- FTH is implemented in two primary components:
    - the detection component, or FTH server, and;
    - the mitigation component, or FTH client.
- the FTH client is an application compatibility shim, it intercepts the calls to the heap routines and redirects them to its own code. The FTH code implements a number of “mitigations” that attempt to allow the application to survive despite various heap-related errors.
- the activity of the fault tolerant heap can be observed in the Event Viewer.

### Virtual Address Space Layouts

- three main types of data are mapped into the VAS in Windows:
    - per-process private code and data
    - sessionwide code and data, 
    - systemwide code and data.
- The information that describes the process VAS, called __page tables__.
    - each process bas its pwn set of page tables.
    - they are stored in kernel mode.
- __session space__ contains information that is common to each session:
    - consists of the processes and other system objects (such as the window station, desktops, and windows) that represent a single user’s logon session
    - each session has a session-specific paged pool area used by Win32k.sys to allocate session-private GUI data structures. 
    - each session has its own copy of the Windows subsystem process (Csrss.exe) and logon process (Winlogon.exe).
- __system space__ contains global OS code and data structures visible by kernel-mode code regardless of which process is currently executing. System space consists of the following components: 
    - __system code__ contains the OS image, HAL, and device drivers used to boot the system.
    - __nonpaged pool__: nonpageable system memory heap.
    - __paged pool__: pageable system memory heap.
    - __system cache__: VAS used to map files open in the system cache.
    - __system page table entries (PTEs)__: pool of system PTEs used to map system pages such as I/O space, kernel stacks, and memory descriptor lists.
    - __system working set lists__: the working set list data structures that describe the three system working sets (the system cache working set, the paged pool working set, and the system PTEs working set).
    - __system mapped views__: used to map Win32k.sys, the loadable kernel-mode part of the Windows subsystem, as well as kernel-mode graphics drivers it uses. 
    - __hyperspace__: a special region used to map the process working set list and other per-process data that doesn’t need to be accessible in arbitrary process context. Hyperspace is also used to temporarily map physical pages into the system space. One example of this is invalidating page table entries in page tables of processes other than the current one (such as when a page is removed from the standby list).
    - __crash dump information__: reserved to record information about the state of a system crash.
    - __HAL__ usage System memory reserved for HAL-specific structures.

#### x86 Address Space Layouts

- x86 virtual address space layouts: <p align="center"><img src="https://i.imgur.com/eb4oAzh.png" width="450px" height="auto"></p>

#### x86 System Address Space Layout

- 32-bit versions of Windows implement a __dynamic system address space__ layout by using a virtual address allocator.
- many kernel-mode structures use dynamic address
space allocation. These structures are therefore not necessarily __virtually contiguous__ with themselves:
    - Nonpaged pool
    - Special pool
    - Paged pool
    - System page table entries (PTEs)
    - System mapped views
    - File system cache
    - File system structures (metadata)
    - Session space

#### x86 session space

- for systems with __multiple__ sessions, the code and data unique to each session are mapped into system
address space but __shared__ by the processes in that session. <p align="center"><img src="https://i.imgur.com/VmYnKuG.png" width="260px" height="auto"></p>
- you can list the active sessions with the `!session` command as follows:
```c
lkd> !session
Sessions on machine: 3
Valid Sessions: 0 1 3
Current Session 1
```
-  you can set the active session using the `!session –s` command and display the address of the session data structures and the processes in that session with the `!sprocess` command:
```c
0: kd> !session -s 1
Sessions on machine: 2
Implicit process is now fffffa80`050acb00
.cache forcedecodeptes done
Using session 1l

0: kd> !sprocess
Dumping Session 1

_MM_SESSION_SPACE fffff88004553000
_MMSESSION        fffff88004553b40
PROCESS fffffa80050acb00
    SessionId: 1  Cid: 0198    Peb: 7fffffdd000  ParentCid: 0188
    DirBase: 93f80000  ObjectTable: fffff8a000e8ca20  HandleCount: 369.
    Image: csrss.exe

PROCESS fffffa80050e45c0
    SessionId: 1  Cid: 01d4    Peb: 7fffffdb000  ParentCid: 0188
    DirBase: a0806000  ObjectTable: fffff8a000ea7f90  HandleCount: 126.
    Image: winlogon.exe

PROCESS fffffa80054d2b00
    SessionId: 1  Cid: 0558    Peb: 7fffffd3000  ParentCid: 0208
    DirBase: 96537000  ObjectTable: fffff8a00b5a3e20  HandleCount: 220.
    Image: taskhost.exe

PROCESS fffffa8005567b00
    SessionId: 1  Cid: 060c    Peb: 7fffffdf000  ParentCid: 036c
    DirBase: 95260000  ObjectTable: fffff8a001f4b9c0  HandleCount:  74.
    Image: dwm.exe

PROCESS fffffa80055f2b00
    SessionId: 1  Cid: 0658    Peb: 7fffffd8000  ParentCid: 05fc
    DirBase: 9448e000  ObjectTable: fffff8a000f60d70  HandleCount: 756.
    Image: explorer.exe

PROCESS fffffa8005665b00
    SessionId: 1  Cid: 0730    Peb: 7fffffda000  ParentCid: 03b8
    DirBase: a7aea000  ObjectTable: fffff8a001fabce0  HandleCount:  85.
    Image: taskeng.exe

PROCESS fffffa800571bb00
    SessionId: 1  Cid: 0784    Peb: 7fffffd5000  ParentCid: 0658
    DirBase: 8f1e3000  ObjectTable: fffff8a001a4c0f0  HandleCount: 252.
    Image: vmtoolsd.exe
...
```
- to view the details of the session, dump the `MM_SESSION_SPACE` structure using the dt command, as follows:
```c
0: kd> dt nt!_MM_SESSION_SPACE fffff88004553000
   +0x000 ReferenceCount   : 0n17
   +0x004 u                : <unnamed-tag>
   +0x008 SessionId        : 1
   +0x00c ProcessReferenceToSession : 0n18
   +0x010 ProcessList      : _LIST_ENTRY [ 0xfffffa80`050acce0 - 0xfffffa80`05c45c10 ]
   +0x020 LastProcessSwappedOutTime : _LARGE_INTEGER 0x0
   +0x028 SessionPageDirectoryIndex : 0xa1704
   +0x030 NonPagablePages  : 0x72
   +0x038 CommittedPages   : 0x1761
   +0x040 PagedPoolStart   : 0xfffff900`c0000000 Void
   +0x048 PagedPoolEnd     : 0xfffff920`bfffffff Void
   +0x050 SessionObject    : 0xfffffa80`050a7390 Void
   +0x058 SessionObjectHandle : 0xffffffff`800001dc Void
   +0x060 ResidentProcessCount : 0n17
   +0x064 SessionPoolAllocationFailures : [4] 0
   +0x078 ImageList        : _LIST_ENTRY [ 0xfffffa80`050aa930 - 0xfffffa80`050b5480 ]
   +0x088 LocaleId         : 0x409
   +0x08c AttachCount      : 0
   +0x090 AttachGate       : _KGATE
   +0x0a8 WsListEntry      : _LIST_ENTRY [ 0xfffff800`02c7c3d0 - 0xfffff880`037bb0a8 ]
   +0x0c0 Lookaside        : [21] _GENERAL_LOOKASIDE
   +0xb40 Session          : _MMSESSION
   +0xb98 PagedPoolInfo    : _MM_PAGED_POOL_INFO
   +0xc00 Vm               : _MMSUPPORT
   +0xc90 Wsle             : 0xfffff900`00812488 _MMWSLE
   +0xc98 DriverUnload     : 0xfffff960`001b2d78     void  +fffff960001b2d78
   +0xcc0 PagedPool        : _POOL_DESCRIPTOR
   +0x1e00 PageDirectory    : _MMPTE
   +0x1e08 SessionVaLock    : _KGUARDED_MUTEX
   +0x1e40 DynamicVaBitMap  : _RTL_BITMAP
   +0x1e50 DynamicVaHint    : 0xf
   +0x1e58 SpecialPool      : _MI_SPECIAL_POOL
   +0x1ea0 SessionPteLock   : _KGUARDED_MUTEX
   +0x1ed8 PoolBigEntriesInUse : 0n263
   +0x1edc PagedPoolPdeCount : 0xc
   +0x1ee0 SpecialPoolPdeCount : 0
   +0x1ee4 DynamicSessionPdeCount : 0x1b
   +0x1ee8 SystemPteInfo    : _MI_SYSTEM_PTE_TYPE
   +0x1f30 PoolTrackTableExpansion : (null) 
   +0x1f38 PoolTrackTableExpansionSize : 0
   +0x1f40 PoolTrackBigPages : 0xfffffa80`050ad000 Void
   +0x1f48 PoolTrackBigPagesSize : 0x200
   +0x1f50 IoState          : 6 ( IoSessionStateLoggedOn )
   +0x1f54 IoStateSequence  : 7
   +0x1f58 IoNotificationEvent : _KEVENT
   +0x1f70 CreateTime       : 0xbdfbd3d
   +0x1f78 CpuQuotaBlock    : (null) 
```

- to view session space memory utilization:
```c
0: kd> !vm 4
Page File: \??\C:\pagefile.sys
  Current:   4193784 Kb  Free Space:   4181660 Kb
  Minimum:   4193784 Kb  Maximum:     12581352 Kb
...

Terminal Server Memory Usage By Session:

Session ID 0 @ fffff88002dfa000:
Paged Pool Usage:     3060 Kb
NonPaged Usage:        316 Kb
Commit Usage:         6484 Kb

Session ID 1 @ fffff88004536000:
Paged Pool Usage:        0 Kb
NonPaged Usage:        324 Kb
Commit Usage:          324 Kb
...
```

#### System Page Table Entries

- are used to dynamically map system pages such as __I/O space, kernel stacks__, and the mapping for __memory descriptor lists__.
- to see how many system PTEs are available:
```c
0: kd> !sysptes
System PTE Information
Total System Ptes 307168
starting PTE: c0200000
free blocks: 32 total free: 3856 largest free block: 542

Kernel Stack PTE Information
Unable to get syspte index array - skipping bins
starting PTE: c0200000
free blocks: 165 total free: 1503 largest free block: 75
0: kd> ? nt!MiSystemPteInfo
Evaluate expression: -2100014016 = 82d45440
0: kd> dt _MI_SYSTEM_PTE_TYPE 82d45440
nt!_MI_SYSTEM_PTE_TYPE
+0x000 Bitmap : _RTL_BITMAP
+0x008 Flags : 3
+0x00c Hint : 0x2271f
+0x010 BasePte : 0xc0200000 _MMPTE
+0x014 FailureCount : 0x82d45468 -> 0
+0x018 Vm : 0x82d67300 _MMSUPPORT
+0x01c TotalSystemPtes : 0n7136
+0x020 TotalFreeSystemPtes : 0n4113
+0x024 CachedPteCount : 0n0
+0x028 PteFailures : 0
+0x02c SpinLock : 0
+0x02c GlobalMutex : (null)
```

- use `!sysptes 4` to show a list of allocators:

```c
lkd>!sysptes 4
0x1ca2 System PTEs allocated to mapping locked pages
VA MDL PageCount Caller/CallersCaller
ecbfdee8 f0ed0958 2 netbt!DispatchIoctls+0x56a/netbt!NbtDispatchDevCtrl+0xcd
f0a8d050 f0ed0510 1 netbt!DispatchIoctls+0x64e/netbt!NbtDispatchDevCtrl+0xcd
ecef5000 1 20 nt!MiFindContiguousMemory+0x63
ed447000 0 2 Ntfs!NtfsInitializeVcb+0x30e/Ntfs!NtfsInitializeDevice+0x95
ee1ce000 0 2 Ntfs!NtfsInitializeVcb+0x30e/Ntfs!NtfsInitializeDevice+0x95
ed9c4000 1 ca nt!MiFindContiguousMemory+0x63
eda8e000 1 ca nt!MiFindContiguousMemory+0x63
efb23d68 f8067888 2 mrxsmb!BowserMapUsersBuffer+0x28
efac5af4 f8b15b98 2 ndisuio!NdisuioRead+0x54/nt!NtReadFile+0x566
f0ac688c f848ff88 1 ndisuio!NdisuioRead+0x54/nt!NtReadFile+0x566
efac7b7c f82fc2a8 2 ndisuio!NdisuioRead+0x54/nt!NtReadFile+0x566
ee4d1000 1 38 nt!MiFindContiguousMemory+0x63
efa4f000 0 2 Ntfs!NtfsInitializeVcb+0x30e/Ntfs!NtfsInitializeDevice+0x95
efa53000 0 2 Ntfs!NtfsInitializeVcb+0x30e/Ntfs!NtfsInitializeDevice+0x95
eea89000 0 1 TDI!DllInitialize+0x4f/nt!MiResolveImageReferences+0x4bc
ee798000 1 20 VIDEOPRT!pVideoPortGetDeviceBase+0x1f1
f0676000 1 10 hal!HalpGrowMapBuffers+0x134/hal!HalpAllocateAdapterEx+0x1ff
f0b75000 1 1 cpqasm2+0x2af67/cpqasm2+0x7847
f0afa000 1 1 cpqasm2+0x2af67/cpqasm2+0x6d82
```

#### 64-Bit Address Space Layouts

