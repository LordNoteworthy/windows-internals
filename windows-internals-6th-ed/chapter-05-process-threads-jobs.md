# Chapter 5: Processes, Threads, and Jobs

## Process Internals

### Data Structures

- Each Windows process is represented by an **executive** process (`EPROCESS`) structure.
- Each process has one or more threads, each represented by an executive thread (`ETHREAD`) structure.
- The `EPROCESS` and most of its related data structures exist in **system** address space. One exception is the **PEB**, which exists in the **process** address space (because it contains information accessed by user-mode code).
- Some of the process data structures used in memory management, such as the **working set list**, are valid only within the context of the **current process**, because they are stored in **process-specific** system space.
- For each process that is executing a Win32 program, the Win32 subsystem process (`Csrss`) maintains a parallel structure called the `CSR_PROCESS`.
- Finally, the kernel-mode part of the Win32 subsystem (`Win32k.sys`) maintains a per-process data structure, `W32PROCESS`.
- Every `EPROCESS` structure is encapsulated as a process object by the **executive** object manager.
<p align="center"><img src="./assets/process-thread-data-structures.png" width="300px" height="auto"></p>

- The first member of the executive process structure is called **Pcb**.
- It is a structure of type `KPROCESS`, for kernel process. Although routines in the **executive** store information in the `EPROCESS`, the **dispatcher**, **scheduler**, and **interrupt/time** accounting code - being part of the OS kernel - use the `KPROCESS` instead.
- This allows a layer of abstraction to exist between the executive’s high-level functionality and its underlying low-level implementation of certain functions, and it helps prevent unwanted **dependencies** between the layers.
<p align="center"><img src="./assets/eprocess.png" width="400px" height="auto"></p>

- The PEB lives in the user-mode address space of the process it describes. It contains information needed by the **image loader**, the **heap manager**, and other Windows components that need to access it from user mode.
- The `EPROCESS` and `KPROCESS` structures are accessible only from kernel mode. The important fields of the PEB are illustrated below:
<p align="center"><img src="./assets/peb-fields.png" width="400px" height="auto"></p>

- Because each session has its own instance of the Windows subsystem, the `CSR_PROCESS` structures are maintained by the *Csrss* process within each individual session.
<p align="center"><img src="./assets/csr_process.png" width="400px" height="auto"></p>

- You can dump the `CSR_PROCESS` structure with the `!dp` command in the `user`-mode debugger.
  - The `!dp` command takes as input the **PID** of the process whose `CSR_PROCESS` structure should be dumped
  - Alternatively, the structure pointer can be given directly as an argument Because `!dp` already performs a dt command internally, there is no need to use `dt` on your own: `0:000> !dp v 0x1c0aa8-8`
- The `W32PROCESS` structure contains all the information that the **Windows graphics** and **window management** code in
the kernel (Win32k) needs to maintain state information about GUI processes.
<p align="center"><img src="./assets/w32_process.png" width="300px" height="auto"></p>

- There is no command provided by the debugger extensions to dump the `W32PROCESS` structure, but it is present in the symbols of the Win32k driver.
  - As such, by using the `dt` command with the appropriate symbol name `win32k!_W32PROCESS`, it is possible to dump the
fields as long as the pointer is known.
  - Because the `!process` does not actually output this pointer (even though it is stored in the `EPROCESS` object), the field must be inspected manually with dt `nt!_EPROCESS Win32Process` followed by an `EPROCESS` pointer.

## Protected Processes

- In the Windows security model, any process running with a token containing the **debug** privilege (such as an **administrator**’s account) can request **any** access right that it desires to any other process running on the machine.
  - For example, it can RW **arbitrary** process memory, inject code, suspend and resume threads, and query information on other processes 🤸‍♀️.
- This logical behavior (which helps ensure that administrators will always have full control of the running code on the system) **clashes** with the system behavior for DRM requirements imposed by the media industry 🤷.
- To support reliable and protected playback of such content, Windows uses **protected processes**.
- The OS will allow a process to be protected only if the image file has been **digitally signed** with a special Windows Media Certificate ‼️
- Example of protected processes:
  - The Audio Device Graph process (`Audiodg.exe`) because protected music content can be decoded through it.
  - WER client process (`Werfault.exe`) can also run protected because it needs to have access to protected processes in case one of them crashes.
  - The System process itself is protected because some of the decryption information is generated by the` Ksecdd.sys` driver and stored in its UM memory. Additionally, it protects the integrity of all kernel **handles**.
