# Chapter 5: Processes, Threads, and Jobs

## Process Internals

### Data Structures

- Each Windows process is represented by an **executive** process (`EPROCESS`) structure.
- Each process has one or more threads, each represented by an executive thread (`ETHREAD`) structure.
- The `EPROCESS` and most of its related data structures exist in **system** address space. One exception is the **PEB**, which exists in the **process** address space (because it contains information accessed by user-mode code).
- Some of the process data structures used in memory management, such as the **working set list**, are valid only within the context of the **current process**, because they are stored in **process-specific** system space.
- or each process that is executing a Win32 program, the Win32 subsystem process (`Csrss`) maintains a parallel structure called the `CSR_PROCESS`.
- Finally, the kernel-mode part of the Win32 subsystem (`Win32k.sys`) maintains a per-process data structure, `W32PROCESS`.
- Every `EPROCESS` structure is encapsulated as a process object by the **executive** object manager.
<p align="center"><img src="./assets/process-thread-data-structures.png" width="300px" height="auto"></p>

- The first member of the executive process structure is called **Pcb**.
- It is a structure of type `KPROCESS`, for kernel process. Although routines in the **executive** store information in the `EPROCESS`, the **dispatcher**, **scheduler**, and **interrupt/time** accounting code - being part of the OS kernel - use the `KPROCESS` instead.
- This allows a layer of abstraction to exist between the executive’s high-level functionality and its underlying low-level implementation of certain functions, and it helps prevent unwanted **dependencies** between the layers.

<p align="center"><img src="./assets/eprocess.png" width="400px" height="auto"></p>
