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

#### _KPCR
```
kd> dt nt!_KPCR
   +0x000 NtTib            : _NT_TIB
   +0x000 Used_ExceptionList : Ptr32 _EXCEPTION_REGISTRATION_RECORD
   +0x004 Used_StackBase   : Ptr32 Void
   +0x008 Spare2           : Ptr32 Void
   +0x00c TssCopy          : Ptr32 Void
   +0x010 ContextSwitches  : Uint4B
   +0x014 SetMemberCopy    : Uint4B
   +0x018 Used_Self        : Ptr32 Void
   +0x01c SelfPcr          : Ptr32 _KPCR
   +0x020 Prcb             : Ptr32 _KPRCB
   +0x024 Irql             : UChar
   +0x028 IRR              : Uint4B
   +0x02c IrrActive        : Uint4B
   +0x030 IDR              : Uint4B
   +0x034 KdVersionBlock   : Ptr32 Void
   +0x038 IDT              : Ptr32 _KIDTENTRY
   +0x03c GDT              : Ptr32 _KGDTENTRY
   +0x040 TSS              : Ptr32 _KTSS
   +0x044 MajorVersion     : Uint2B
   +0x046 MinorVersion     : Uint2B
   +0x048 SetMember        : Uint4B
   +0x04c StallScaleFactor : Uint4B
   +0x050 SpareUnused      : UChar
   +0x051 Number           : UChar
   +0x052 Spare0           : UChar
   +0x053 SecondLevelCacheAssociativity : UChar
   +0x054 VdmAlert         : Uint4B
   +0x058 KernelReserved   : [14] Uint4B
   +0x090 SecondLevelCacheSize : Uint4B
   +0x094 HalReserved      : [16] Uint4B
   +0x0d4 InterruptMode    : Uint4B
   +0x0d8 Spare1           : UChar
   +0x0dc KernelReserved2  : [17] Uint4B
   +0x120 PrcbData         : _KPRCB
```

