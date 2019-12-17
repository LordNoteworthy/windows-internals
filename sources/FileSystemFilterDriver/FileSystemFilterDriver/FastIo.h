#pragma once

#include "Driver.h"


// Prototypes
BOOLEAN FsFilterFastIoCheckIfPossible(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ ULONG              Length,
	_In_ BOOLEAN            Wait,
	_In_ ULONG              LockKey,
	_In_ BOOLEAN            CheckForReadOperation,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoRead(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ ULONG              Length,
	_In_ BOOLEAN            Wait,
	_In_ ULONG              LockKey,
	__out PVOID             Buffer,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoWrite(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ ULONG              Length,
	_In_ BOOLEAN            Wait,
	_In_ ULONG              LockKey,
	_In_ PVOID              Buffer,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoQueryBasicInfo(
	_In_ PFILE_OBJECT       FileObject,
	_In_ BOOLEAN            Wait,
	__out PFILE_BASIC_INFORMATION Buffer,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoQueryStandardInfo(
	_In_ PFILE_OBJECT       FileObject,
	_In_ BOOLEAN            Wait,
	__out PFILE_STANDARD_INFORMATION Buffer,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoLock(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ PLARGE_INTEGER     Length,
	_In_ PEPROCESS          ProcessId,
	_In_ ULONG              Key,
	_In_ BOOLEAN            FailImmediately,
	_In_ BOOLEAN            ExclusiveLock,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoUnlockSingle(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ PLARGE_INTEGER     Length,
	_In_ PEPROCESS          ProcessId,
	_In_ ULONG              Key,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoUnlockAll(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PEPROCESS          ProcessId,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoUnlockAllByKey(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PVOID              ProcessId,
	_In_ ULONG              Key,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoDeviceControl(
	_In_ PFILE_OBJECT       FileObject,
	_In_ BOOLEAN            Wait,
	_In_opt_ PVOID          InputBuffer,
	_In_ ULONG              InputBufferLength,
	__out_opt PVOID         OutputBuffer,
	_In_ ULONG              OutputBufferLength,
	_In_ ULONG              IoControlCode,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
);

VOID FsFilterFastIoDetachDevice(
	_In_ PDEVICE_OBJECT     SourceDevice,
	_In_ PDEVICE_OBJECT     TargetDevice
);

BOOLEAN FsFilterFastIoQueryNetworkOpenInfo(
	_In_ PFILE_OBJECT       FileObject,
	_In_ BOOLEAN            Wait,
	__out PFILE_NETWORK_OPEN_INFORMATION Buffer,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoMdlRead(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ ULONG              Length,
	_In_ ULONG              LockKey,
	__out PMDL*             MdlChain,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoMdlReadComplete(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PMDL               MdlChain,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoPrepareMdlWrite(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ ULONG              Length,
	_In_ ULONG              LockKey,
	__out PMDL*             MdlChain,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoMdlWriteComplete(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ PMDL               MdlChain,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoReadCompressed(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ ULONG              Length,
	_In_ ULONG              LockKey,
	__out PVOID             Buffer,
	__out PMDL*             MdlChain,
	__out PIO_STATUS_BLOCK  IoStatus,
	__out struct _COMPRESSED_DATA_INFO* CompressedDataInfo,
	_In_ ULONG              CompressedDataInfoLength,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoWriteCompressed(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ ULONG              Length,
	_In_ ULONG              LockKey,
	_In_ PVOID              Buffer,
	__out PMDL*             MdlChain,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ struct _COMPRESSED_DATA_INFO*  CompressedDataInfo,
	_In_ ULONG              CompressedDataInfoLength,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoMdlReadCompleteCompressed(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PMDL               MdlChain,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoMdlWriteCompleteCompressed(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ PMDL               MdlChain,
	_In_ PDEVICE_OBJECT     DeviceObject
);

BOOLEAN FsFilterFastIoQueryOpen(
	_In_ PIRP               Irp,
	__out PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
	_In_ PDEVICE_OBJECT     DeviceObject
);

