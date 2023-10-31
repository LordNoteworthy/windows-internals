# Chapter 3 System Mechanisms

## Trap Dispatching

- __Interrupts__ and __exceptions__ are OS conditions that divert the processor to code outside the normal flow of control.
- The term __trap__ refers to a processor’s mechanism for capturing an executing thread when an exception or an interrupt occurs and transferring control to a fixed location in the OS.
- The processor transfers control to a __trap handler__, which is a function specific to a particular interrupt or exception.
- The kernel distinguishes between interrupts and exceptions in the following way:
    - An interrupt is an __asynchronous__ event (one that can occur at any time) that is unrelated to what the processor is executing.
        - Generated primarily by __I/O devices, processor clocks, or timers__, and they can be enabled or disabled.
    - An exception, in contrast, is a __synchronous__ condition that usually results from the execution of a particular instruction.
        - Running a program a second time with the same data under the same conditions can reproduce exceptions.
        - Examples of exceptions include __memory-access violations__, __certain debugger instructions__, and __divideby-zero__ errors.
        - The kernel also regards system service calls as exceptions (although technically they’re system traps).
    <p align="center"><img src="./assets/trap-handlers.png" width="400px" height="auto"></p>
- Either hardware or software can generate exceptions and interrupts:
    - A __bus error__ exception is caused by a hardware problem, whereas a __divide-by-zero__ exception is the result of a software bug.
    - An __I/O device__ can generate an interrupt, or the kernel itself can issue a software interrupt such as an __APC or DPC__.
- When a hardware exception or interrupt is generated, the processor records enough machine state on the __kernel stack of the thread__ that’s interrupted to return to that point in the control flow and continue execution as if nothing had happened. If the thread was executing in user mode, Windows __switches to the thread’s kernel-mode stack__.
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

### Interrupt Dispatching

- Hardware-generated interrupts typically originate from I/O devices that must notify the processor when they need service.
    - Interrupt-driven devices allow the OS to get the __maximum use out of the processor__ by overlapping central processing with I/O operations.
    - A thread starts an I/O transfer to or from a device and then can execute other useful work while the device completes the transfer.
    - When the device is finished, it interrupts the processor for service.
    - Pointing devices, printers, keyboards, disk drives, and network cards are generally __interrupt driven__.
- The kernel installs interrupt trap handlers to respond to device interrupts.
    - Those handlers transfer control either to an external routine (__Interrupt Service Routine ISR__) that handles the interrupt or ;
    - To an internal kernel routine that responds to the interrupt.
    - Device drivers supply ISRs to service device interrupts, and the kernel provides interrupt-handling routines for other types of interrupts.

#### Hardware Interrupt Processing

- On the hardware platforms supported by Windows, external I/O interrupts come into one of the lines on an __interrupt controller__.
- The controller, in turn, interrupts the processor on a single line.
- Once the processor is interrupted, it queries the controller to get the __interrupt request (IRQ)__.
- The interrupt controller translates the IRQ to an interrupt number, uses this number as an index into a structure called the __interrupt dispatch table (IDT)__, and transfers control to the appropriate interrupt dispatch routine.
- At system boot time, Windows fills in the IDT with pointers to the kernel routines that handle each interrupt and exception.
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
- Each processor has a separate IDT so that different processors can run different ISRs, if appropriate.

#### x86 Interrupt Controllers

- Most x86 systems rely on either the __i8259A Programmable Interrupt Controller (PIC)__ or a variant of the __i82489 Advanced Programmable Interrupt Controller (APIC)__.
- Today’s computers include an APIC.
- PIC:
    - Works only on __uni-processors__ systems.
    - Has only __eight__ interrupt lines
    - IBM PC arch extended it to __15__ interrupt lines (7 on master + 8 on slave).
- APIC:
    - Work with __multiprocessor__ systems.
    - Have __256__ interrupt lines. I
    - For compatibility, APIC supports a PIC mode.
    - Consists of several components:
        - __I/O APIC__ that receives interrupts from devices
        - __Local APICs__ that receive interrupts from the I/O APIC on the bus and that interrupt the CPU they are associated with.
        - An i8259A-compatible interrupt controller that translates APIC input into PIC-equivalent signals.
        <p align="center"><img src="./assets/apic.png" width="300px" height="auto"></p>
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
Linti1.: 00000000`000004ff  Vec:FF  NMI  Dest=Self      edg high
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

- Although interrupt controllers perform interrupt prioritization, Windows __imposes__ its own interrupt __priority__ scheme known as __interrupt request levels (IRQLs)__.
- The kernel represents IRQLs internally as a number from 0 through 31 on x86 and from 0 to 15 on x64 and IA64, with __higher numbers__ representing __higher-priority__ interrupts. <p align="center"><img src="./assets/irqls.png" width="600px" height="auto"></p>
- Interrupts are serviced in priority order, and a higher-priority interrupt preempts the servicing of a lower-priority interrupt.
- When a high-priority interrupt occurs, the processor saves the __interrupted thread’s state__ and invokes the trap dispatchers associated with the interrupt.
- The trap dispatcher raises the IRQL and calls the interrupt’s service routine.
- After the service routine executes, the interrupt dispatcher lowers the processor’s IRQL to where it was before the interrupt occurred and then loads the saved machine state. The interrupted thread resumes executing where it left off.
- When the kernel lowers the IRQL, lower-priority interrupts that were masked might materialize. If this happens, the kernel repeats the process to handle the new interrupts.
- IRQLs are also used to __synchronize__ access to kernel-mode data structures. As a kernel-mode thread runs, it raises or lowers the processor’s IRQL either directly by calling `KeRaiseIrql` and `KeLowerIrql` or, more commonly, indirectly via calls to functions that acquire kernel synchronization objects.
- For example, when an interrupt occurs, the trap handler (or perhaps the processor) raises the processor’s IRQL to the assigned IRQL of the interrupt source. This elevation __masks__ all interrupts __at and below that IRQL__ (on that processor only), which ensures that the processor servicing the interrupt isn’t waylaid by an interrupt at the __same level or a lower level__. The masked interrupts are either handled by another processor or held back until the IRQL drops. Therefore, all components of the system, including the kernel and device drivers, attempt to __keep the IRQL at passive level__ (sometimes called low level). They do this because device drivers can respond to hardware interrupts in a timelier manner if the IRQL isn’t kept __unnecessarily elevated for long periods__.
-  > :bangbang: An exception to the rule that raising the IRQL blocks interrupts of that level and lower relates to APC-level interrupts If a thread raises the IRQL to APC level and then is rescheduled because of a dispatch/DPC-level interrupt, the system might deliver an APC-level interrupt to the newly scheduled thread Thus, APC level can be considered a thread-local rather than processor-wide IRQL.
- Processor’s IRQL is always at __passive level__ when it’s executing usermode code. Only when the processor is executing kernel-mode code can the IRQL be higher.

#### Mapping Interrupts to IRQLs

- IRQL levels aren’t the same as the interrupt requests (IRQs) defined by interrupt controllers.
- In HAL in Windows, a type of device driver called a __bus driver__ determines the presence of devices on its bus (PCI, USB, and so on) and what interrupts can be assigned to a device.
- The bus driver reports this information to the __Plug and Play__ manager, which decides, after taking into account the acceptable interrupt assignments for all other devices, which interrupt will be assigned to each device.
- Then it calls a Plug and Play interrupt arbiter, which __maps interrupts to IRQLs__.

#### Predefined IRQLs

- The kernel uses __high level__ only when it’s halting the system in `KeBugCheckEx` and masking out all interrupts.
- __Power fail level__ originated in the original Windows NT design documents, which specified the behavior of system power failure code, but this IRQL has never been used.
- __Interprocessor interrupt level__ is used to request another processor to perform an action, such as updating the processor’s TLB cache, system shutdown, or system crash.
- __Clock level__ is used for the system’s clock, which the kernel uses to track the time of day as well as to measure and allot CPU time to threads.
- The system’s real-time clock (or another source, such as the local APIC timer) uses __profile level__ when kernel profiling (a performance-measurement mechanism) is enabled.
- The __synchronization IRQL__ is internally used by the dispatcher and scheduler code to protect access to global thread scheduling and wait/synchronization code.
- The __device IRQLs__ are used to prioritize device interrupts.
- The __corrected machine check interrupt level__ is used to signal the OS after a serious but corrected hardware condition or error that was reported by the CPU or firmware through the Machine Check Error (MCE) interface.
- __DPC/dispatch-level and APC-level interrupts__ are software interrupts that the kernel and device drivers generate.
- The lowest IRQL, __passive level__, isn’t really an interrupt level at all; it’s the setting at which normal thread execution takes place and all interrupts are allowed to occur.
> ‼️ One important restriction on code running at DPC/dispatch level or above is that it can’t wait for an object if doing so necessitates the scheduler to select another thread to execute, which is an illegal operation because the scheduler relies on DPC-level software interrupts to schedule threads.

> ‼️ Another restriction is that only nonpaged memory can be accessed at IRQL DPC/dispatch level or higher. This rule is actually a side effect of the first restriction because attempting to access memory that isn’t resident results in a page fault. When a page fault occurs, the memory manager initiates a disk I/O and then needs to wait for the file system driver to read the page in from disk. This wait would, in turn, require the scheduler to perform a context switch (perhaps to the idle thread if no user thread is waiting to run), thus violating the rule that the scheduler can’t be invoked (because the IRQL is still DPC/dispatch level or higher at the time of the disk read). A further problem results in the fact that I/O completion typically occurs at APC_LEVEL, so even in cases where a wait wouldn’t be required, the I/O would never complete because the completion APC would not get a chance to run.

#### Interrupt Objects

- The kernel provides a **portable** mechanism —a kernel control object called an **interrupt object**— that allows device drivers to **register ISRs** for their devices.
- An interrupt object contains all the information the kernel needs to associate a device ISR with a particular level of interrupt, including the **address of the ISR**, the **IRQL** at which the **device interrupts**, and the **entry** in the kernel’s interrupt dispatch table (**IDT**) with which the ISR should be associated.
- When an interrupt object is initialized, a few instructions of assembly language code, called the **dispatch code**, are copied from an interrupt-handling template, `KiInterruptTemplate`, and stored in the object . When an interrupt occurs, this code is executed.
<p align="center"><img src="./assets/interupt-control-flow.png" width="300px" height="auto"></p>

To view the contents of the interrupt object associated with the interrupt, execute dt `nt!_kinterrupt` with the address following `KINTERRUPT` (get it via `!idt`):

```c
lkd> dt nt!_KINTERRUPT fffffa80045bad80
	+0x000 Type             : 22
	+0x002 Size             : 160
    +0x008 InterruptListEntry : _LIST_ENTRY [ 0x0000000000000000 - 0x0 ]
    +0x018 ServiceRoutine   : 0xfffff8800356ca04     unsigned char i8042prt!I8042KeyboardInterruptService+0 // <-- The ISR’s address
    +0x020 MessageServiceRoutine : (null)
	+0x028 MessageIndex
	+0x030 ServiceContext
	+0x038 SpinLock
	+0x040 TickCount
	+0x048 ActualLock
	+0x050 DispatchAddress  : 0xfffff80001a7db90 //
	+0x058 Vector           : 0x81
	+0x05c Irql             : 0x8 ''
	...
	+0x090 DispatchCode		: [4] 0x8d485550 // <-- interrupt code that executes when an interrupt occurs,
											 // it build the trap frame on the stack and then call the function
											 // stored in the DispatchAddress passing it a pointer to the interrupt object .