- At the kernel level, support for protected processes is twofold:
  1. The bulk of process creation occurs in KM to avoid injection attacks.
  2. Protected processes have a special bit set in their `EPROCESS` structure that modifies the behavior of security-related routines in the process manager to deny certain access rights that would normally be granted to administrators. In fact, the only access rights that are granted for protected processes are `PROCESS_QUERY`/`SET_LIMITED_INFORMATION`, `PROCESS_TERMINATE`, and `PROCESS_SUSPEND_RESUME`. Certain access rights are also disabled for threads running inside protected processes.

## Flow of CreateProcess

- A Windows subsystem process is created when a call is make to one of the process-creation functions, such as `CreateProcess`, `CreateProcessAsUser`, `CreateProcessWithTokenW`, or `CreateProcessWithLogonW`.
- Here is an overview of the stages Windows follows to create a process:
  1. **Validate parameters**; convert Windows subsystem flags and options to their native counterparts; parse, validate, and convert the attribute list to its native counterpart.
  2. Open the image file (.exe) to be executed inside the process.
  3. Create the Windows **executive** process object.
  4. Create the **initial thread** (stack, context, and Windows **executive** thread object)
  5. Perform post-creation, **Windows-subsystem-specific** process initialization.
  6. Start execution of the initial thread (unless the `CREATE_SUSPENDED` flag was specified).
  7. In the context of the new process and thread, complete the initialization of the address space (such as load **required DLLs**) and begin execution of the program.
<p align="center"><img src="./assets/process-creation-stages.png" width="400px" height="auto"></p>

### Stage 1: Converting and Validating Parameters and Flags

- You can specify more than one **priority class** for a single `CreateProcess` call but Windows chooses the lowest-priority class set.
- If no priority class is specified for the new process, the priority class defaults to **Normal** unless the priority class of the process that created it is **Idle** or **Below Normal**, in which case the priority class of the new process will have the same priority as the creating class.
- If a **Real-time** priority class is specified for the new process and the process’ caller doesn’t have the *Increase Scheduling Priority* privilege, the **High** priority class is used instead.
- If no desktop is specified in `CreateProcess`, the process is associated with the caller’s **current desktop**.
- If the creation flags specify that the process will be **debugged**, `Kernel32` initiates a connection to the native debugging code in `Ntdll.dll` by calling `DbgUiConnectToDbg` and gets a handle to the debug object from the current TEB.
- The user-specified attribute list is **converted** from Windows subsystem format to **native** format and internal attributes are added to it.

<details><summary>Process Attributes:</summary>


