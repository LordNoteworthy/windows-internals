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
- `!pcr`   <p align="center"><img src="https://i.imgur.com/2MCLigj.png" width="300px" height="auto"></p>
- `!prcb` <p align="center"><img src="https://i.imgur.com/qXyfQvf.png" width="600px" height="auto"></p>
- `dt nt!_KPCR` <p align="center"><img src="https://i.imgur.com/sBs1zRT.png" width="500px" height="auto"></p>
- `dt nt!_KPRCB` <p align="center"><img src="https://i.imgur.com/8l0NNEi.png" width="400px" height="auto"></p>

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