```

- Although there is **no direct mapping** between an **interrupt vector** and an **IRQ**, Windows does keep track of this translation when managing device resources through what are called *arbiters*.
- For each resource type, an arbiter maintains the relationship between **virtual resource** usage (such as an interrupt vector) and **physical resources** (such as an interrupt line).
- You can query the ACPI IRQ arbiter with the `!apciirqarb` command to obtain information on the ACPI IRQ arbiter.

```c
lkd> !acpiirqarb
	Processor 0 (0, 0):
	Device Object: 0000000000000000
	Current IDT Allocation:
	...
	0000000000000081 - 0000000000000081 D fffffa80029b4c20 (i8042prt) A:0000000000000000 IRQ:0
...
```

- You will be given the owner of the vector, in the type of a device object. You can then use the `!devobj` command to get information on the `i8042prt` device in this example (which corresponds to the PS/2 driver):

```c
lkd> !devobj fffffa80029b4c20

...
Entry 4 - Interrupt (0x2) Device Exclusive (0x1)
      Flags (0x01) - LATCHED
      Level 0x1, Vector 0x1, Group 0, Affinity 0xffffffff
```

> ‼️ Windows and Real-Time Processing
> Because Windows doesn’t enable controlled prioritization of device IRQs and user-level applications execute only when a processor’s IRQL is at passive level, Windows **isn’t** typically suitable as a **real-time OS**.
- Associating or disconnecting an ISR with a particular level of interrupt is called connecting or disconnecting an interrupt object.
	- Accomplished by calling `IoConnectInterruptEx` and `IoDisconnectInterruptEx`, which allow a device driver to “turn on” an ISR when the driver is loaded into the system and to “turn off” the ISR if the driver is unloaded.
	- :+1: Prevents device drivers from fiddling directly with interrupt hardware.
	- :+1: Aids in creating portable device drivers.

- Interrupt objects allow the kernel to easily call more than one ISR for any interrupt level.
	- Multiple device drivers create interrupt objects and connect them to the same IDT entry.
	- Allows the kernel to support *daisy-chain* configurations.

<details><summary>Line-Based vs. Message Signaled-Based Interrupts:</summary>

- Shared interrupts are often the cause of high interrupt latency and can also cause stability issues.
- Consuming four IRQ lines for a single device quickly leads to **IRQ line exhaustion**.
- PCI devices are each connected to only one IRQ line anyway, so we cannot use more than one IRQ in the first place.
- Provide poor scalability in multiprocessor environments.
- Message-signaled interrupts (MSI) model solves these issue: a device delivers a message to its driver by **writing to a specific memory address**.
- This action causes an interrupt, and Windows then calls the ISR with the message content (value) and the address where the message was delivered.
- A device can also deliver multiple messages (up to 32) to the memory address, delivering different payloads based on the event.
- The need for IRQ lines is removed ▶️ decreasing latency.
- Nullifies any benefit of sharing interrupts, decreasing latency further by directly delivering the interrupt data to the concerned ISR.
- MSI-X, an extension to the MSI model, introduced in PCI v3.0
- Add the ability to use a different address (which can be dynamically determined) for each of the MSI payloads ▶️ enabling NUMA.
- Improves latency and scalability.
</details>

<details><summary>Interrupt Affinity and Priority:</summary>

- On systems that both support ACPI and contain an APIC, Windows enables driver developers and administrators to somewhat control the processor affinity (selecting the processor or group of processors that receives the interrupt) and affinity policy (selecting how processors will be chosen and which processors in a group will be chosen).
- Configurable through a registry value called `InterruptPolicyValue` in the Interrupt Management\Affinity Policy key under the device’s instance key in the registry.
</details>

#### Software Interrupts

- Although hardware generates most interrupts, the Windows kernel also generates **software interrupts** for a variety of tasks:
	- Initiating thread dispatching
	- Non-time-critical interrupt processing
	- Handling timer expiration
	- Asynchronously executing a procedure in the context of a particular thread
	- Supporting asynchronous I/O operations

##### Dispatch or Deferred Procedure Call (DPC) Interrupts

- The kernel always raises the processor’s IRQL to DPC/dispatch level or above when it needs to **synchronize access to shared** kernel structures.
- When the kernel detects that **dispatching should occur**, it requests a DPC/dispatch-level interrupt.
-  The kernel uses DPCs to process **timer expiration** (and release threads waiting for the timers) and to **reschedule the processor** after a thread’s quantum expires.
- Device drivers use DPCs to process interrupts:
	- Perform the minimal work necessary to acknowledge their device, save volatile interrupt state, and defer data transfer or other less time-critical interrupt processing activity for execution in a DPC at DPC/dispatch IRQL.
- By default, the kernel places DPC objects at the end of the DPC queue of the **processor on which the DPC was requested** (typically the processor on which the **ISR executed**).
<p align="center"><img src="./assets/delivering-a-dpc.png" width="500px" height="auto"></p>

- DPC routines execute without regard to what thread is running, meaning that when a DPC routine runs, it **can’t** assume what process address space is **currently mapped**.
- DPC routines can call kernel functions, but they **can’t call system services**, generate page faults, or create or wait for dispatcher objects. They can, however, access nonpaged system memory addresses, because system address space is always mapped regardless of what the current process is.
DPCs are provided **primarily for device drivers**, but the kernel uses them too . The kernel most frequently uses a DPC to **handle quantum expiration**.
- At every tick of the system clock, an interrupt occurs at clock IRQL. The *clock interrupt handler* updates the system time
and then decrements a counter that tracks how long the current thread has run. When the counter **reaches 0**, the thread’s time quantum has expired and the kernel might need to reschedule the processor, a lower-priority task that should be done at DPC/dispatch IRQL.
- ‼️ Because DPCs execute **regardless of whichever thread is currently running** on the system, they are a primary cause for perceived system **unresponsiveness** of client systems or workstation workloads because even the highest-priority thread will be interrupted by a pending DPC. Some DPCs run long enough that users might perceive **video or sound lagging**, and even **abnormal mouse or keyboard latencies**, so for the benefit of drivers with long-running DPCs, Windows supports **threaded DPCs**.
- Threaded DPCs function by executing the DPC routine at **passive level** on a **real-time priority (priority 31) thread**.

##### Asynchronous Procedure Call (APC) Interrupts

- APCs provide a way for user programs and system code **to execute in the context of a particular user thread** (and hence a particular process address space).
- Because APCs are queued to execute in the context of a particular thread and run at an IRQL less than DPC/dispatch level, they don’t operate under the same restrictions as a DPC.
	- An APC routine can **acquire** resources (objects), **wait** for object handles, incur **page faults**, and call **system services**.
- Unlike the DPC queue, which is **systemwide**, the APC queue is **thread-specific** — each thread has its own APC queue.
- There are two kinds of APCs: **kernel mode** and **user mode**:
- Kernel mode APCs:
	- They don’t require **permission** from a target thread to run in that thread’s context, while user-mode APCs do.
	- Two types of kernel-mode APCs: **normal** and **special** .
		- Special APCs execute at **APC level** and allow the APC routine to **modify** some of the APC **parameters**.
		- Normal APCs execute at **passive level** and receive the modified parameters from the special APC routine (or the original parameters if they weren’t modified).
- Both normal and special APCs can be **disabled** by **raising the IRQL** to **APC level** or by calling `KeEnterGuardedRegion`.
- `KeEnterGuardedRegion` disables APC delivery by setting the `SpecialApcDisable` field in the calling thread’s `KTHREAD` structure.
- A thread can **disable normal APCs only** by calling `KeEnterCriticalRegion`, which sets the `KernelApcDisable` field in the thread’s `KTHREAD` structure.
- Kernel-mode APCs uses cases by the executive:
	- Direct a thread to stop executing an interruptible system service.
	- Record the results of an asynchronous I/O operation in a thread’s address space.
- Environment subsystems use special kernel-mode APCs to make a thread suspend or terminate itself or to get or set its user-mode execution context.
- Several Windows APIs—such as `ReadFileEx`, `WriteFileEx`, and `QueueUserAPC` use **user-mode APCs**.
	- For example, the `ReadFileEx` and `WriteFileEx` allow the caller to specify a **completion routine** to be called when the I/O operation finishes.
	- The I/O completion is implemented by queuing an APC to the thread that issued the I/O.
	- However, the callback to the completion routine doesn’t necessarily take place when the APC is queued because user-mode APCs are delivered to a thread only when it’s in an **alertable wait state**.
	- A thread can enter a wait state either by **waiting for an object handle** and specifying that its **wait is alertable** (with the Windows `WaitForMultipleObjectsEx()`) or by testing directly whether it has a pending APC (using `SleepEx()`).
	- In both cases, if a user-mode APC is pending, the kernel interrupts (alerts) the thread, transfers control to the APC routine, and resumes the thread’s execution when the APC routine completes.

### Timer Processing

- The system’s clock interval timer is probably the **most important device** on a Windows machine, as evidenced by its high IRQL value (`CLOCK_LEVEL`) and due to the critical nature of the work it is responsible for. Without this interrupt:
	- Windows would lose track of time, causing erroneous results in calculations of **uptime** and **clock time**;
	- And worst, causing timers **not to expire** anymore and **threads** never to **lose** their **quantum** anymore.
	- :arrow_forward: Windows would also not be a preemptive OS, and unless the current running thread yielded the CPU, critical background tasks and scheduling could never occur on a given processor.
- Windows programs the system clock to fire at the **most appropriate interval** for the machine, and subsequently allows drivers, applications, and administrators to **modify the clock interval** for their needs.
- The system clock is maintained either by the *PIT (Programmable Interrupt Timer)* chip that is present on all computers since the PC/AT, or the *RTC (Real Time Clock)*.
- On today’s machines, the APIC Multiprocessor HAL configures the RTC to fire every 15.6 milliseconds, which corresponds to about 64 times a second.

<details><summary>Identifying High-Frequency Timers:</summary>

- Windows uses ETW to trace all processes and drivers that request a change in the system’s clock interval, displaying the time of the occurrence and the requested interval.
- Can help identifying the causes of poor battery performance on otherwise healthy system.
- To obtain it, simply run `powercfg /energy` ; or with a debugger, for each process, the `EPROCESS` structure contains a number of fields that help identify changes in timer resolution:
```c
   +0x4a8 TimerResolutionLink : _LIST_ENTRY [ 0xfffffa80'05218fd8 - 0xfffffa80'059cd508 ]
   +0x4b8 RequestedTimerResolution : 0
   +0x4bc ActiveThreadsHighWatermark : 0x1d
   +0x4c0 SmallestTimerResolution : 0x2710
   +0x4c8 TimerResolutionStackRecord : 0xfffff8a0'0476ecd0 _PO_DIAG_STACK_RECORD
   // Then:
   lkd> !list "-e -x \"dt nt!_EPROCESS @$extret-@@(#FIELD_OFFSET(nt!_EPROCESS,TimerResolutionLink))