#### _KPRCB
kd> dt nt!_KPRCB
   +0x000 MinorVersion     : Uint2B
   +0x002 MajorVersion     : Uint2B
   +0x004 CurrentThread    : Ptr32 _KTHREAD
   +0x008 NextThread       : Ptr32 _KTHREAD
   +0x00c IdleThread       : Ptr32 _KTHREAD
   +0x010 LegacyNumber     : UChar
   +0x011 NestingLevel     : UChar
   +0x012 BuildType        : Uint2B
   +0x014 CpuType          : Char
   +0x015 CpuID            : Char
   +0x016 CpuStep          : Uint2B
   +0x016 CpuStepping      : UChar
   +0x017 CpuModel         : UChar
   +0x018 ProcessorState   : _KPROCESSOR_STATE
   +0x338 KernelReserved   : [16] Uint4B
   +0x378 HalReserved      : [16] Uint4B
   +0x3b8 CFlushSize       : Uint4B
   +0x3bc CoresPerPhysicalProcessor : UChar
   +0x3bd LogicalProcessorsPerCore : UChar
   +0x3be PrcbPad0         : [2] UChar
   +0x3c0 MHz              : Uint4B
   +0x3c4 CpuVendor        : UChar
   +0x3c5 GroupIndex       : UChar
   +0x3c6 Group            : Uint2B
   +0x3c8 GroupSetMember   : Uint4B
   +0x3cc Number           : Uint4B
   +0x3d0 PrcbPad1         : [72] UChar
   +0x418 LockQueue        : [17] _KSPIN_LOCK_QUEUE
   +0x4a0 NpxThread        : Ptr32 _KTHREAD
   +0x4a4 InterruptCount   : Uint4B
   +0x4a8 KernelTime       : Uint4B
   +0x4ac UserTime         : Uint4B
   +0x4b0 DpcTime          : Uint4B
   +0x4b4 DpcTimeCount     : Uint4B
   +0x4b8 InterruptTime    : Uint4B
   +0x4bc AdjustDpcThreshold : Uint4B
   +0x4c0 PageColor        : Uint4B
   +0x4c4 DebuggerSavedIRQL : UChar
   +0x4c5 NodeColor        : UChar
   +0x4c6 PrcbPad20        : [2] UChar
   +0x4c8 NodeShiftedColor : Uint4B
   +0x4cc ParentNode       : Ptr32 _KNODE
   +0x4d0 SecondaryColorMask : Uint4B
   +0x4d4 DpcTimeLimit     : Uint4B
   +0x4d8 PrcbPad21        : [2] Uint4B
   +0x4e0 CcFastReadNoWait : Uint4B
   +0x4e4 CcFastReadWait   : Uint4B
   +0x4e8 CcFastReadNotPossible : Uint4B
   +0x4ec CcCopyReadNoWait : Uint4B
   +0x4f0 CcCopyReadWait   : Uint4B
   +0x4f4 CcCopyReadNoWaitMiss : Uint4B
   +0x4f8 MmSpinLockOrdering : Int4B
   +0x4fc IoReadOperationCount : Int4B
   +0x500 IoWriteOperationCount : Int4B
   +0x504 IoOtherOperationCount : Int4B
   +0x508 IoReadTransferCount : _LARGE_INTEGER
   +0x510 IoWriteTransferCount : _LARGE_INTEGER
   +0x518 IoOtherTransferCount : _LARGE_INTEGER
   +0x520 CcFastMdlReadNoWait : Uint4B
   +0x524 CcFastMdlReadWait : Uint4B
   +0x528 CcFastMdlReadNotPossible : Uint4B
   +0x52c CcMapDataNoWait  : Uint4B
   +0x530 CcMapDataWait    : Uint4B
   +0x534 CcPinMappedDataCount : Uint4B
   +0x538 CcPinReadNoWait  : Uint4B
   +0x53c CcPinReadWait    : Uint4B
   +0x540 CcMdlReadNoWait  : Uint4B
   +0x544 CcMdlReadWait    : Uint4B
   +0x548 CcLazyWriteHotSpots : Uint4B
   +0x54c CcLazyWriteIos   : Uint4B
   +0x550 CcLazyWritePages : Uint4B
   +0x554 CcDataFlushes    : Uint4B
   +0x558 CcDataPages      : Uint4B
   +0x55c CcLostDelayedWrites : Uint4B
   +0x560 CcFastReadResourceMiss : Uint4B
   +0x564 CcCopyReadWaitMiss : Uint4B
   +0x568 CcFastMdlReadResourceMiss : Uint4B
   +0x56c CcMapDataNoWaitMiss : Uint4B
   +0x570 CcMapDataWaitMiss : Uint4B
   +0x574 CcPinReadNoWaitMiss : Uint4B
   +0x578 CcPinReadWaitMiss : Uint4B
   +0x57c CcMdlReadNoWaitMiss : Uint4B
   +0x580 CcMdlReadWaitMiss : Uint4B
   +0x584 CcReadAheadIos   : Uint4B
   +0x588 KeAlignmentFixupCount : Uint4B
   +0x58c KeExceptionDispatchCount : Uint4B
   +0x590 KeSystemCalls    : Uint4B
   +0x594 AvailableTime    : Uint4B
   +0x598 PrcbPad22        : [2] Uint4B
   +0x5a0 PPLookasideList  : [16] _PP_LOOKASIDE_LIST
   +0x620 PPNPagedLookasideList : [32] _GENERAL_LOOKASIDE_POOL
   +0xf20 PPPagedLookasideList : [32] _GENERAL_LOOKASIDE_POOL
   +0x1820 PacketBarrier    : Uint4B
   +0x1824 ReverseStall     : Int4B
   +0x1828 IpiFrame         : Ptr32 Void
   +0x182c PrcbPad3         : [52] UChar
   +0x1860 CurrentPacket    : [3] Ptr32 Void
   +0x186c TargetSet        : Uint4B
   +0x1870 WorkerRoutine    : Ptr32     void 
   +0x1874 IpiFrozen        : Uint4B
   +0x1878 PrcbPad4         : [40] UChar
   +0x18a0 RequestSummary   : Uint4B
   +0x18a4 SignalDone       : Ptr32 _KPRCB
   +0x18a8 PrcbPad50        : [56] UChar
   +0x18e0 DpcData          : [2] _KDPC_DATA
   +0x1908 DpcStack         : Ptr32 Void
   +0x190c MaximumDpcQueueDepth : Int4B
   +0x1910 DpcRequestRate   : Uint4B
   +0x1914 MinimumDpcRate   : Uint4B
   +0x1918 DpcLastCount     : Uint4B
   +0x191c PrcbLock         : Uint4B
   +0x1920 DpcGate          : _KGATE
   +0x1930 ThreadDpcEnable  : UChar
   +0x1931 QuantumEnd       : UChar
   +0x1932 DpcRoutineActive : UChar
   +0x1933 IdleSchedule     : UChar
   +0x1934 DpcRequestSummary : Int4B
   +0x1934 DpcRequestSlot   : [2] Int2B
   +0x1934 NormalDpcState   : Int2B
   +0x1936 DpcThreadActive  : Pos 0, 1 Bit
   +0x1936 ThreadDpcState   : Int2B
   +0x1938 TimerHand        : Uint4B
   +0x193c LastTick         : Uint4B
   +0x1940 MasterOffset     : Int4B
   +0x1944 PrcbPad41        : [2] Uint4B
   +0x194c PeriodicCount    : Uint4B
   +0x1950 PeriodicBias     : Uint4B
   +0x1958 TickOffset       : Uint8B
   +0x1960 TimerTable       : _KTIMER_TABLE
   +0x31a0 CallDpc          : _KDPC
   +0x31c0 ClockKeepAlive   : Int4B
   +0x31c4 ClockCheckSlot   : UChar
   +0x31c5 ClockPollCycle   : UChar
   +0x31c6 PrcbPad6         : [2] UChar
   +0x31c8 DpcWatchdogPeriod : Int4B
   +0x31cc DpcWatchdogCount : Int4B
   +0x31d0 ThreadWatchdogPeriod : Int4B
   +0x31d4 ThreadWatchdogCount : Int4B
   +0x31d8 KeSpinLockOrdering : Int4B
   +0x31dc PrcbPad70        : [1] Uint4B
   +0x31e0 WaitListHead     : _LIST_ENTRY
   +0x31e8 WaitLock         : Uint4B
   +0x31ec ReadySummary     : Uint4B
   +0x31f0 QueueIndex       : Uint4B
   +0x31f4 DeferredReadyListHead : _SINGLE_LIST_ENTRY
   +0x31f8 StartCycles      : Uint8B
   +0x3200 CycleTime        : Uint8B
   +0x3208 HighCycleTime    : Uint4B
   +0x320c PrcbPad71        : Uint4B
   +0x3210 PrcbPad72        : [2] Uint8B
   +0x3220 DispatcherReadyListHead : [32] _LIST_ENTRY
   +0x3320 ChainedInterruptList : Ptr32 Void
   +0x3324 LookasideIrpFloat : Int4B
   +0x3328 MmPageFaultCount : Int4B
   +0x332c MmCopyOnWriteCount : Int4B
   +0x3330 MmTransitionCount : Int4B
   +0x3334 MmCacheTransitionCount : Int4B
   +0x3338 MmDemandZeroCount : Int4B
   +0x333c MmPageReadCount  : Int4B
   +0x3340 MmPageReadIoCount : Int4B
   +0x3344 MmCacheReadCount : Int4B
   +0x3348 MmCacheIoCount   : Int4B
   +0x334c MmDirtyPagesWriteCount : Int4B
   +0x3350 MmDirtyWriteIoCount : Int4B
   +0x3354 MmMappedPagesWriteCount : Int4B
   +0x3358 MmMappedWriteIoCount : Int4B
   +0x335c CachedCommit     : Uint4B
   +0x3360 CachedResidentAvailable : Uint4B
   +0x3364 HyperPte         : Ptr32 Void
   +0x3368 PrcbPad8         : [4] UChar
   +0x336c VendorString     : [13] UChar
   +0x3379 InitialApicId    : UChar
   +0x337a LogicalProcessorsPerPhysicalProcessor : UChar
   +0x337b PrcbPad9         : [5] UChar
   +0x3380 FeatureBits      : Uint4B
   +0x3388 UpdateSignature  : _LARGE_INTEGER
   +0x3390 IsrTime          : Uint8B
   +0x3398 RuntimeAccumulation : Uint8B
   +0x33a0 PowerState       : _PROCESSOR_POWER_STATE
   +0x3468 DpcWatchdogDpc   : _KDPC
   +0x3488 DpcWatchdogTimer : _KTIMER
   +0x34b0 WheaInfo         : Ptr32 Void
   +0x34b4 EtwSupport       : Ptr32 Void
   +0x34b8 InterruptObjectPool : _SLIST_HEADER
   +0x34c0 HypercallPageList : _SLIST_HEADER
   +0x34c8 HypercallPageVirtual : Ptr32 Void
   +0x34cc VirtualApicAssist : Ptr32 Void
   +0x34d0 StatisticsPage   : Ptr32 Uint8B
   +0x34d4 RateControl      : Ptr32 Void
   +0x34d8 Cache            : [5] _CACHE_DESCRIPTOR
   +0x3514 CacheCount       : Uint4B
   +0x3518 CacheProcessorMask : [5] Uint4B
   +0x352c PackageProcessorSet : _KAFFINITY_EX
   +0x3538 PrcbPad91        : [1] Uint4B
   +0x353c CoreProcessorSet : Uint4B
   +0x3540 TimerExpirationDpc : _KDPC
   +0x3560 SpinLockAcquireCount : Uint4B
   +0x3564 SpinLockContentionCount : Uint4B
   +0x3568 SpinLockSpinCount : Uint4B
   +0x356c IpiSendRequestBroadcastCount : Uint4B
   +0x3570 IpiSendRequestRoutineCount : Uint4B
   +0x3574 IpiSendSoftwareInterruptCount : Uint4B
   +0x3578 ExInitializeResourceCount : Uint4B
   +0x357c ExReInitializeResourceCount : Uint4B
   +0x3580 ExDeleteResourceCount : Uint4B
   +0x3584 ExecutiveResourceAcquiresCount : Uint4B
   +0x3588 ExecutiveResourceContentionsCount : Uint4B
   +0x358c ExecutiveResourceReleaseExclusiveCount : Uint4B
   +0x3590 ExecutiveResourceReleaseSharedCount : Uint4B
   +0x3594 ExecutiveResourceConvertsCount : Uint4B
   +0x3598 ExAcqResExclusiveAttempts : Uint4B
   +0x359c ExAcqResExclusiveAcquiresExclusive : Uint4B
   +0x35a0 ExAcqResExclusiveAcquiresExclusiveRecursive : Uint4B
   +0x35a4 ExAcqResExclusiveWaits : Uint4B
   +0x35a8 ExAcqResExclusiveNotAcquires : Uint4B
   +0x35ac ExAcqResSharedAttempts : Uint4B
   +0x35b0 ExAcqResSharedAcquiresExclusive : Uint4B
   +0x35b4 ExAcqResSharedAcquiresShared : Uint4B
   +0x35b8 ExAcqResSharedAcquiresSharedRecursive : Uint4B
   +0x35bc ExAcqResSharedWaits : Uint4B
   +0x35c0 ExAcqResSharedNotAcquires : Uint4B
   +0x35c4 ExAcqResSharedStarveExclusiveAttempts : Uint4B
   +0x35c8 ExAcqResSharedStarveExclusiveAcquiresExclusive : Uint4B
   +0x35cc ExAcqResSharedStarveExclusiveAcquiresShared : Uint4B
   +0x35d0 ExAcqResSharedStarveExclusiveAcquiresSharedRecursive : Uint4B
   +0x35d4 ExAcqResSharedStarveExclusiveWaits : Uint4B
   +0x35d8 ExAcqResSharedStarveExclusiveNotAcquires : Uint4B
   +0x35dc ExAcqResSharedWaitForExclusiveAttempts : Uint4B
   +0x35e0 ExAcqResSharedWaitForExclusiveAcquiresExclusive : Uint4B
   +0x35e4 ExAcqResSharedWaitForExclusiveAcquiresShared : Uint4B
   +0x35e8 ExAcqResSharedWaitForExclusiveAcquiresSharedRecursive : Uint4B
   +0x35ec ExAcqResSharedWaitForExclusiveWaits : Uint4B
   +0x35f0 ExAcqResSharedWaitForExclusiveNotAcquires : Uint4B
   +0x35f4 ExSetResOwnerPointerExclusive : Uint4B
   +0x35f8 ExSetResOwnerPointerSharedNew : Uint4B
   +0x35fc ExSetResOwnerPointerSharedOld : Uint4B
   +0x3600 ExTryToAcqExclusiveAttempts : Uint4B
   +0x3604 ExTryToAcqExclusiveAcquires : Uint4B
   +0x3608 ExBoostExclusiveOwner : Uint4B
   +0x360c ExBoostSharedOwners : Uint4B
   +0x3610 ExEtwSynchTrackingNotificationsCount : Uint4B
   +0x3614 ExEtwSynchTrackingNotificationsAccountedCount : Uint4B
   +0x3618 Context          : Ptr32 _CONTEXT
   +0x361c ContextFlags     : Uint4B
   +0x3620 ExtendedState    : Ptr32 _XSAVE_AREA