| Native Attribute |  Equivalent Windows Attribute | Type | Description |
|------------------|-------------------------------|------|-------------|
| PS_CP_PARENT_PROCESS | PROC_THREAD_ATTRIBUTE_PARENT_PROCESS. Also used when elevating | Input | Handle to the parent process |
| PS_CP_DEBUG_OBJECT   | N/A – used when using DEBUG_PROCESS as a flag | Input | Debug object if process is being started debugged |
| PS_CP_PRIMARY_TOKEN  | N/A – used when using `CreateProcessAsUser/WithToken` | Input | Process token if `CreateProcessAsUser` was used |
| PS_CP_CLIENT_ID      | N/A – returned by Win32 API as a parameter | Output | Returns the TID and PID of the initial thread and the process |
| PS_CP_TEB_ADDRESS    | N/A – internally used and not exposed | Output |  Returns the address of the TEB for the initial thread |
| PS_CP_FILENAME       | N/A – used as a parameter in `CreateProcess` API | Input | Name of the process that should be created |
| PS_CP_IMAGE_INFO     | N/A – internally used and not exposed | Output | Returns `SECTION_IMAGE_INFORMATION`, which contains information on the version, flags, and subsystem of the executable, as well as the stack size and entry point |
| PS_CP_MEM_RESERVE    | N/A – internally used by SMSS and CSRSS | Input | Array of virtual memory reservations that should be made during initial process address space creation, allowing guaranteed availability because no other allocations have taken place yet |
| PS_CP_PRIORITY_CLASS | N/A – passed in as a parameter to the CreateProcess API | Input | Priority class that the process should be given |
| PS_CP_ERROR_MODE     | N/A – passed in through `CREATE_DEFAULT_ERROR_MODE` flag | Input | Hard error-processing mode for the process |
| PS_CP_STD_HANDLE_INFO| | Input | Specifies if standard handles should be duplicated, or if new handles should be created |
| PS_CP_HANDLE_LIST    | `PROC_THREAD_ATTRIBUTE_HANDLE_LIST` | Input | List of handles belonging to the parent process that should be inherited by the new process|
| PS_CP_GROUP_AFFINITY | `PROC_THREAD_ATTRIBUTE_GROUP_AFFINITY` | Input | Processor group(s) the thread should be allowed to run on |
| PS_CP_PREFERRED_NODE | `PROC_THREAD_ATTRIBUTES_PRFERRED_NODE` | Input | Preferred (ideal) node that should be associated with the process. It affects the node on which the initial process heap and thread stack will be created |
| PS_CP_IDEAL_PROCESSOR| `PROC_THREAD_ATTTRIBUTE_IDEAL_PROCESSOR` | Input | Preferred (ideal) processor that the thread should be scheduled on |
| PS_CP_UMS_THREAD     | `PROC_THREAD_ATTRIBUTE_UMS_THREAD` | Input | Contains the UMS attributes, completion list, and context |
| PS_CP_EXECUTE_OPTIONS| `PROC_THREAD_MITIGATION_POLICY` | Input | Contains information on which mitigations (SEHOP, ATL Emulation, NX) should be enabled/disabled for the process |

</details>

### Stage 2: Opening the Image to Be Executed

- The first stage in `NtCreateUserProcess` is to find the appropriate Windows image that will run the executable file specified by the caller and to create a section object to later map it into the address space of the new process.
<p align="center"><img src="./assets/windows-image-to-activate.png" width="300px" height="auto"></p>

- Now that `NtCreateUserProcess` has found a valid Windows executable image, it looks in the registry under `HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options` to see whether a subkey with the file name and extension of the executable image exists there.
  - If it does, `PspAllocateProcess` looks for a value named *Debugger* for that key. If this value is present, the image to be run becomes the string in that value and `CreateProcess` restarts at Stage 1.

<details><summary>Decision Tree for Stage 1 of CreateProcess:</summary>

| If the Image ... | Create State Code | This Image Will Run |  ... and This Will Happen |
| ---------------- | ----------------- | ------------------- | ------------------------- |
| Is a POSIX executable file | PsCreateSuccess | `Posix.exe` | CreateProcess restarts Stage 1 |
| Is an MS-DOS application with an exe, com, or pif extension | PsCreateFailOnSectionCreate | `Ntvdm.exe` | CreateProcess restarts Stage 1 |
| Is a Win16 application | PsCreateFailOnSectionCreate | `Ntvdm.exe` | CreateProcess restarts Stage 1|
| Is a Win64 application on a 32-bit system (or a PPC, MIPS, or Alpha Binary) | PsCreateFailMachineMismatch  | N/A | CreateProcess will fail |
| Has a Debugger key with another image name | PsCreateFailExeName | Name specified in the Debugger key | CreateProcess restarts Stage 1 |
| Is an invalid or damaged Windows EXE | PsCreateFailExeFormat | N/A | CreateProcess will fail Cannot be opened PsCreateFailOnFileOpen N/A CreateProcess will fail |
| Is a command procedure (application with a bat or cmd extension) | PsCreateFailOnSectionCreate | `Cmd.exe` | CreateProcess restarts Stage 1 |

</details>

### Stage 3: Creating the Windows Executive Process Object (PspAllocateProcess)

- At this point, `NtCreateUserProcess` has opened a valid Windows executable file and created a **section object** to map it into the new process address space. Next it creates a Windows executive process object to run the image by calling the internal system function `PspAllocateProcess`. Creating the executive process object involves the following substages:
  - Setting up the `EPROCESS` object
  - Creating the initial process address space
  - Initializing the kernel process structure(`KPROCESS`)
  - Setting up the PEB
  - Concluding the setup of the process address space (which includes initializing the working set list and VA space descriptors and mapping the image into address space).