```
</details>

#### Timer Expiration

- One of the main tasks of the ISR associated with the interrupt that the RTC or PIT will generate is to **keep track of system time**, which is mainly done by the `KeUpdateSystemTime` routine.
- Its second job is to keep track of **logical run time**, such as **process/thread execution times** and the **system tick time**, which is the underlying number used by APIs such as `GetTickCount` that developers use to time operations in their applications. This part of the work is performed by `KeUpdateRunTime`. Before doing any of that work, however, `KeUpdateRunTime` checks whether any timers have expired.
- Because the clock fires at known interval multiples, the **bottom bits** of the current system time will be at one of 64 known positions (on an APIC HAL). Windows uses that fact to organize all driver and application timers into **linked lists** based on an **array** where each entry corresponds to a **possible multiple of the system time**. This table, called the *timer table*, is located in the `PRCB`, which enables each processor to perform its own independent timer expiration without needing to acquire a **global lock**.
- Each multiple of the system time that a timer can be associated with is called the *hand*, and it’s stored in the timer object’s **dispatcher header**.
- Therefore, to determine if a clock has expired, it is only necessary to check if there are any timers on the linked list associated with the current hand.
<p align="center"><img src="./assets/per-processor-timer-lists.png" width="400px" height="auto"></p>

- Similarly to how a driver ISR queues a DPC to defer work, the clock ISR requests a DPC software interrupt, setting a flag in the `PRCB` so that the DPC draining mechanism knows timers need expiration.
- Likewise, when updating process/thread runtime, if the clock ISR determines that a thread has expired its quantum, it also queues a DPC software interrupt and sets a different `PRCB` flag.
- Once the IRQL eventually drops down back to `DISPATCH_LEVEL`, as part of DPC processing, these two flags will be picked up.

#### Processor Selection

- A critical determination that must be made when a timer is inserted is to pick the appropriate table to use—in other words, the most optimal processor choice.
- If the timer has **no DPC associated with it**, the kernel **scans all processors** in the current processor’s group that have not been parked. If the current processor is parked, it picks the next processor in the group; otherwise, the current processor is used.
- If the timer does **have an associated DPC**, the insertion code simply looks at the target processor associated with the DPC and selects that processor’s timer table.

<details><summary>Listing System Timers:</summary>

- You can use the kernel debugger to dump all the current registered timers on the system, as well as information on the DPC associated with each timer (if any) . See the following output for a sample:
```c
   lkd> !timer
   Dump system timers
   Interrupt time: 61876995 000003df [ 4/ 5/2010 18:58:09.189]
   List Timer    Interrupt Low/High     Fire Time              DPC/thread
   PROCESSOR 0 (nt!_KTIMER_TABLE fffff80001bfd080)
     5 fffffa8003099810   627684ac 000003df [ 4/ 5/2010 18:58:10.756]
   NDIS!ndisMTimerObjectDpc (DPC @ fffffa8003099850)
   13 fffffa8003027278   272dde78 000004cf [ 4/ 6/2010 23:34:30.510]  NDIS!ndisMWakeUpDpcX
   (DPC @ fffffa80030272b8)
       fffffa8003029278   272e0588 000004cf [ 4/ 6/2010 23:34:30.511]  NDIS!ndisMWakeUpDpcX
   (DPC @ fffffa80030292b8)
       fffffa8003025278   272e0588 000004cf [ 4/ 6/2010 23:34:30.511]  NDIS!ndisMWakeUpDpcX
   (DPC @ fffffa80030252b8)
       fffffa8003023278   272e2c99 000004cf [ 4/ 6/2010 23:34:30.512]  NDIS!ndisMWakeUpDpcX
   (DPC @ fffffa80030232b8)
    16 fffffa8006096c20   6c1613a6 000003df [ 4/ 5/2010 18:58:26.901]  thread
   fffffa8006096b60
   ...