#### _ETHREAD
kd> dt nt!_ETHREAD
   +0x000 Tcb              : _KTHREAD
   +0x200 CreateTime       : _LARGE_INTEGER
   +0x208 ExitTime         : _LARGE_INTEGER
   +0x208 KeyedWaitChain   : _LIST_ENTRY
   +0x210 ExitStatus       : Int4B
   +0x214 PostBlockList    : _LIST_ENTRY
   +0x214 ForwardLinkShadow : Ptr32 Void
   +0x218 StartAddress     : Ptr32 Void
   +0x21c TerminationPort  : Ptr32 _TERMINATION_PORT
   +0x21c ReaperLink       : Ptr32 _ETHREAD
   +0x21c KeyedWaitValue   : Ptr32 Void
   +0x220 ActiveTimerListLock : Uint4B
   +0x224 ActiveTimerListHead : _LIST_ENTRY
   +0x22c Cid              : _CLIENT_ID
   +0x234 KeyedWaitSemaphore : _KSEMAPHORE
   +0x234 AlpcWaitSemaphore : _KSEMAPHORE
   +0x248 ClientSecurity   : _PS_CLIENT_SECURITY_CONTEXT
   +0x24c IrpList          : _LIST_ENTRY
   +0x254 TopLevelIrp      : Uint4B
   +0x258 DeviceToVerify   : Ptr32 _DEVICE_OBJECT
   +0x25c CpuQuotaApc      : Ptr32 _PSP_CPU_QUOTA_APC
   +0x260 Win32StartAddress : Ptr32 Void
   +0x264 LegacyPowerObject : Ptr32 Void
   +0x268 ThreadListEntry  : _LIST_ENTRY
   +0x270 RundownProtect   : _EX_RUNDOWN_REF
   +0x274 ThreadLock       : _EX_PUSH_LOCK
   +0x278 ReadClusterSize  : Uint4B
   +0x27c MmLockOrdering   : Int4B
   +0x280 CrossThreadFlags : Uint4B
   +0x280 Terminated       : Pos 0, 1 Bit
   +0x280 ThreadInserted   : Pos 1, 1 Bit
   +0x280 HideFromDebugger : Pos 2, 1 Bit
   +0x280 ActiveImpersonationInfo : Pos 3, 1 Bit
   +0x280 Reserved         : Pos 4, 1 Bit
   +0x280 HardErrorsAreDisabled : Pos 5, 1 Bit
   +0x280 BreakOnTermination : Pos 6, 1 Bit
   +0x280 SkipCreationMsg  : Pos 7, 1 Bit
   +0x280 SkipTerminationMsg : Pos 8, 1 Bit
   +0x280 CopyTokenOnOpen  : Pos 9, 1 Bit
   +0x280 ThreadIoPriority : Pos 10, 3 Bits
   +0x280 ThreadPagePriority : Pos 13, 3 Bits
   +0x280 RundownFail      : Pos 16, 1 Bit
   +0x280 NeedsWorkingSetAging : Pos 17, 1 Bit
   +0x284 SameThreadPassiveFlags : Uint4B
   +0x284 ActiveExWorker   : Pos 0, 1 Bit
   +0x284 ExWorkerCanWaitUser : Pos 1, 1 Bit
   +0x284 MemoryMaker      : Pos 2, 1 Bit
   +0x284 ClonedThread     : Pos 3, 1 Bit
   +0x284 KeyedEventInUse  : Pos 4, 1 Bit
   +0x284 RateApcState     : Pos 5, 2 Bits
   +0x284 SelfTerminate    : Pos 7, 1 Bit
   +0x288 SameThreadApcFlags : Uint4B
   +0x288 Spare            : Pos 0, 1 Bit
   +0x288 StartAddressInvalid : Pos 1, 1 Bit
   +0x288 EtwPageFaultCalloutActive : Pos 2, 1 Bit
   +0x288 OwnsProcessWorkingSetExclusive : Pos 3, 1 Bit
   +0x288 OwnsProcessWorkingSetShared : Pos 4, 1 Bit
   +0x288 OwnsSystemCacheWorkingSetExclusive : Pos 5, 1 Bit
   +0x288 OwnsSystemCacheWorkingSetShared : Pos 6, 1 Bit
   +0x288 OwnsSessionWorkingSetExclusive : Pos 7, 1 Bit
   +0x289 OwnsSessionWorkingSetShared : Pos 0, 1 Bit
   +0x289 OwnsProcessAddressSpaceExclusive : Pos 1, 1 Bit
   +0x289 OwnsProcessAddressSpaceShared : Pos 2, 1 Bit
   +0x289 SuppressSymbolLoad : Pos 3, 1 Bit
   +0x289 Prefetching      : Pos 4, 1 Bit
   +0x289 OwnsDynamicMemoryShared : Pos 5, 1 Bit
   +0x289 OwnsChangeControlAreaExclusive : Pos 6, 1 Bit
   +0x289 OwnsChangeControlAreaShared : Pos 7, 1 Bit
   +0x28a OwnsPagedPoolWorkingSetExclusive : Pos 0, 1 Bit
   +0x28a OwnsPagedPoolWorkingSetShared : Pos 1, 1 Bit
   +0x28a OwnsSystemPtesWorkingSetExclusive : Pos 2, 1 Bit
   +0x28a OwnsSystemPtesWorkingSetShared : Pos 3, 1 Bit
   +0x28a TrimTrigger      : Pos 4, 2 Bits
   +0x28a Spare1           : Pos 6, 2 Bits
   +0x28b PriorityRegionActive : UChar
   +0x28c CacheManagerActive : UChar
   +0x28d DisablePageFaultClustering : UChar
   +0x28e ActiveFaultCount : UChar
   +0x28f LockOrderState   : UChar
   +0x290 AlpcMessageId    : Uint4B
   +0x294 AlpcMessage      : Ptr32 Void
   +0x294 AlpcReceiveAttributeSet : Uint4B
   +0x298 AlpcWaitListEntry : _LIST_ENTRY
   +0x2a0 CacheManagerCount : Uint4B
   +0x2a4 IoBoostCount     : Uint4B
   +0x2a8 IrpListLock      : Uint4B
   +0x2ac ReservedForSynchTracking : Ptr32 Void
   +0x2b0 CmCallbackListHead : _SINGLE_LIST_ENTRY