#### Stage 3A: Setting Up the EPROCESS Object

- **Affinity & Node Setup**: The process inherits the CPU affinity and NUMA node from the parent, unless explicitly set during creation.
- **Priority Inheritance**: The I/O and page priority are inherited from the parent, or default values are used if no parent exists.
- **Exit Status**: The new process is marked as pending.
- **Error Handling**: Error handling modes are set, either inherited from the parent or defaulting to showing all errors.
- **Parent Process Info**: The parent process ID is stored in the new process.
- **Execution Options**: The process checks execution options for settings like **large page** mapping or **NUMA** assignments.
- **ASLR & Privileges**: Stack randomization is disabled if necessary, and the system acquires privileges required for process creation.
- **Token & Security**: The process inherits the parent’s security token or uses a specified token, ensuring privilege checks are passed.
- **Session Handling**: If the process spans across sessions, the parent temporarily attaches to handle resources correctly.
- **Quota & Working Set**: The process inherits or gets new resource quotas, including working set sizes (memory limits).
- **Address Space & Affinity**: The process address space is initialized, and **group affinity** is set based on inheritance or system settings.
- **KPROCESS Initialization**: The `KPROCESS` part of the process object is initialized.
- **Priority & Handle Table**: The process inherits or sets its own priority class, and the handle table is initialized with inheritable handles.
- **Final Settings**: Performance options, process priority, and other settings are finalized, including the **PEB** and mitigation options like **No-Execute** (NX) support.
- **Process Creation Time**: The process ID and creation time are set, but the process is not yet inserted into the system's process table.

#### Stage 3B: Creating the Initial Process Address Space

- The initial address space includes:
  - **Page Directory**: Creates the main page directory, which may involve multiple directories for systems with more complex page tables (e.g., PAE mode or 64-bit systems).
  - **Hyperspace Page**: Allocates a special memory page used for managing the process's memory.
  - **VAD Bitmap Page**: Allocates a page for tracking the Virtual Address Descriptors (VAD) in the process.
  - **Working Set List**: Allocates memory for the process’s working set, which tracks the active memory pages.

#### Stage 3C: Creating the Kernel Process Structure

- Involves initializing the `KPROCESS` (Pcb) part of the process object. This is done through `KeInitializeProcess`, which sets up various kernel-level attributes of the process. Key steps include:
    - **Thread List**: Initializes an empty doubly-linked list to track all threads within the process.
    - **Default Quantum**: Sets an initial thread scheduling quantum value (default is 6), which will later be adjusted based on system settings.
    - **Base Priority**: Sets the process's base priority as calculated in *Stage 3A*.
    - **Processor & Group Affinity**: Configures the default processor and group affinity for the process's threads, either inherited or set in earlier stages.
    - **Swapping State**: Marks the process as **resident** in memory.
    - **Thread Seed**: Assigns an ideal processor for the process based on the system's **round-robin** algorithm, ensuring load balancing across processors.

#### Stage 3D: Concluding the Setup of the Process Address Space

- Setting up a new process's address space in Windows involves several steps coordinated by the memory manager:
  - The virtual memory manager sets the process's **last trim time**, which the working set manager uses to determine when to trim the process's working set.
  - The **working set list** for the process is initialized, allowing page faults to be handled.
  - The **section** created when opening the image file is mapped into the process’s address space, setting the **base address**.
  - `Ntdll.dll` is mapped, and for *Wow64* processes, the 32-bit `Ntdll.dll` is mapped as well.
  - A new **session** is created if required, typically for session-based processes managed by SMSS.
  - Standard **handles** are **duplicated** and stored in the process's parameter structure.
  - **Memory reservations** are processed, with options to reserve the first 1 or 16 MB for low-address requirements.
  - **User parameters** are added, with relative memory adjustments.
  - **Affinity** details are written into the PEB.
  - The **MinWin** API redirection set is mapped into the process/

#### Stage 3E: Setting Up the PEB