```
</details>

#### Intelligent Timer Tick Distribution

- The figure below which shows processors handling the clock ISR and expiring timers, reveals that processor 1 wakes up a number of times (the solid arrows) even when there are no associated expiring timers (the dotted arrows).

<p align="center"><img src="./assets/timer-expiration.png" width="400px" height="auto"></p>

- Because the only other work required that was referenced earlier is to update the overall system time/clock ticks, it’s sufficient to designate merely one processor as the **time-keeping processor** (in this case, processor 0) and allow other processors to remain in their **sleep state**; if they wake, any time-related adjustments can be performed by **resynchronizing** with processor 0.
- Windows does, in fact, make this realization (internally called *intelligent timer tick distribution*), and the figure below shows the processor states under the scenario where processor 1 is sleeping (unlike earlier, when we assumed it was running code).
- :arrow_forward: As the processor detects that the work load is going lower and lower, it decreases its **power consumption** (P states), until it finally reaches an idle state.
<p align="center"><img src="./assets/intelligent-timer-tick-distribution.png" width="400px" height="auto"></p>

#### Timer Coalescing

- Why ? Reducing the amount of software timer-expiration work would both help to decrease latency (by requiring less work at DISPATCH_LEVEL) as well as allow other processors to stay in their sleep states even longer (because we’ve established that the processors wake up only to handle expiring timers, fewer timer expirations result in longer sleep times).
- How ? Combine separate timer hands into an individual hand with multiple expirations.
- Assuming that most drivers and user-mode applications do not particularly care about the exact firing period of their timers :)
- Not all timers are ready to be coalesced into coarser granularities, so Windows enables this mechanism only for timers that have **marked themselves as coalescable**, either through the `KeSetCoalescableTimer` kernel API or through its user-mode counterpart, `SetWaitableTimerEx`.
- Assuming all the timers specified tolerances and are thus coalescable, in the previous scenario, Windows could decide to coalesce the timers as:
<p align="center"><img src="./assets/timer-coalescing.png" width="400px" height="auto"></p>

### Exception Dispatching

- In contrast to interrupts, which can occur at any time, exceptions are conditions that **result directly from the execution of the program that is running**.
- On the x86 and x64 processors, all exceptions have **predefined interrupt numbers** that directly correspond to the entry in the `IDT` that points to the trap handler for a particular exception.
- Table below shows x86-defined exceptions and their assigned interrupt numbers. Because the first entries of the IDT are used for exceptions, hardware interrupts are assigned entries later in the table.

|Interrupt Number|Exception|
|----------------|---------|
|0 |Divide Error|
|1 |Debug (Single Step)|
|2 |Non-Maskable Interrupt (NMI)|
|3 |Breakpoint|
|4 |Overflow|
|5 |Bounds Check|
|6 |Invalid Opcode|
|7 |NPX Not Available|
|8 |Double Fault|
|9 |NPX Segment Overrun|
|10| Invalid Task State Segment (TSS)|
|11| Segment Not Present|
|12| Stack Fault|
|13| General Protection|
|14| Page Fault|
|15| Intel Reserved|
|16| Floating Point|
|17| Alignment Check|
|18| Machine Check|
|19| SIMD Floating Point|

- The kernel traps and handles some of these exceptions **transparently** to user programs (i.e a breakpoint when a debugger is attached).
- A few exceptions are allowed to filter back, untouched, to user mode (memory-access violations).
	- **32-bit apps** can establish **frame-based** exception handlers to deal with exceptions.
		- The term frame-based refers to an exception handler’s association with a particular **procedure activation**.
		- When a procedure is invoked, a **stack frame** representing that activation of the procedure is pushed onto the stack.
		- A stack frame can have one or more exception handlers associated with it, each of which protects a particular block of code in the source program.
		- When an exception occurs, the kernel searches for an exception handler associated with the current stack frame.
		- If none exists, the kernel searches for an exception handler associated with the previous stack frame, and so on, until it finds a frame-based exception handler. If no exception handler is found, the kernel calls its **own default exception handlers**.
	- For **64-bit apps**, SEH does not use frame-based handlers.
		- Instead, a table of handlers for each function is **built into the image during compilation**.
		- The kernel looks for handlers associated with each function and generally follows the same algorithm we described for 32-bit code.
- When an exception occurs, whether it is explicitly raised by software or implicitly raised by hardware, a chain of events begins in the kernel.
	1. The CPU hardware transfers control to the kernel trap handler, which creates a **trap frame** (as it does when an interrupt occurs).
	2. The trap frame allows the system to resume where it left off if the exception is resolved.
	3. The trap handler also creates an **exception record** that contains the reason for the exception and other pertinent information.
- If the exception occurred in **kernel mode**, the exception dispatcher simply calls a routine to locate a frame-based exception handler that will handle the exception.
- If the exception occurred in **user mode**, the kernel uses a **debug object/port** in its default exception handling. The Windows subsystem has a **debugger port** and an **exception port** to receive notification of user-mode exceptions in Windows processes.
<p align="center"><img src="./assets/exception-dispatching.png" width="400px" height="auto"></p>

- If the process has no debugger process attached or if the debugger doesn’t handle the exception, the exception dispatcher switches into user mode, copies the trap frame to the user stack formatted as a `CONTEXT` data structure, and calls a routine to find a SE/VE handler. If none is found or if none handles the exception, the exception dispatcher switches back into kernel mode and calls the debugger again to allow the user to do more debugging. This is called the **second-chance notification**.
- If the debugger isn’t running and no user-mode exception handlers are found, the kernel sends a message to the exception port associated with the thread’s process. The exception port gives the environment subsystem the opportunity to translate the exception into an environment-specific signal or exception.
- If the kernel progresses this far in processing the exception and the subsystem doesn’t handle the exception, the kernel sends a message to a **systemwide error port** that *Csrss* uses for *Windows Error Reporting* (WER), and executes a default exception handler that simply terminates the process whose thread caused the exception (often by launching the *WerFault.exe*).

#### Windows Error Reporting (WRE)

- WER is a sophisticated mechanism that automates the submission of both user-mode process crashes as well as kernel-mode system crashes.
- When an unhandled exception is caught by the unhandled exception filter, it builds context information (registers and stack) and opens an **ALPC port** connection to the WER service.
- WER service begins to analyze the crashed program’s state and performs the appropriate actions to notify the user, in most cases this means launching the *WerFault.exe* program.
- In default configured systems, an error report (a minidump and an XML file with various details) is sent to Microsoft’s online crash analysis server.
- WER performs this work **externally** from the **crashed thread** if the unhandled exception filter itself crashes, which allows any kind of process or thread crash to be logged and for the user to be notified.
- WER service uses an ALPC port for communicating with crashed processes. This mechanism uses a systemwide error port that the WER service registers through `NtSetInformationProcess` (which uses `DbgkRegisterErrorPort`). As a result, all Windows processes now have an error port that is actually an ALPC port object registered by the WER service.
<details><summary>More details:</summary>
The kernel, which is first notified of an exception, uses this port to send a message to the WER service, which then analyzes the crashing process. This means that even in severe cases of thread state damage, WER will still be able to receive notifications and launch WerFault.exe to display a user interface instead of having to do this work within the crashing thread itself. Additionally, WER will be able to generate a crash dump for the process, and a message will be written to the Event Log. This solves all the problems of silent process death: users are notified, debugging can occur, and service administrators can see the crash event.
</details>

### System Service Dispatching

- On x86 processors prior to the *Pentium II*, Windows uses the `int 0x2e` instruction, which results in a trap. Windows fills in entry 46 in the IDT to point to the system service dispatcher.
- The trap causes the executing thread to transition into kernel mode and enter the system service dispatcher.
- `EAX` register indicates the **system service number** being requested.
- `EDX` register points to the **list of parameters** the caller passes to the system service.
- To return to user mode, the system service dispatcher uses the `iret` (interrupt return instruction).
- On x86 *Pentium II*+, Windows uses the `sysenter` instruction, which Intel defined specifically for **fast system service dispatches**.
    - To support the instruction, Windows stores at boot time the **address** of the kernel’s **system service dispatcher** routine in a **machine-specific register** (MSR) associated with the instruction.
	- EAX and EDX plays the same role as for the INT instruction.
	- To return to user mode, the system service dispatcher usually executes the `sysexit` instruction.
	- (In some cases, like when the **single-step flag** is enabled on the processor, the system service dispatcher uses the `iret` instead because `sysexit` does not allow returning to user-mode with a different `EFLAGS` register, which is needed if `sysenter` was executed while the trap flag was set as a result of a **user-mode debugger** tracing or stepping over a system call.)
- On the x64 architecture, Windows uses the `syscall` instruction, passing the **system call number** in the `EAX` register, the **first four parameters** in **registers**, and any parameters beyond those four on the **stack**.

<details><summary>Locating the System Service Dispatcher:</summary>

- To see the handler on 32-bit systems for the `interrupt 2E` version of the system call dispatcher:
```c
   lkd> !idt 2e
   Dumping IDT:
   2e:    8208c8ee nt!KiSystemService
 ```
- To see the handler for the `sysenter` version, use the `rdmsr` debugger command to read from the MSR register 0x176, which stores the handler:
```c
   lkd> rdmsr 176
   msr[176] = 000000008208c9c0

    lkd> ln 000000008208c9c0
           (8208c9c0)   nt!KiFastCallEntry
```
- If you have a 64-bit machine, you can look at the service call dispatcher by using the `0xC0000082` MSR instead. You will see it corresponds to `nt!KiSystemCall64`:
```c
   lkd> rdmsr c0000082
   msr[c0000082] = fffff80001a71ec0
   lkd> ln fffff80001a71ec0
   (fffff80001a71ec0)   nt!KiSystemCall64
 ```
- You can disassemble the `KiSystemService` or `KiSystemCall64` routine. On a 32-bit system, you’ll eventually notice the following instructions:
```c
   nt!KiSystemService+0x7b:
   8208c969 897d04          mov     dword ptr [ebp+4],edi
   8208c96c fb              sti
   8208c96d e9dd000000      jmp     nt!KiFastCallEntry+0x8f (8208ca4f)
```
- Because the actual system call dispatching operations are common regardless of the mechanism used to reach the handler, the older interrupt-based handler simply calls into the middle of the newer sysenter-based handler to perform the same generic tasks.
- In 32-bit Windows:
```c
0:000> u ntdll!NtReadFile
ntdll!ZwReadFile:
77020074 b802010000		mov     eax,102h
77020079 ba0003fe7f		mov     edx,offset SharedUserData!SystemCallStub (7ffe0300) // system service dispatch code SystemCallStub member
                                                                                    // of the KUSER_SHARED_DATA