#### _KTHREAD
kd> dt nt!_KTHREAD
   +0x000 Header           : _DISPATCHER_HEADER
   +0x010 CycleTime        : Uint8B
   +0x018 HighCycleTime    : Uint4B
   +0x020 QuantumTarget    : Uint8B
   +0x028 InitialStack     : Ptr32 Void
   +0x02c StackLimit       : Ptr32 Void
   +0x030 KernelStack      : Ptr32 Void
   +0x034 ThreadLock       : Uint4B
   +0x038 WaitRegister     : _KWAIT_STATUS_REGISTER
   +0x039 Running          : UChar
   +0x03a Alerted          : [2] UChar
   +0x03c KernelStackResident : Pos 0, 1 Bit
   +0x03c ReadyTransition  : Pos 1, 1 Bit
   +0x03c ProcessReadyQueue : Pos 2, 1 Bit
   +0x03c WaitNext         : Pos 3, 1 Bit
   +0x03c SystemAffinityActive : Pos 4, 1 Bit
   +0x03c Alertable        : Pos 5, 1 Bit
   +0x03c GdiFlushActive   : Pos 6, 1 Bit
   +0x03c UserStackWalkActive : Pos 7, 1 Bit
   +0x03c ApcInterruptRequest : Pos 8, 1 Bit
   +0x03c ForceDeferSchedule : Pos 9, 1 Bit
   +0x03c QuantumEndMigrate : Pos 10, 1 Bit
   +0x03c UmsDirectedSwitchEnable : Pos 11, 1 Bit
   +0x03c TimerActive      : Pos 12, 1 Bit
   +0x03c SystemThread     : Pos 13, 1 Bit
   +0x03c Reserved         : Pos 14, 18 Bits
   +0x03c MiscFlags        : Int4B
   +0x040 ApcState         : _KAPC_STATE
   +0x040 ApcStateFill     : [23] UChar
   +0x057 Priority         : Char
   +0x058 NextProcessor    : Uint4B
   +0x05c DeferredProcessor : Uint4B
   +0x060 ApcQueueLock     : Uint4B
   +0x064 ContextSwitches  : Uint4B
   +0x068 State            : UChar
   +0x069 NpxState         : Char
   +0x06a WaitIrql         : UChar
   +0x06b WaitMode         : Char
   +0x06c WaitStatus       : Int4B
   +0x070 WaitBlockList    : Ptr32 _KWAIT_BLOCK
   +0x074 WaitListEntry    : _LIST_ENTRY
   +0x074 SwapListEntry    : _SINGLE_LIST_ENTRY
   +0x07c Queue            : Ptr32 _KQUEUE
   +0x080 WaitTime         : Uint4B
   +0x084 KernelApcDisable : Int2B
   +0x086 SpecialApcDisable : Int2B
   +0x084 CombinedApcDisable : Uint4B
   +0x088 Teb              : Ptr32 Void
   +0x090 Timer            : _KTIMER
   +0x0b8 AutoAlignment    : Pos 0, 1 Bit
   +0x0b8 DisableBoost     : Pos 1, 1 Bit
   +0x0b8 EtwStackTraceApc1Inserted : Pos 2, 1 Bit
   +0x0b8 EtwStackTraceApc2Inserted : Pos 3, 1 Bit
   +0x0b8 CalloutActive    : Pos 4, 1 Bit
   +0x0b8 ApcQueueable     : Pos 5, 1 Bit
   +0x0b8 EnableStackSwap  : Pos 6, 1 Bit
   +0x0b8 GuiThread        : Pos 7, 1 Bit
   +0x0b8 UmsPerformingSyscall : Pos 8, 1 Bit
   +0x0b8 VdmSafe          : Pos 9, 1 Bit
   +0x0b8 UmsDispatched    : Pos 10, 1 Bit
   +0x0b8 ReservedFlags    : Pos 11, 21 Bits
   +0x0b8 ThreadFlags      : Int4B
   +0x0bc ServiceTable     : Ptr32 Void
   +0x0c0 WaitBlock        : [4] _KWAIT_BLOCK
   +0x120 QueueListEntry   : _LIST_ENTRY
   +0x128 TrapFrame        : Ptr32 _KTRAP_FRAME
   +0x12c FirstArgument    : Ptr32 Void
   +0x130 CallbackStack    : Ptr32 Void
   +0x130 CallbackDepth    : Uint4B
   +0x134 ApcStateIndex    : UChar
   +0x135 BasePriority     : Char
   +0x136 PriorityDecrement : Char
   +0x136 ForegroundBoost  : Pos 0, 4 Bits
   +0x136 UnusualBoost     : Pos 4, 4 Bits
   +0x137 Preempted        : UChar
   +0x138 AdjustReason     : UChar
   +0x139 AdjustIncrement  : Char
   +0x13a PreviousMode     : Char
   +0x13b Saturation       : Char
   +0x13c SystemCallNumber : Uint4B
   +0x140 FreezeCount      : Uint4B
   +0x144 UserAffinity     : _GROUP_AFFINITY
   +0x150 Process          : Ptr32 _KPROCESS
   +0x154 Affinity         : _GROUP_AFFINITY
   +0x160 IdealProcessor   : Uint4B
   +0x164 UserIdealProcessor : Uint4B
   +0x168 ApcStatePointer  : [2] Ptr32 _KAPC_STATE
   +0x170 SavedApcState    : _KAPC_STATE
   +0x170 SavedApcStateFill : [23] UChar
   +0x187 WaitReason       : UChar
   +0x188 SuspendCount     : Char
   +0x189 Spare1           : Char
   +0x18a OtherPlatformFill : UChar
   +0x18c Win32Thread      : Ptr32 Void
   +0x190 StackBase        : Ptr32 Void
   +0x194 SuspendApc       : _KAPC
   +0x194 SuspendApcFill0  : [1] UChar
   +0x195 ResourceIndex    : UChar
   +0x194 SuspendApcFill1  : [3] UChar
   +0x197 QuantumReset     : UChar
   +0x194 SuspendApcFill2  : [4] UChar
   +0x198 KernelTime       : Uint4B
   +0x194 SuspendApcFill3  : [36] UChar
   +0x1b8 WaitPrcb         : Ptr32 _KPRCB
   +0x194 SuspendApcFill4  : [40] UChar
   +0x1bc LegoData         : Ptr32 Void
   +0x194 SuspendApcFill5  : [47] UChar
   +0x1c3 LargeStack       : UChar
   +0x1c4 UserTime         : Uint4B
   +0x1c8 SuspendSemaphore : _KSEMAPHORE
   +0x1c8 SuspendSemaphorefill : [20] UChar
   +0x1dc SListFaultCount  : Uint4B
   +0x1e0 ThreadListEntry  : _LIST_ENTRY
   +0x1e8 MutantListHead   : _LIST_ENTRY
   +0x1f0 SListFaultAddress : Ptr32 Void
   +0x1f4 ThreadCounters   : Ptr32 _KTHREAD_COUNTERS
   +0x1f8 XStateSave       : Ptr32 _XSTATE_SAVE

