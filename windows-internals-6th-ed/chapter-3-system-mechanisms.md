## Chapter 3 System Mechanisms

### Trap Dispatching

- __Interrupts__ and __exceptions__ are OS conditions that divert the processor to code outside the normal flow of control.
- The term __trap__ refers to a processor’s mechanism for capturing an executing thread when an exception or an interrupt occurs and transferring control to a fixed location in the OS.
- The processor transfers control to a __trap handler__, which is a function specific to a particular interrupt or exception.
- The kernel distinguishes between interrupts and exceptions in the following way:
    - an interrupt is an __asynchronous__ event (one that can occur at any time) that is unrelated to what the processor is executing.
        - are generated primarily by __I/O devices, processor clocks, or timers__, and they can be enabled or disabled.
    - an exception, in contrast, is a __synchronous__ condition that usually results from the execution of a particular instruction.
        - running a program a second time with the same data under the same conditions can reproduce exceptions.
        - examples of exceptions include __memory-access violations__, __certain debugger instructions__, and __divideby-zero__ errors.
        - the kernel also regards system service calls as exceptions (although technically they’re system traps).
    <p align="center"><img src="https://i.imgur.com/UFazASp.png" width="400px" height="auto"></p>
- Either hardware or software can generate exceptions and interrupts:
    - a __bus error__ exception is caused by a hardware problem, whereas a __divide-by-zero__ exception is the result of a software bug.
    - an __I/O device__ can generate an interrupt, or the kernel itself can issue a software interrupt such as an __APC or DPC__.
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

#### Interrupt Dispatching

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
- For example, when an interrupt occurs, the trap handler (or perhaps the processor) raises the processor’s IRQL to the assigned IRQL of the interrupt source. This elevation __masks__ all interrupts __at and below that IRQL__ (on that processor only), which ensures that
the processor servicing the interrupt isn’t waylaid by an interrupt at the __same level or a lower level__. The masked interrupts are either handled by another processor or held back until the IRQL drops. Therefore, all components of the system, including the kernel and device drivers, attempt to __keep the IRQL at passive level__ (sometimes called low level). They do this because device drivers can respond to hardware interrupts in a timelier manner if the IRQL isn’t kept unnecessarily elevated for long periods.
