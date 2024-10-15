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

- bEcause each session has its own instance of the Windows subsystem, the `CSR_PROCESS` structures are maintained by the *Csrss* process within each individual session.
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

### Protected Processes

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