#### _KAPC_STATE
kd> dt nt!_KAPC_STATE
   +0x000 ApcListHead      : [2] _LIST_ENTRY
   +0x010 Process          : Ptr32 _KPROCESS
   +0x014 KernelApcInProgress : UChar
   +0x015 KernelApcPending : UChar
   +0x016 UserApcPending   : UChar

#### _EPROCESS
kd> dt nt!_EPROCESS
   +0x000 Pcb              : _KPROCESS
   +0x098 ProcessLock      : _EX_PUSH_LOCK
   +0x0a0 CreateTime       : _LARGE_INTEGER
   +0x0a8 ExitTime         : _LARGE_INTEGER
   +0x0b0 RundownProtect   : _EX_RUNDOWN_REF
   +0x0b4 UniqueProcessId  : Ptr32 Void
   +0x0b8 ActiveProcessLinks : _LIST_ENTRY
   +0x0c0 ProcessQuotaUsage : [2] Uint4B
   +0x0c8 ProcessQuotaPeak : [2] Uint4B
   +0x0d0 CommitCharge     : Uint4B
   +0x0d4 QuotaBlock       : Ptr32 _EPROCESS_QUOTA_BLOCK
   +0x0d8 CpuQuotaBlock    : Ptr32 _PS_CPU_QUOTA_BLOCK
   +0x0dc PeakVirtualSize  : Uint4B
   +0x0e0 VirtualSize      : Uint4B
   +0x0e4 SessionProcessLinks : _LIST_ENTRY
   +0x0ec DebugPort        : Ptr32 Void
   +0x0f0 ExceptionPortData : Ptr32 Void
   +0x0f0 ExceptionPortValue : Uint4B
   +0x0f0 ExceptionPortState : Pos 0, 3 Bits
   +0x0f4 ObjectTable      : Ptr32 _HANDLE_TABLE
   +0x0f8 Token            : _EX_FAST_REF
   +0x0fc WorkingSetPage   : Uint4B
   +0x100 AddressCreationLock : _EX_PUSH_LOCK
   +0x104 RotateInProgress : Ptr32 _ETHREAD
   +0x108 ForkInProgress   : Ptr32 _ETHREAD
   +0x10c HardwareTrigger  : Uint4B
   +0x110 PhysicalVadRoot  : Ptr32 _MM_AVL_TABLE
   +0x114 CloneRoot        : Ptr32 Void
   +0x118 NumberOfPrivatePages : Uint4B
   +0x11c NumberOfLockedPages : Uint4B
   +0x120 Win32Process     : Ptr32 Void
   +0x124 Job              : Ptr32 _EJOB
   +0x128 SectionObject    : Ptr32 Void
   +0x12c SectionBaseAddress : Ptr32 Void
   +0x130 Cookie           : Uint4B
   +0x134 Spare8           : Uint4B
   +0x138 WorkingSetWatch  : Ptr32 _PAGEFAULT_HISTORY
   +0x13c Win32WindowStation : Ptr32 Void
   +0x140 InheritedFromUniqueProcessId : Ptr32 Void
   +0x144 LdtInformation   : Ptr32 Void
   +0x148 VdmObjects       : Ptr32 Void
   +0x14c ConsoleHostProcess : Uint4B
   +0x150 DeviceMap        : Ptr32 Void
   +0x154 EtwDataSource    : Ptr32 Void
   +0x158 FreeTebHint      : Ptr32 Void
   +0x160 PageDirectoryPte : _HARDWARE_PTE
   +0x160 Filler           : Uint8B
   +0x168 Session          : Ptr32 Void
   +0x16c ImageFileName    : [15] UChar
   +0x17b PriorityClass    : UChar
   +0x17c JobLinks         : _LIST_ENTRY
   +0x184 LockedPagesList  : Ptr32 Void
   +0x188 ThreadListHead   : _LIST_ENTRY
   +0x190 SecurityPort     : Ptr32 Void
   +0x194 PaeTop           : Ptr32 Void
   +0x198 ActiveThreads    : Uint4B
   +0x19c ImagePathHash    : Uint4B
   +0x1a0 DefaultHardErrorProcessing : Uint4B
   +0x1a4 LastThreadExitStatus : Int4B
   +0x1a8 Peb              : Ptr32 _PEB
   +0x1ac PrefetchTrace    : _EX_FAST_REF
   +0x1b0 ReadOperationCount : _LARGE_INTEGER
   +0x1b8 WriteOperationCount : _LARGE_INTEGER
   +0x1c0 OtherOperationCount : _LARGE_INTEGER
   +0x1c8 ReadTransferCount : _LARGE_INTEGER
   +0x1d0 WriteTransferCount : _LARGE_INTEGER
   +0x1d8 OtherTransferCount : _LARGE_INTEGER
   +0x1e0 CommitChargeLimit : Uint4B
   +0x1e4 CommitChargePeak : Uint4B
   +0x1e8 AweInfo          : Ptr32 Void
   +0x1ec SeAuditProcessCreationInfo : _SE_AUDIT_PROCESS_CREATION_INFO
   +0x1f0 Vm               : _MMSUPPORT
   +0x25c MmProcessLinks   : _LIST_ENTRY
   +0x264 HighestUserAddress : Ptr32 Void
   +0x268 ModifiedPageCount : Uint4B
   +0x26c Flags2           : Uint4B
   +0x26c JobNotReallyActive : Pos 0, 1 Bit
   +0x26c AccountingFolded : Pos 1, 1 Bit
   +0x26c NewProcessReported : Pos 2, 1 Bit
   +0x26c ExitProcessReported : Pos 3, 1 Bit
   +0x26c ReportCommitChanges : Pos 4, 1 Bit
   +0x26c LastReportMemory : Pos 5, 1 Bit
   +0x26c ReportPhysicalPageChanges : Pos 6, 1 Bit
   +0x26c HandleTableRundown : Pos 7, 1 Bit
   +0x26c NeedsHandleRundown : Pos 8, 1 Bit
   +0x26c RefTraceEnabled  : Pos 9, 1 Bit
   +0x26c NumaAware        : Pos 10, 1 Bit
   +0x26c ProtectedProcess : Pos 11, 1 Bit
   +0x26c DefaultPagePriority : Pos 12, 3 Bits
   +0x26c PrimaryTokenFrozen : Pos 15, 1 Bit
   +0x26c ProcessVerifierTarget : Pos 16, 1 Bit
   +0x26c StackRandomizationDisabled : Pos 17, 1 Bit
   +0x26c AffinityPermanent : Pos 18, 1 Bit
   +0x26c AffinityUpdateEnable : Pos 19, 1 Bit
   +0x26c PropagateNode    : Pos 20, 1 Bit
   +0x26c ExplicitAffinity : Pos 21, 1 Bit
   +0x270 Flags            : Uint4B
   +0x270 CreateReported   : Pos 0, 1 Bit
   +0x270 NoDebugInherit   : Pos 1, 1 Bit
   +0x270 ProcessExiting   : Pos 2, 1 Bit
   +0x270 ProcessDelete    : Pos 3, 1 Bit
   +0x270 Wow64SplitPages  : Pos 4, 1 Bit
   +0x270 VmDeleted        : Pos 5, 1 Bit
   +0x270 OutswapEnabled   : Pos 6, 1 Bit
   +0x270 Outswapped       : Pos 7, 1 Bit
   +0x270 ForkFailed       : Pos 8, 1 Bit
   +0x270 Wow64VaSpace4Gb  : Pos 9, 1 Bit
   +0x270 AddressSpaceInitialized : Pos 10, 2 Bits
   +0x270 SetTimerResolution : Pos 12, 1 Bit
   +0x270 BreakOnTermination : Pos 13, 1 Bit
   +0x270 DeprioritizeViews : Pos 14, 1 Bit
   +0x270 WriteWatch       : Pos 15, 1 Bit
   +0x270 ProcessInSession : Pos 16, 1 Bit
   +0x270 OverrideAddressSpace : Pos 17, 1 Bit
   +0x270 HasAddressSpace  : Pos 18, 1 Bit
   +0x270 LaunchPrefetched : Pos 19, 1 Bit
   +0x270 InjectInpageErrors : Pos 20, 1 Bit
   +0x270 VmTopDown        : Pos 21, 1 Bit
   +0x270 ImageNotifyDone  : Pos 22, 1 Bit
   +0x270 PdeUpdateNeeded  : Pos 23, 1 Bit
   +0x270 VdmAllowed       : Pos 24, 1 Bit
   +0x270 CrossSessionCreate : Pos 25, 1 Bit
   +0x270 ProcessInserted  : Pos 26, 1 Bit
   +0x270 DefaultIoPriority : Pos 27, 3 Bits
   +0x270 ProcessSelfDelete : Pos 30, 1 Bit
   +0x270 SetTimerResolutionLink : Pos 31, 1 Bit
   +0x274 ExitStatus       : Int4B
   +0x278 VadRoot          : _MM_AVL_TABLE
   +0x298 AlpcContext      : _ALPC_PROCESS_CONTEXT
   +0x2a8 TimerResolutionLink : _LIST_ENTRY
   +0x2b0 RequestedTimerResolution : Uint4B
   +0x2b4 ActiveThreadsHighWatermark : Uint4B
   +0x2b8 SmallestTimerResolution : Uint4B
   +0x2bc TimerResolutionStackRecord : Ptr32 _PO_DIAG_STACK_RECORD