7702007e ff12			call    dword ptr [edx]
77020080 c22400			ret 	24h
77020083 90				nop
```
- Because 64-bit systems have only one mechanism for performing system calls, the system service entry points in *Ntdll.dll* use the syscall instruction directly:
```c
ntdll!NtReadFile:
0000000077f9fc60 4c8bd1
0000000077f9fc63 b810200000
0000000077f9fc68 0f05             syscall
0000000077f9fc6a c3               ret
```
</details>

#### Kernel-Mode System Service Dispatching

- The kernel uses the system call number to locate the system service information in the **system service dispatch table (SSDT)**.
- On 32-bit systems, this table is similar to the interrupt dispatch table described earlier in the chapter except that each entry contains a **pointer to a system service** rather than to an interrupt-handling routine.
- On 64-bit systems, the table is implemented slightly differently—instead of **containing pointers to the system service**, it contains **offsets relative to the table itself**.
<p align="center"><img src="./assets/system-service-exceptions.png" width="400px" height="auto"></p>

- `KiSystemService`, copies the caller’s arguments from the thread’s user-mode stack to its kernel-mode stack and then executes the system service.
- The kernel knows how many stack bytes require copying by using a second table, called the **argument table**, which is a byte array (instead of a pointer array like the dispatch table), each entry describing the number of bytes to copy.
- On 64-bit systems, Windows actually encodes this information within the service table itself through a process called **system call table compaction**.
- The **previous mode** is a value (kernel or user) that the kernel saves in the thread whenever it executes a trap handler and identifies the privilege level of the incoming exception, trap, or system call.
	- If a system call comes from a driver or the kernel itself, the probing and capturing of parameters is skipped, and all parameters are assumed to be pointing to valid kernel-mode buffers.
	- If it is coming from user-mode, the buffers **must be probed**.
- Drivers should call the Zw* version (which is a trampoline to the Nt* system call) of the system calls, because they set the previous mode to kernel. Instead of generating an interrupt or a `sysenter`, which would be slow and/or unsupported, they build a fake interrupt stack (the stack that the CPU would generate after an interrupt) and call the `KiSystemService` routine directly, essentially emulating the CPU interrupt.
- Windows has two system service tables, and third-party drivers cannot extend the tables or insert new ones to add their own service calls. On 32-bit and IA64 versions of Windows, the system service dispatcher locates the tables via a pointer in the **thread kernel structure**, and on x64 versions it finds them via their **global addresses**.
	- The system service dispatcher determines which table contains the requested service by interpreting a **2-bit** field in the 32-bit system service number as a table index.
	- The low **12 bits** of the system service number serve as the index into the table specified by the table index.

<p align="center"><img src="./assets/system-service-tables.png" width="400px" height="auto"></p>

#### Service Descriptor Tables

- A primary default array table, `KeServiceDescriptorTable`, defines the core executive system services implemented in *Ntosrknl.exe*.
- The other table array, `KeServiceDescriptorTableShadow`, includes the Windows USER and GDI services implemented in *Win32k.sys*.
- The difference between System service dispatching for Windows kernel API and a USER/GDI API call.
<p align="center"><img src="./assets/system-service-dispatching.png" width="400px" height="auto"></p>

<details><summary>Mapping System Call Numbers to Functions and Arguments:</summary>

- The `KeServiceDescriptorTable` and `KeServiceDescriptorTableShadow` tables both point to the same array of pointers (or offsets, on 64-bit) for kernel system calls, called `KiServiceTable`.
- On a 32-bit system, you can use the kernel debugger command `dds` to dump the data along with symbolic information.
```c
lkd> dds KiServiceTable
     820807d0  821be2e5 nt!NtAcceptConnectPort
     820807d4  820659a6 nt!NtAccessCheck
     820807d8  8224a953 nt!NtAccessCheckAndAuditAlarm
     820807dc  820659dd nt!NtAccessCheckByType
     820807e0  8224a992 nt!NtAccessCheckByTypeAndAuditAlarm
     820807e4  82065a18 nt!NtAccessCheckByTypeResultList
     820807e8  8224a9db nt!NtAccessCheckByTypeResultListAndAuditAlarm
     820807ec  8224aa24 nt!NtAccessCheckByTypeResultListAndAuditAlarmByHandle
     820807f0  822892af nt!NtAddAtom
```
- On a 64-bit system, the base of the pointer is the `KiServiceTable` itself, so you’ll have to dump the data in its raw format with the `dq` command:
```c
lkd> dq nt!KiServiceTable
	fffff800'01a73b00  02f6f000'04106900 031a0105'fff72d00
```
</details>

...

## Synchronization

- The concept of **mutual exclusion** is a crucial one in OS development It refers to the guarantee that one, and only one, thread can access a particular resource at a time.
- Resources that aren’t subject to **modification** can be shared without worrying about synchronization.
- Developers have to worry about shared data in multi-threading program even in **single-processor** system.

### High-IRQL Synchronization

- Before using a global resource, the kernel **temporarily masks** the interrupts whose interrupt handlers also use the resource.
- It does so by **raising** the processor’s IRQL to the highest level used by any potential interrupt source that accesses the global data.
- 🤷‍♂️ This strategy is fine for a **single-processor** system, but it’s inadequate for a **multiprocessor** configuration.
    - Raising the IRQL on one processor doesn’t prevent an interrupt from occurring on another processor.

#### Interlocked Operations

- Rely on **hardware** support for multiprocessor safe manipulation of integer values and for performing comparisons.

#### Spinlocks

- The mechanism the kernel uses to achieve multiprocessor mutual exclusion is called a **spinlock**.
- Before entering a critical code region, the kernel must acquire the spinlock.
    - If the spinlock isn’t free, the kernel keeps trying to acquire the lock until it succeeds.
    - The spinlock gets its name from the fact that the kernel (and thus, the processor) waits, *spinning* until it gets the lock.
- Are implemented with a hardware-supported *test-and-set* operation, which tests the value of a lock variable and acquires the lock in one **atomic** instruction.
- Additionally, the `lock` instruction can also be used on the *test-and-set* operation, resulting in the combined `lock bts` assembly operation, which also locks the **multiprocessor bus**; otherwise, it would be possible for more than one processor to atomically perform the operation.

<details><summary>All kernel-mode spinlocks in Windows have an associated IRQL that is always DPC/dispatch level or higher. 🚩</summary>

-  When a thread is trying to acquire a spinlock, all other activity at the spinlock’s IRQL or lower ceases on that processor.
    - Because thread dispatching happens at DPC/dispatch level, a thread that holds a spinlock is **never preempted** because the IRQL masks the dispatching mechanisms.
    - This masking allows code executing in a CS protected by a spinlock to **continue executing** so that it will release the lock quickly.
    - Any processor that attempts to acquire the spinlock will essentially be busy, waiting indefinitely, **consuming power** (a busy wait results in 100% CPU usage 😣) and performing no actual work.
    - 👍 In x86/x64, the `PAUSE` asm instruction can be inserted in busy wait loops to offer a hint to the processor that the loop instructions it is processing are part of a spinlock (or a similar construct) acquisition loop.
</details>

- The kernel provides a set of kernel functions including: `KeAcquireSpinLock` and `KeReleaseSpinLock`.
- The kernel also exports the `KeAcquireInterruptSpinLock` and `KeReleaseInterruptSpinLock` for device drivers, because raising the IRQL only to DPC/dispatch level this isn’t enough to protect against **interrupts**.
- Devices can use the `KeSynchronizeExecution` API to synchronize an entire function with an ISR, instead of just a CS.
- ⚠️ Because spinlocks always have an IRQL of DPC/dispatch +, code holding a spinlock will crash the system if it attempts to make the **scheduler perform a dispatch operation** or if it causes a **page fault**.

#### Queued Spinlocks

- To increase the scalability of spinlocks, a special type of spinlock, called a **queued spinlock**, is used in most circumstances instead of a standard spinlock.
    - When a processor wants to acquire a queued spinlock (`KeAcquireQueuedSpinLock`) that is currently held, it places its identifier in a **queue** associated with the spinlock.
    - When the processor that’s holding the spinlock **releases** it, it hands the lock over to the **first processor** identified in the queue.
    - In the meantime, a processor waiting for a busy spinlock checks the status not of the **spinlock itself** but of a **per-processor flag** that the processor ahead of it in the queue sets to indicate that the waiting processor’s turn has arrived.
    - 👍 The multiprocessor’s bus isn’t as **heavily trafficked** by interprocessor synchronization.
    - 👍 Enforces **FIFO** ordering to the lock. FIFO ordering means more **consistent performance** across processors accessing the same locks.
- 💁 These locks are reserved for the kernel’s own internal use. Device drivers should use **Instack Queued Spinlocks**.
- Device drivers can use dynamically allocated queued spinlocks with the `KeAcquireInStackQueuedSpinLock` and `KeReleaseInStackQueuedSpinLock` functions.

### Low-IRQL Synchronization

- Because waiting for a spinlock literally **stalls a processor**, spinlocks can be used only under the following strictly limited circumstances:
    - The protected resource must be accessed **quickly** and without complicated interactions with other code
    - The CS code can’t be **paged out of memory**, can’t make references to **pageable data**, can’t call **external procedures** (including system services), and can’t generate interrupts or exceptions.
- The following Kernel Synchronization Mechanisms are available for Kernel mode:
<p align="center"><img src="./assets/kernel-synchronization-mechanisms.png" width="700px" height="auto"></p>

#### Kernel Dispatcher Objects

- The kernel furnishes additional synchronization mechanisms to the executive in the form of kernel objects, known collectively as **dispatcher objects**.
- Each Windows API-visible object that supports synchronization **encapsulates** at least one kernel dispatcher object.
- The executive’s synchronization semantics are visible to Windows programmers through the `WaitForSingleObject` and `WaitForMultipleObjects` functions, which the Windows subsystem implements by calling analogous system services that the object manager supplies.

### Waiting for Dispatcher Objects

- A thread can synchronize with a dispatcher object by waiting for the object’s handle. Doing so causes the kernel to put the thread in a **wait state**.
- At any given moment, a synchronization object is in one of two states: **signaled state** or **nonsignaled state**.
- A thread can’t resume its execution until its wait is satisfied, a condition that occurs when the dispatcher object whose handle the thread is waiting for also undergoes a state change, from the nonsignaled state to the signaled state.

### What Signals an Object?

- The signaled state is defined differently for different objects
<p align="center"><img src="./assets/definition-of-signaled-state.png" width="700px" height="auto"></p>

- Whether a thread’s wait ends when an object is set to the signaled state varies with the type of object the thread is waiting for:
<p align="center"><img src="./assets/selected-kernel-dispatcher-objects.png" height="auto"></p>

- When an object is set to the **signaled** state, **waiting** threads are generally **released** from their wait states immediately.

### Data Structures

- Three data structures are key to tracking **who is waiting**, **how they are waiting**, **what they are waiting for**, and **which state the entire wait operation is at**. These three structures are the *dispatcher header*, the *wait block*, and the *wait status register*.
- The **dispatcher header** is a packed structure because it needs to hold lots of information in a fixed-size structure.
    - It contains some fields which applies only to some specific dispatcher object ;
    - but it also contain information generic for any dispatcher object: the object type, signaled state, and a list of the threads waiting for that object.
- The **wait block** represents a thread waiting for an object:
    - Each thread that is in a wait state has a **list of the wait blocks** that represent the objects the thread is waiting for.
    - Each dispatcher object has a list of the wait blocks that represent **which threads** are waiting for the **object**.
    - The wait block has a pointer to the object being waited for, a pointer to the thread waiting for the object, and a pointer to the next wait block (if the thread is waiting for more than one object).
-The wait block also contains a volatile **wait block state**, which defines the current state of this wait block in the transactional wait operation it is currently being engaged in.
<p align="center"><img src="./assets/wait-data-structures.png" width="700px" height="auto"></p>

- An object undergoes a different wait states:
    - When a thread is instructed to wait for a given object (`WaitForSingleObject()`), it first attempts to enter the in-progress wait state (**WaitInProgress**) by beginning the wait.
        - If there are no pending alerts to the thread, this operation succeeds.
        - Otherwise the thread now enters the **WaitInProgress** state.
    - Once the wait is in progress, the thread can initialize the wait blocks as needed and mark them as **WaitBlockActive** in the process and then proceed to **lock** all the objects that are part of this wait.
    - The next step is to check for immediate satisfaction of the wait, such as an mutex that has already been released or a timer that already expired, in this cases, the wait is not "satisfied", ▶️ perform a wait exit.
    - If none of these shortcuts were effective, the wait block is inserted into the thread’s wait list, and the thread now attempts to **commit** its wait.
    - it is possible and likely that the thread attempting to commit its wait has experienced a change while its wait was still in progress, this causes the associated wait block to enter the **WaitBlockBypassStart** state, and the thread’s wait status register now shows the **WaitAborted** wait state.
    - Another possible scenario is for an alert or `APC` to have been issued to the waiting thread, which does not set the **WaitAborted** state but enables one of the corresponding bits in the wait status register. Because **APCs can break waits** (depending on the type of APC, wait mode, and alertability), the APC is delivered and the wait is **aborted**. Other operations that will modify the wait status register without generating a full abort cycle include **modifications** to the **thread’s priority** or **affinity**, which will be processed when exiting the wait due to failure to commit, as with the previous cases mentioned.

### :telescope: Looking at Wait Queues

- You can see the list of objects a thread is waiting for with the kernel debugger’s `!process` command:

```c
kd> !process
THREAD fffffa8005292060 Cid 062c062c.0660 Teb: 000007fffffde000 Win32Thread:
fffff900c01c68f0 WAIT: (WrUserRequest) UserMode Non-Alertable
fffffa80047b8240 SynchronizationEvent
```
<details><summary>You can use the dt command to interpret the dispatcher header of the object like this:</summary>

```c
lkd> dt nt!_DISPATCHER_HEADER fffffa80047b8240
+0x000 Type : 0x1 ''
+0x001 TimerControlFlags : 0 ''
+0x001 Absolute : 0y0
+0x001 Coalescable : 0y0
+0x001 KeepShifting : 0y0
+0x001 EncodedTolerableDelay : 0y00000 (0)
+0x001 Abandoned : 0 ''
+0x001 Signalling : 0 ''
+0x002 ThreadControlFlags : 0x6 ''
+0x002 CpuThrottled : 0y0
+0x002 CycleProfiling : 0y1
+0x002 CounterProfiling : 0y1
+0x002 Reserved : 0y00000 (0)
+0x002 Hand : 0x6 ''
+0x002 Size : 0x6
+0x003 TimerMiscFlags : 0 ''
+0x003 Index : 0y000000 (0)
+0x003 Inserted : 0y0
+0x003 Expired : 0y0
+0x003 DebugActive : 0 ''
+0x003 ActiveDR7 : 0y0
+0x003 Instrumented : 0y0
+0x003 Reserved2 : 0y0000
+0x003 UmsScheduled : 0y0
+0x003 UmsPrimary : 0y0
+0x003 DpcActive : 0 ''
+0x000 Lock : 393217
+0x004 SignalState : 0
+0x008 WaitListHead : _LIST_ENTRY [ 0xfffffa80'047b8248 - 0xfffffa80'047b8248 ]
```
</details>

- Apart from these flags, the `Type` field contains the identifier for the object. This identifier corresponds to a number in the `KOBJECTS` enumeration.
<details><summary>Which you can dump with the debugger like this:</summary>

```c
lkd> dt nt!_KOBJECTS

