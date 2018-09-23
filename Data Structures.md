#### _DRIVER_OBJECT
```
kd> dt nt!_DRIVER_OBJECT 
   +0x000 Type             : Int2B
   +0x002 Size             : Int2B
   +0x004 DeviceObject     : Ptr32 _DEVICE_OBJECT
   +0x008 Flags            : Uint4B
   +0x00c DriverStart      : Ptr32 Void
   +0x010 DriverSize       : Uint4B
   +0x014 DriverSection    : Ptr32 Void
   +0x018 DriverExtension  : Ptr32 _DRIVER_EXTENSION
   +0x01c DriverName       : _UNICODE_STRING
   +0x024 HardwareDatabase : Ptr32 _UNICODE_STRING
   +0x028 FastIoDispatch   : Ptr32 _FAST_IO_DISPATCH
   +0x02c DriverInit       : Ptr32     long 
   +0x030 DriverStartIo    : Ptr32     void 
   +0x034 DriverUnload     : Ptr32     void 
   +0x038 MajorFunction    : [28] Ptr32     long 
```

#### _DEVICE_OBJECT
```
kd> dt nt!_DEVICE_OBJECT 
   +0x000 Type             : Int2B
   +0x002 Size             : Uint2B
   +0x004 ReferenceCount   : Int4B
   +0x008 DriverObject     : Ptr32 _DRIVER_OBJECT
   +0x00c NextDevice       : Ptr32 _DEVICE_OBJECT
   +0x010 AttachedDevice   : Ptr32 _DEVICE_OBJECT
   +0x014 CurrentIrp       : Ptr32 _IRP
   +0x018 Timer            : Ptr32 _IO_TIMER
   +0x01c Flags            : Uint4B
   +0x020 Characteristics  : Uint4B
   +0x024 Vpb              : Ptr32 _VPB
   +0x028 DeviceExtension  : Ptr32 Void
   +0x02c DeviceType       : Uint4B
   +0x030 StackSize        : Char
   +0x034 Queue            : <unnamed-tag>
   +0x05c AlignmentRequirement : Uint4B
   +0x060 DeviceQueue      : _KDEVICE_QUEUE
   +0x074 Dpc              : _KDPC
   +0x094 ActiveThreadCount : Uint4B
   +0x098 SecurityDescriptor : Ptr32 Void
   +0x09c DeviceLock       : _KEVENT
   +0x0ac SectorSize       : Uint2B
   +0x0ae Spare1           : Uint2B
   +0x0b0 DeviceObjectExtension : Ptr32 _DEVOBJ_EXTENSION
   +0x0b4 Reserved         : Ptr32 Void
```

#### _IRP
```
kd> dt nt!_IRP 
   +0x000 Type             : Int2B
   +0x002 Size             : Uint2B
   +0x004 MdlAddress       : Ptr32 _MDL
   +0x008 Flags            : Uint4B
   +0x00c AssociatedIrp    : <unnamed-tag>
   +0x010 ThreadListEntry  : _LIST_ENTRY
   +0x018 IoStatus         : _IO_STATUS_BLOCK
   +0x020 RequestorMode    : Char
   +0x021 PendingReturned  : UChar
   +0x022 StackCount       : Char
   +0x023 CurrentLocation  : Char
   +0x024 Cancel           : UChar
   +0x025 CancelIrql       : UChar
   +0x026 ApcEnvironment   : Char
   +0x027 AllocationFlags  : UChar
   +0x028 UserIosb         : Ptr32 _IO_STATUS_BLOCK
   +0x02c UserEvent        : Ptr32 _KEVENT
   +0x030 Overlay          : <unnamed-tag>
   +0x038 CancelRoutine    : Ptr32     void 
   +0x03c UserBuffer       : Ptr32 Void
   +0x040 Tail             : <unnamed-tag>
```

#### _IO_STACK_LOCATION
```
kd> dt nt!_IO_STACK_LOCATION
   +0x000 MajorFunction    : UChar
   +0x001 MinorFunction    : UChar
   +0x002 Flags            : UChar
   +0x003 Control          : UChar
   +0x004 Parameters       : <unnamed-tag>
   +0x014 DeviceObject     : Ptr32 _DEVICE_OBJECT
   +0x018 FileObject       : Ptr32 _FILE_OBJECT
   +0x01c CompletionRoutine : Ptr32     long 
   +0x020 Context          : Ptr32 Void
```