#### _KPROCESS
kd> dt nt!_KPROCESS
   +0x000 Header           : _DISPATCHER_HEADER
   +0x010 ProfileListHead  : _LIST_ENTRY
   +0x018 DirectoryTableBase : Uint4B
   +0x01c LdtDescriptor    : _KGDTENTRY
   +0x024 Int21Descriptor  : _KIDTENTRY
   +0x02c ThreadListHead   : _LIST_ENTRY
   +0x034 ProcessLock      : Uint4B
   +0x038 Affinity         : _KAFFINITY_EX
   +0x044 ReadyListHead    : _LIST_ENTRY
   +0x04c SwapListEntry    : _SINGLE_LIST_ENTRY
   +0x050 ActiveProcessors : _KAFFINITY_EX
   +0x05c AutoAlignment    : Pos 0, 1 Bit
   +0x05c DisableBoost     : Pos 1, 1 Bit
   +0x05c DisableQuantum   : Pos 2, 1 Bit
   +0x05c ActiveGroupsMask : Pos 3, 1 Bit
   +0x05c ReservedFlags    : Pos 4, 28 Bits
   +0x05c ProcessFlags     : Int4B
   +0x060 BasePriority     : Char
   +0x061 QuantumReset     : Char
   +0x062 Visited          : UChar
   +0x063 Unused3          : UChar
   +0x064 ThreadSeed       : [1] Uint4B
   +0x068 IdealNode        : [1] Uint2B
   +0x06a IdealGlobalNode  : Uint2B
   +0x06c Flags            : _KEXECUTE_OPTIONS
   +0x06d Unused1          : UChar
   +0x06e IopmOffset       : Uint2B
   +0x070 Unused4          : Uint4B
   +0x074 StackCount       : _KSTACK_COUNT
   +0x078 ProcessListEntry : _LIST_ENTRY
   +0x080 CycleTime        : Uint8B
   +0x088 KernelTime       : Uint4B
   +0x08c UserTime         : Uint4B
   +0x090 VdmTrapcHandler  : Ptr32 Void