- `NtCreateUserProcess` calls `MmCreatePeb`, which first maps the systemwide NLS tables into the process’ address space.
- It next calls `MiCreatePebOrTeb` to allocate a page for the PEB and then initializes a number of fields, most of them based on internal variables that were configured through the registry, such as `MmHeap*` values, `MmCriticalSectionTimeout`, and `MmMinimumStackCommitInBytes`.
- Some of these fields can be **overridden** by settings in the linked executable image, such as the Windows version in the PE header or the affinity mask in the load configuration directory of the PE header.

#### Stage 3F: Completing the Setup of the Executive Process Object (PspInsertProcess)

- In the final stage of process creation, `PspInsertProcess` completes setup for the executive process object with these steps:
  - If system-wide process **auditing** is enabled, it logs the process creation in the Security event log.
  - If the parent process is part of a **job**, the job is inherited by the new process and added to the job’s session.
  - The new process is added to the list of **active processes**.
  - The parent’s **debug port** is inherited unless specified otherwise, or a new debug port is attached if one is provided.
  - The function checks that the process’s **group affinity** aligns with any restrictions set by its job object, preventing conflicts between job and process permissions.
  - Finally, `ObOpenObjectByPointer` creates a handle for the process and returns it, with process creation callbacks triggered only once the first thread is created.

### Stage 4: Creating the Initial Thread and Its Stack and Context

- At this point, the Windows executive process object is completely set up. It still has no thread, however, so it can’t do anything yet.
- Normally, the `PspCreateThread` routine is responsible for all aspects of thread creation and is called by `NtCreateThread` when a new thread is being created.
- However, because the **initial thread** is created **internally** by the kernel without user-mode input, the two helper routines that `PspCreateThread` relies on are used instead:
`PspAllocateThread` and `PspInsertThread`.
- `PspAllocateThread` handles the actual creation and initialization of the executive thread object itself, while `PspInsertThread` handles the creation of the thread handle and security attributes and the call to `KeStartThread` to turn the executive object into a schedulable thread on the system.
- However, the thread won’t do anything yet—it is created in a **suspended** state and isn’t resumed until the process is completely initialized.
- `PspAllocateThread` creates a thread in a Windows process and includes these steps:
  - Prevents **UMS** threads in Wow64 processes and disallows user-mode thread creation in system processes.
  - Creates and initializes an executive thread object.
  - Sets up CPU **rate limiting** if enabled.
  - Initializes lists for **LPC**, **I/O** management, and **Executive**.
  - Sets the thread **creation time** and **ID**.
  - Prepares the thread's **stack and context**, initializing the Wow64 context if necessary.
  - Allocates the **TEB**.
  - Sets the user-mode start address (`RtlUserThreadStart`) in the **ETHREAD**.
  - Calls `KeInitThread` to initialize `KTHREAD` with process priorities, affinity, and **machine-dependent hardware context** (context, trap, exception frames).
  - Initializes the UMS state via `PspUmsInitThread` if the thread is a UMS thread.
- After `NtCreateUserProcess` completes preliminary setup, it calls `PspInsertThread` to finalize thread creation with these steps:
  - Checks the thread’s **group affinity** to ensure it aligns with **job limits**.
  - Verifies the process and thread are still active; fails if either has been terminated.
  - Initializes `KTHREAD` (via `KeStartThread`) by inheriting scheduler settings from the owner process, setting the ideal node and processor, updating group affinity, and inserting it in process lists maintained by `KPROCESS`. On x64. another system-wide list of processes, `KiProcessListHead`, is used by PatchGuard to maintain the integrity of the executive’s `PsActiveProcessHead`.
  - Increments the process’s thread count, updates the high watermark if this is a new maximum, and freezes the primary token if this is the second thread.
  - Increments the UMS thread count if applicable.
  - Inserts the thread into the process’s list, suspending it if requested.
  - Sets up **CPU rate** control if enabled.
  - Calls registered thread and process callbacks.
  - The handle is created with `ObOpenObjectByPointer`
  - The thread is readied for execution by calling `KeReadyThread` It enters the deferred ready queue, the process is paged out, and a page in is requested.

### Stage 5: Performing Windows Subsystem–Specific PostInitialization