EventNotificationObject = 0
EventSynchronizationObject = 1
MutantObject = 2
ProcessObject = 3
QueueObject = 4
SemaphoreObject = 5
ThreadObject = 6
GateObject = 7
TimerNotificationObject = 8
TimerSynchronizationObject = 9
Spare2Object = 10
Spare3Object = 11
Spare4Object = 12
Spare5Object = 13
Spare6Object = 14
Spare7Object = 15
Spare8Object = 16
Spare9Object = 17
ApcObject = 18
DpcObject = 19
DeviceQueueObject = 20
EventPairObject = 21
InterruptObject = 22
ProfileObject = 23
ThreadedDpcObject = 24
MaximumKernelObject = 25
```
</details>

- When the wait list head pointers are **identical**, there are either **zero** threads or one **thread** waiting on this object. Dumping a wait block for an object that is part of a multiple wait from a thread, or that multiple threads are waiting on, can yield the following:

```c
dt nt!_KWAIT_BLOCK 0xfffffa80'053cf628
+0x000 WaitListEntry : _LIST_ENTRY [ 0xfffffa80'02efe568 - 0xfffffa80'02803468 ]
+0x010 Thread : 0xfffffa80'053cf520 _KTHREAD
+0x018 Object : 0xfffffa80'02803460
+0x020 NextWaitBlock : 0xfffffa80'053cf628 _KWAIT_BLOCK
+0x028 WaitKey : 0
+0x02a WaitType : 0x1 ''
+0x02b BlockState : 0x2 ''
+0x02c SpareLong : 8
```

### Keyed Events

- Keyed events (which are not documented) were originally implemented to help processes deal with **low-memory** situations when using critical sections (CS).
- They were added to Windows XP as a new kernel object type, and there is always **one global event** `\KernelObjects\CritSecOutOfMemoryEvent`, shared among all processes.
- The implementation of keyed events allows multiple CS (waiters) to use the same **global (per-process) keyed event** handle. This allows the CS functions to operate properly even when memory is temporarily low.
- When a thread waits on or sets the event, they specify a **key**. This key is just a **pointer-sized value**, and represents a unique identifier for the event in question.
    - When a thread sets an event for key `K`, only a single thread that has begun waiting on `K` is woken (like an auto-reset event). Only waiters in the current process are woken, so `K` is effectively isolated between processes although there’s a global event. `K` is most often just a memory address. And there you go: you have an arbitrarily large number of events in the process (bounded by the addressable bytes in the system), but without the cost of allocating a true event object for each one.
- However, keyed events are more than just fallback objects for low-memory conditions, a thread can signal a keyed event **without** any threads on the **waiter list**.
    - In this scenario, the signaling thread instead waits on the **event itself**. Without this fallback, a signaling thread could signal the keyed event during the time that the usermode code saw the keyed event as unsignaled and attempt a wait.
    - The wait might have come after the signaling thread signaled the keyed event, resulting in a **missed pulse**, so the waiting thread would deadlock.
    - By forcing the signaling thread to wait in this scenario, it actually signals the keyed event only when **someone is looking** (waiting).

### Fast Mutexes and Guarded Mutexes

- **Fast mutexes**, which are also known as *executive mutexes*, usually offer better **performance** than mutex objects because, although they are built on **dispatcher event** objects, they perform a wait through the dispatcher only if the fast mutex is **contended** — unlike a standard mutex, which always attempts the acquisition through the dispatcher.
- :warning: Fast mutexes limitations:
    - suitable only when normal kernel-mode APC delivery can be **disabled**.
    - can’t be acquired **recursively**, like mutex objects can.
- **Guarded mutexes** are essentially the same as fast mutexes (although they use a different
synchronization object, the `KGATE`, internally).
    - but instead of disabling APCs by raising the IRQL to APC level, they disable all kernel-mode APC delivery by calling `KeEnterGuardedRegion`.
    - Recall that a **guarded region**, unlike a critical region, **disables both special and normal kernel-mode APCs**, which allows the guarded mutex to avoid raising the IRQL.
- Three differences make guarded mutexes **faster** than fast mutexes:
    - By avoiding raising the IRQL, the kernel can avoid talking to the local `APIC` of every processor on the bus, which is a significant operation on large SMP systems. On uni-processor systems, this isn’t a problem because of lazy IRQL evaluation, but lowering the IRQL might still require accessing the `PIC`.
    - The gate primitive is an **optimized** version of the **event**. By not having both synchronization and notification versions and by being the exclusive object that a thread can wait on, the code for acquiring and releasing a gate is **heavily optimized**. Gates even have their own dispatcher lock instead of acquiring the entire dispatcher database.
    - In the **non-contended** case, the acquisition and release of a guarded mutex works on a single bit, with an atomic bit test-and-reset operation instead of the more complex integer operations fast mutexes perform.

### Executive Resources

- are synchronization mechanism that supports **shared** and **exclusive** access;
- like fast mutexes, they require that normal kernel-mode `APC` delivery be **disabled** before they are acquired.
- Threads waiting to acquire an executive resource for **shared** access wait for a **semaphore**
associated with the resource, and threads waiting to acquire an **executive** resource for exclusive access wait for an **event**.

<details><summary>🔭 Listing Acquired Executive Resources:</summary>

- The kernel debugger `!locks` command searches paged pool for executive resource objects and dumps their state.

```c
lkd> !locks
**** DUMP OF ALL RESOURCE OBJECTS ****
KD: Scanning for held locks.
Resource @ 0x89929320 Exclusively owned
Contention Count = 3911396
Threads: 8952d030-01<*>
KD: Scanning for held locks.......................................
Resource @ 0x89da1a68 Shared 1 owning threads
Threads: 8a4cb533-01<*> *** Actual Thread 8a4cb530
```

- You can examine the details of a specific resource object, including the thread that owns the resource and any threads that are waiting for the resource, by specifying the `–v` switch and the address of the resource:

```c
lkd> !locks -v 0x89929320
Resource @ 0x89929320 Exclusively owned
Contention Count = 3913573
Threads: 8952d030-01<*>
THREAD 8952d030 Cid 0acc.050c Teb: 7ffdf000 Win32Thread: fe82c4c0 RUNNING on processor 0
Not impersonating
DeviceMap 9aa0bdb8
Owning Process 89e1ead8 Image: windbg.exe
Wait Start TickCount 24620588 Ticks: 12 (0:00:00:00.187)
Context Switch Count 772193
UserTime 00:00:02.293
KernelTime 00:00:09.828
Win32 Start Address windbg (0x006e63b8)
Stack Init a7eba000 Current a7eb9c10 Base a7eba000 Limit a7eb7000 Call 0
Priority 10 BasePriority 8 PriorityDecrement 0 IoPriority 2 PagePriority 5
Unable to get context for thread running on processor 1, HRESULT 0x80004001
1 total locks, 1 locks currently held
```
</details>

### Pushlocks

- are another optimized synchronization mechanism built on **gate objects**; like guarded mutexes, they wait for a gate object only when there’s contention on the lock.
- 👍 over the guarded mutex :
    - they can be acquired in **shared** or **exclusive** mode.
    - their main advantage is their **size**: a resource object is *56 bytes*, but a pushlock is *pointer-size*.
- There are two types of pushlocks:
    -  A **normal** pushlock: When a thread acquires a normal pushlock, the pushlock code marks the pushlock as owned if it is not currently owned. If the pushlock is owned exclusively or the thread wants to acquire the thread exclusively and the pushlock is owned on a shared basis, the thread allocates a wait block on the thread’s stack, initializes a gate object in the wait block, and adds the wait block to the wait list associated with the pushlock When a thread releases a pushlock, the thread wakes a waiter, if any are present, by signaling the event in the waiter’s wait block
    - A **cache-aware** pushlock adds layers to the normal (basic) pushlock by allocating a pushlock for **each processor** in the system and associating it with the cache-aware pushlock.
- 👍 Other than a much smaller memory footprint, one of the large advantages that pushlocks have over **executive resources** is that in the *non-contended* case they do not require lengthy accounting and integer operations to perform acquisition or release.
- pushlocks use several algorithmic tricks to avoid **lock convoys** (a situation that can occur when multiple threads of the same priority are all waiting on a lock and little actual work gets done), and they are also **self-optimizing**: the list of threads waiting on a pushlock will be periodically rearranged to provide fairer behavior when the pushlock is released.

### Critical Sections

- are one of the main synchronization primitives that Windows provides to user-mode applications on top of the kernel-based synchronization primitives.
- have one major advantage over their kernel counterparts, which is saving a **round-trip** to **kernel** mode in cases in which the lock is *non-contended*, which is typically **99%** of the time or more.
- performs the locking logic using interlocked CPU operations.
- in contended cases, the kernel must be called to put the thread in a wait state.
- because CSs are **not kernel objects**, they have certain limitations 👎:
    - you cannot obtain a kernel handle to a CS; as such, **no security**, **naming**, or other object manager functionality can be applied to a CS.
    - Two processes cannot use the same CS to coordinate their operations, nor can duplication or inheritance be used.

### User-Mode Resources

- also provide more fine-grained locking mechanisms than kernel primitives.
- not exposed through the Windows API for standard apps but you can use `ntdll` alternatives (`RtlAcquireResourceExclusive`, ..) 🤓.
- can be acquired for shared mode or for exclusive mode, allowing it to function as a **multiple-reader** (shared), **single-writer** (exclusive) lock for data structures such as databases.
- no trip to the kernel is required because none of the threads will be waiting. Only when a thread attempts to acquire the resource for **exclusive** access, or the resource is **already locked** by an exclusive owner, will this be required.
- A resource data structure (`RTL_RESOURCE`) contains handles to a **kernel mutex** as well as a **kernel semaphore** object.
    - When the resource is acquired **exclusively** by more than one thread, the resource uses the mutex because it permits only one owner.
    - When the resource is acquired in shared mode by more than one thread, the resource uses a semaphore because it allows multiple owner counts.

### Condition Variables

- Condition variables provide a Windows native implementation for synchronizing a set of threads that are waiting on a specific result to a **conditional test**.
- Although this operation was possible with other user-mode synchronization methods, there was **no atomic** mechanism to check the result of the conditional test and to begin waiting on a change in the result.
- Before condition variables, it was common to use either a **notification** event or a **synchronization** event (recall that these are referred to as **auto-reset** or **manual**-reset in the Windows API) to signal the change to a variable, such as the state of a worker queue.
    - Waiting for a change required a CS to be acquired and then released, followed by a wait on an event.
    - After the wait, the CS had to be re-acquired. During this series of acquisitions and releases, the thread might have switched contexts, causing problems if one of the threads called `PulseEvent` (a similar problem to the one that keyed events solve by **forcing a wait** for the signaling thread if there is **no waiter**).
- With condition variables, acquisition of the CS can be maintained by the app while `SleepConditionVariableCS` is called and can be released only after the actual work is done.
    - ▶️ This makes writing work-queue code (and similar implementations) much simpler and predictable.
- Internally, condition variables can be thought of as a **port** of the existing **pushlock** algorithms present in kernel mode, with the additional complexity of acquiring and releasing CSs in the `SleepConditionVariableCS` API.
    - 👍 condition variables are **pointer-size** (just like pushlocks),
    - 👍 avoid using the **dispatcher** (which requires a ring transition to kernel mode in this scenario, making the advantage even more noticeable),
    - 👍 automatically optimize the wait list during wait operations, and protect against **lock convoys**,
    - 👍 Additionally, condition variables make full use of **keyed events** instead of the regular **event object** that developers would have used on their own, which makes even **contended** cases more **optimized**.

### Slim Reader-Writer Locks

- If condition variables share a lot of similarities with pushlocks, Slim Reader-Writer (SRW) Locks are nearly identical.
- They are also **pointer-size**, use **atomic** operations for acquisition and release, rearrange their **waiter lists**, protect against lock **convoys**, and can be acquired both in **shared** and **exclusive** mode.
- Some differences from pushlocks, however:
    - SRW Locks cannot be **“upgraded”** or **converted** from **shared** to **exclusive** or vice versa,
    - they cannot be recursively acquired,
    - are exclusive to **user-mode** code, while pushlocks are exclusive to **kernel-mode** code, and the two cannot be shared or exposed from one layer to the other.
- SRW Locks entirely replace CSs in application code, but they also offer **multiple-reader**, **single-writer** functionality.

### Run Once Initialization

- Windows implements *init once*, or *one-time initialization* (also called *run once initialization* internally).
- This mechanism allows for both **synchronous** (meaning that the other threads must wait for initialization to complete) execution of a certain piece of code, as well as **asynchronous** (meaning that the other threads can attempt to do their own initialization and race) execution.
- For the **synchronous case**:  call `InitOnceExecuteOnce` with the *parameter*, *context*, and *run-once* function pointer after initializing an `INIT_ONCE` object with `InitOnceInitialize` API The system will take care of the rest.
- For the  **asynchronous case**: the threads call `InitOnceBeginInitialize` and receive a `BOOLEAN` *pending status* and the *context*.
    - If the *pending status* is `FALSE`, initialization has already taken place, and the thread uses the *context* value for the result (It’s also possible for the function itself to return `FALSE`, meaning that initialization failed ).
    - If the *pending status* comes back as `TRUE`, the thread **should race** to be the first to create the object.
        1. The code that follows performs whatever initialization tasks are required, such as creating objects or allocating memory.
        2. When this work is done, the thread calls `InitOnceComplete` with the result of the work as the context and receives a `BOOLEAN` status.
            - If the *status* is `TRUE`, the thread **won the race**, and the object that it created or allocated is the one that will be the global object. The thread can now save this object or return it to a caller, depending on the usage.
            - In the more complex scenario when the status is `FALSE`, this means that the thread lost the race. The thread must **undo** all the work it did, such as deleting objects or freeing memory, and then call `InitOnceBeginInitialize` again. However, instead of requesting to start a race as it did initially, it uses the `INIT_ONCE_CHECK_ONLY` flag, knowing that it has lost, and requests the winner’s context instead (for example, the objects or memory that were created or allocated by the winner). This returns another status, which can be `TRUE`, meaning that the context is valid and should be used or returned to the caller, or `FALSE`, meaning that initialization failed and nobody has actually been able to perform the work (such as in the case of a low-memory condition, perhaps) 🤷‍♂️.
- The *init once* structure is **pointer-size**, and **inline assembly** versions of the **SRW** acquisition/release code are used for the non-contended case, while **keyed events** are used whe contention has occurred (which happens when the mechanism is used in synchronous mode) and the other threads must wait for initialization. In the asynchronous case, the locks are used in shared mode, so multiple threads can perform initialization at the same time.

### System Worker Threads

- During system initialization, Windows creates several threads in the System process, called **system worker threads**, which exist solely to perform work on behalf of other threads.
- Some device drivers and executive components create their **own** threads dedicated to processing work at **passive** level; however, most use system worker threads instead:
    - 👍 avoids the unnecessary scheduling and memory overhead associated with having additional threads in the system.
- `IoQueueWorkItem` requests a system worker thread’s services by placing a work item on a **queue dispatcher** object where the threads look for work. At some stage, a system worker thread will remove the work item from its queue and execute the driver’s routine. If there aren’t any more, the system worker thread blocks until a work item is placed on the queue.
- There are three types of system worker threads:
    - *Delayed worker threads* execute at **priority 12**, process work items that **aren’t** considered **time-critical**, and can have their **stack paged out** to a paging file while they wait for work items.
    - *Critical worker threads* execute at **priority 13**, process **time-critical** work items, and on Windows Server systems have their **stacks present in physical memory** at all times.
    - A *single hypercritical worker thread* executes at **priority 15** and also keeps its stack in memory.
- The number of delayed and critical worker threads created by the executive’s `ExpWorkerInitialization` function, which is called **early in the boot process**, depends on the amount of **memory present** on the system and whether the system is a **server**.

🔭 EXPERIMENT: Listing System Worker Threads

- You can use the `!exqueue` kernel debugger command to see a listing of system worker threads classified by their type.
- <details><summary>lkd> !exqueue</summary>

```c
Dumping ExWorkerQueue: 820FDE40
**** Critical WorkQueue( current = 0 maximum = 2 )
THREAD 861160b8 Cid 0004.001c Teb: 00000000 Win32Thread: 00000000 WAIT
THREAD 8613b020 Cid 0004.0020 Teb: 00000000 Win32Thread: 00000000 WAIT
THREAD 8613bd78 Cid 0004.0024 Teb: 00000000 Win32Thread: 00000000 WAIT
THREAD 8613bad0 Cid 0004.0028 Teb: 00000000 Win32Thread: 00000000 WAIT
THREAD 8613b828 Cid 0004.002c Teb: 00000000 Win32Thread: 00000000 WAIT
**** Delayed WorkQueue( current = 0 maximum = 2 )
THREAD 8613b580 Cid 0004.0030 Teb: 00000000 Win32Thread: 00000000 WAIT
THREAD 8613b2d8 Cid 0004.0034 Teb: 00000000 Win32Thread: 00000000 WAIT
THREAD 8613c020 Cid 0004.0038 Teb: 00000000 Win32Thread: 00000000 WAIT
THREAD 8613cd78 Cid 0004.003c Teb: 00000000 Win32Thread: 00000000 WAIT
THREAD 8613cad0 Cid 0004.0040 Teb: 00000000 Win32Thread: 00000000 WAIT
THREAD 8613c828 Cid 0004.0044 Teb: 00000000 Win32Thread: 00000000 WAIT
THREAD 8613c580 Cid 0004.0048 Teb: 00000000 Win32Thread: 00000000 WAIT
**** HyperCritical WorkQueue( current = 0 maximum = 2 )
THREAD 8613c2d8 Cid 0004.004c Teb: 00000000 Win32Thread: 00000000 WAIT
```
</details>

### Windows Global Flags

- Windows has a set of flags stored in a **systemwide global variable** named `NtGlobalFlag` that enable various internal debugging, tracing, and validation support in the OS.
- In addition, each image has a set of **global flags** that also turn on internal tracing and validation code
-`Gflags.exe` can be used to view and change the system global flags (either in the registry or in the running system) as well as image global flags.
- You can use the `!gflag` kernel debugger command to view and set the state of the `NtGlobalFlag` kernel variable.

### Advanced Local Procedure Call

-  Windows implements an internal IPC mechanism called Advanced Local Procedure Call, or **ALPC**, which is a high-speed, scalable, and secured facility for message passing arbitrary-size messages.
- ALPC superceded the legacy IPC system called **LPC**. LPC is now emulated on top of ALPC for **compatibility** and has been removed from the kernel (legacy system calls still exist, which get wrapped into ALPC calls).
- Although it is internal, and thus not available for third-party developers, ALPC is widely used in various parts of Windows:
    - **RPC**, a documented API, indirectly use ALPC when they specify local-RPC over the *ncalrpc* transport, a form of RPC used to communicate between processes on the same system Kernel-mode RPC, used by the network stack, also uses ALPC.
    - Whenever a Windows **process and/or thread** starts, as well as during any Windows **subsystem** operation (such as all console I/O), ALPC is used to communicate with the subsystem process (`CSRSS`). ▶️ All subsystems communicate with the session manager (SMSS) over ALPC.
    - Winlogon uses ALPC to communicate with LSASS.
    - The SRM uses ALPC to communicate with the LSASS process.
    - The user-mode **power manager** and **power monitor** communicate with the kernel-mode power manager over ALPC, such as whenever the LCD brightness is changed.
    - **Windows Error Reporting** uses ALPC to receive context information from crashing processes
    - The UMDF enables user-mode drivers to communicate using ALPC.

### Connection Model

- An ALPC connection can be established between two or more user-mode processes or between a kernel-mode component and one or more user-mode processes.
- ALPC exports a single executive object called the **port object** to maintain the state needed for communication.
- A server first creates a server connection port (`NtAlpcCreatePort`), while a client attempts to connect to it (`NtAlpcConnectPort`).
    - If the server was in a listening state, it receives a connection request message and can choose to accept it (`NtAlpcAcceptPort`).
    - In doing so, both the client and server communication ports are created, and each respective endpoint process receives a handle to its communication port.
    - Messages are then sent across this handle (`NtAlpcSendWaitReceiveMessage`), typically in a **dedicated thread**, so that the server can continue listening for connection requests on the original connection port (unless this server expects only one client).
- Once a connection is made, a connection information structure (actually, a **blob**) stores the linkage between all the different ports. <p align="center"><img src="./assets/use-of-alpc-ports.png" width="400px" height="auto"></p>

### Message Model

- Using ALPC, a client and thread using blocking messages each take turns performing a loop around the `NtAlpcSendWaitReplyPort` system call, in which one side sends a request and waits for a reply while the other side does the opposite.
- ALPC supports asynchronous messages, so it’s possible for either side not to block and choose instead to perform some other work.
- ALPC supports the following three methods of exchanging payloads sent with a message:
    1. A message can be sent to another process through the standard **double-buffering** mechanism, in which the kernel maintains a copy of the message, switches to the target process, and copies the data from the kernel’s buffer.
    2. A message can be stored in an ALPC **section** object from which the client and server processes **map views**.
    3. A message can be stored in a **message zone**, which is an MDL that backs the physical pages containing the data and that is mapped into the kernel’s address space.

<details><summary>🔭 EXPERIMENT: Viewing Subsystem ALPC Port Objects</summary>
<p align="center"><img src="./assets/winobj-alpc-port.png" width="400px" height="auto"></p>
</details>

### Asynchronous Operation

- The synchronous model of ALPC is tied to the original **LPC** architecture in the early NT design, and is similar to other blocking IPC mechanisms, such as *Mach ports*.
- ALPC was primarily designed to support **asynchronous** operation as well, which is a requirement for **scalable** RPC and other uses, such as support for **pending I/O** in user-mode drivers.
- ALPC also introduced a feature where blocking calls can have a **timeout** parameter.
- ALPC is more optimized for asynchronous messages and provides three different models for asynchronous notifications:
    1. The first doesn’t actually notify the client or server, but simply copies the data payload.
        - up to the implementor to choose a reliable synchronization method (events, polling. etc..).
        - The data structure used by this model is the ALPC completion list.
    2. The second is a waiting model that uses the Windows **completion port** mechanism (on top of the ALPC completion list).
        - The RPC system in Windows, when using Local RPC (over *ncalrpc*), also makes use of this functionality to provide efficient message delivery by taking advantage of this kernel support.
    3. The third model provides a mechanism for a more basic, kernel-based notification using **executive callback** objects.
        - A driver can register its own callback and context with `NtSetInformationAlpcPort`, after which it will get called whenever a message is received.

### Views, Regions, and Sections

