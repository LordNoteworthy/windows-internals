# Programming the Microsoft Windows Driver Model 2nd ed.


- Windows NT driver model -> WDM (Windows Driver Model) -> WDF (Windows Driver Fondation)
- Checked build (debug), free build(release)
- Driver: mediate communication between the device and the Windows kernel, and expose the device's capabilities to clients such as applications.
- The device object is a data structure that includes pointers to the driver's dispatchfunctions, which allow the I/O manager to communicate with the driver.
- The device objects are arranged in a device stack, with a separate stack for each device. Typically,"device stack" refers to the stack of device objects, plus the associated drivers.
- For example, when the bus driver is installed, it enumerates the devices attached to the bus and requests resources for those devices.

- Devices are usually attached to a standard hardware bus such as PCI or USB. A bus driver typically manages several pieces of hardware that are attached to the physical bus
- The function driver translates the Windows abstraction of a device into the actual commands that are required to transfer data to and from a real device.
- The usual purpose of a filter driver is to modify some of the I/O requests as they pass through the device stack, much like an audio filter modifies an audio stream.
- The Windows I/O model is a general packet based mechanism that handle communication between clients and the device stack. By client we mean kernel subsystem + applications + drivers themselves.
- All Windows I/O requests are carried by I/O request packets (IRPs).
- Windows I/O is inherently asynchronous.
- Applications often use synchronous I/O.

- Data Buffers and I/O Transfer Types:
  -- Buffered IO
  -- Direct IO
  -- Neither buffered not direct I/O

- Kernel-mode drivers use DPCs for purposes such as handling the time-consuming aspects of processing a hardware interrupt.
- The most used IRQL: PASSIVE_LEVEL, DISPATCH_LEVEL, DIRQL.
- Most driver routines do not know their process context and run on an arbitrary thread.
- Driver routines often run at DISPATCH_LEVEL and sometimes at DIRQL.
- The primary synchronization tool for DISPATCH_LEVEL routines is an object called a spin lock.
- When a routine acquires a spin lock, its IRQL is raised to DISPATCH_LEVEL if it is not already running at that level.
- Unlike user mode, in which each process has its own virtual address space, the sharedaddress space in kernel mode means that kernel-mode drivers can corrupt each other's memory as well as system memory.

- A page fault in IRQL >= DISPATCH_LEVEL causes a bsod (IRQL_NOT_LESS_OR_EQUAL).
- For routines running at IRQL < DISPATCH_LEVEL, if fault happen while servicing a page fault => Deadlock & double fault crash.

- Memory pools: paged pool vs Non paged pool.
- An MDL is a structure that describes the buffer and contains a list of the locked pages in kernel memory that constitute the buffer.

- WDF provides a unified driver model for a large range of device types.
- UMDF objects are implemented as COM objects, whereas KMDF objects are implemented as a combination of opaque "handles" and functions that operate on those handles.