`Kernel32.dll` then performs various operations related to Windows subsystem–specific operations to finish initializing the process.
- Before an executable runs in Windows, multiple checks are performed:
  - **Validation Checks**: Verifies the executable's image version and applies group policy restrictions (e.g., **application certification**). On certain Windows Server editions, additional checks prevent use of **disallowed APIs**.
  - **Restricted Token Creation**: If software restriction policies apply, a restricted token is created.
  - **Compatibility Check**: The application-compatibility database is queried for entries related to the process. Compatibility settings are stored but applied later.
- Next, `Kernel32.dll` sends a message to the Windows subsystem to initialize structures for **SxS**, including **manifest** information and** DLL redirection** paths. The subsystem receives process and thread handles, creation flags, UI language, and other initialization data, performing these steps:
  - **Process and Thread Initialization**: `CsrCreateProcess` duplicates handles, sets process priority, and allocates structures (`CSR_PROCESS` and `CSR_THREAD`) to track the new process and thread.
  - **Session Management**: Updates session process count and assigns a default shutdown level.
  - **Exception Handling**: Links the exception port to notify the subsystem of second-chance exceptions.
  - **GUI Initialization**: Shows the application start cursor, allowing five seconds for a GUI response before reverting to a standard cursor.
- For **elevated processes**, further checks include whether the process was launched via `ShellExecute` with *AppInfo* service elevation. If so, the token’s virtualization flag is enabled, or an elevation request is sent for re-evaluation.
- **Protected processes** skip most **compatibility** and **elevation** checks since they are designed with built-in security to avoid vulnerabilities associated with **shimming** and **virtualization**.

### Stage 6: Starting Execution of the Initial Thread

- At this point, the process environment has been determined, resources for its threads to use have been allocated, the process has a thread, and the Windows subsystem knows about the new process.
- Unless the caller specified the CREATE_ `SUSPENDED` flag, the **initial** thread is now **resumed** so that it can start running and perform the remainder of the process initialization work that occurs in the context of the new process (Stage 7).

### Stage 7: Performing Process Initialization in the Context of the New Process

The new thread begins by running the kernel-mode routine KiThreadStartup, which lowers the IRQL level (DPC ➡️ APC) and calls `PspUserThreadStartup`, passing the user-defined start address. `PspUserThreadStartup` then:
  - Disables **swapping** of the primary process **token** at runtime, intended only for *POSIX* support.
  - Sets Locale ID and the **ideal processor** in the TEB using kernel-mode data structures.
  - Checks if thread creation **failed**; if so, it proceeds with **cleanup**.
  - Calls `DbgkCreateThread` to send debugger and image notifications. If image notifications weren't previously sent, notifications are sent for the process and for `Ntdll.dll`. These notifications occur now because the process ID wasn’t available during earlier image mapping.
- If the process is being debugged, `PspUserThreadStartup` ensures debug events, such as `CREATE_PROCESS_DEBUG_INFO` and thread startup events, are sent to the debugger using `DbgkCreateThread`. It then waits for the debugger’s response via `ContinueDebugEvent`.
- If the thread was marked for termination on startup, it terminates after the debugger and image notifications, ensuring they are not missed, even if the thread **does not fully execute**.
- `PspUserThreadStartup` checks for application **prefetching**. If enabled, it invokes prefetcher and Superfetch to load pages referenced during the first 10 seconds of the process's last run. It then verifies if the systemwide cookie in `SharedUserData` is set. If not, it creates it from a hash of system data, used to encode pointers for security, especially in the heap manager.
- Next, `PspUserThreadStartup` prepares the thread context to run `LdrInitializeThunk` in `Ntdll.dll` and `RtlUserThreadStart`. The `LdrInitializeThunk` routine handles **loader** setup, initializing the **heap manager**, **NLS** tables, **TLS** and **FLS** arrays, and **critical sections**. It loads necessary DLLs and triggers DLL entry points with `DLL_PROCESS_ATTACH`.
- Finally, `NtContinue` restores the context to user mode, starting actual thread execution. `RtlUserThreadStart` calls the app EP, having already set up the process environment. This approach enables **exception handling** from `Ntdll.dll` and `Kernel32.dll` for any unhandled exceptions, and it coordinates thread exit and cleanup. This also allows developers to register their own unhandled exception handlers via `SetUnhandledExceptionFilter`.