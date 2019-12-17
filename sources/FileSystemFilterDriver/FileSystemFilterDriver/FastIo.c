#include "FastIo.h"

// Fast-IO Handlers

BOOLEAN FsFilterFastIoCheckIfPossible(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ ULONG              Length,
	_In_ BOOLEAN            Wait,
	_In_ ULONG              LockKey,
	_In_ BOOLEAN            CheckForReadOperation,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, FastIoCheckIfPossible))
	{
		return (fastIoDispatch->FastIoCheckIfPossible)(
			FileObject,
			FileOffset,
			Length,
			Wait,
			LockKey,
			CheckForReadOperation,
			IoStatus,
			nextDeviceObject);
	}

	return FALSE;
}

BOOLEAN FsFilterFastIoRead(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ ULONG              Length,
	_In_ BOOLEAN            Wait,
	_In_ ULONG              LockKey,
	__out PVOID             Buffer,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, FastIoRead))
	{
		return (fastIoDispatch->FastIoRead)(
			FileObject,
			FileOffset,
			Length,
			Wait,
			LockKey,
			Buffer,
			IoStatus,
			nextDeviceObject);
	}

	return FALSE;
}

BOOLEAN FsFilterFastIoWrite(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ ULONG              Length,
	_In_ BOOLEAN            Wait,
	_In_ ULONG              LockKey,
	_In_ PVOID              Buffer,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, FastIoWrite))
	{
		return (fastIoDispatch->FastIoWrite)(
			FileObject,
			FileOffset,
			Length,
			Wait,
			LockKey,
			Buffer,
			IoStatus,
			nextDeviceObject);
	}

	return FALSE;
}

BOOLEAN FsFilterFastIoQueryBasicInfo(
	_In_ PFILE_OBJECT       FileObject,
	_In_ BOOLEAN            Wait,
	__out PFILE_BASIC_INFORMATION Buffer,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, FastIoQueryBasicInfo))
	{

		return (fastIoDispatch->FastIoQueryBasicInfo)(
			FileObject,
			Wait,
			Buffer,
			IoStatus,
			nextDeviceObject);
	}

	return FALSE;
}

BOOLEAN FsFilterFastIoQueryStandardInfo(
	_In_ PFILE_OBJECT       FileObject,
	_In_ BOOLEAN            Wait,
	__out PFILE_STANDARD_INFORMATION Buffer,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, FastIoQueryStandardInfo))
	{
		return (fastIoDispatch->FastIoQueryStandardInfo)(
			FileObject,
			Wait,
			Buffer,
			IoStatus,
			nextDeviceObject);
	}

	return FALSE;
}

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
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, FastIoLock))
	{
		return (fastIoDispatch->FastIoLock)(
			FileObject,
			FileOffset,
			Length,
			ProcessId,
			Key,
			FailImmediately,
			ExclusiveLock,
			IoStatus,
			nextDeviceObject);
	}

	return FALSE;
}

BOOLEAN FsFilterFastIoUnlockSingle(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ PLARGE_INTEGER     Length,
	_In_ PEPROCESS          ProcessId,
	_In_ ULONG              Key,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, FastIoUnlockSingle))
	{
		return (fastIoDispatch->FastIoUnlockSingle)(
			FileObject,
			FileOffset,
			Length,
			ProcessId,
			Key,
			IoStatus,
			nextDeviceObject);
	}

	return FALSE;
}

BOOLEAN FsFilterFastIoUnlockAll(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PEPROCESS          ProcessId,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, FastIoUnlockAll))
	{
		return (fastIoDispatch->FastIoUnlockAll)(
			FileObject,
			ProcessId,
			IoStatus,
			nextDeviceObject);
	}

	return FALSE;
}

BOOLEAN FsFilterFastIoUnlockAllByKey(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PVOID              ProcessId,
	_In_ ULONG              Key,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, FastIoUnlockAllByKey))
	{
		return (fastIoDispatch->FastIoUnlockAllByKey)(
			FileObject,
			ProcessId,
			Key,
			IoStatus,
			nextDeviceObject);
	}

	return FALSE;
}

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
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, FastIoDeviceControl))
	{
		return (fastIoDispatch->FastIoDeviceControl)(
			FileObject,
			Wait,
			InputBuffer,
			InputBufferLength,
			OutputBuffer,
			OutputBufferLength,
			IoControlCode,
			IoStatus,
			nextDeviceObject);
	}

	return FALSE;
}

VOID FsFilterFastIoDetachDevice(
	_In_ PDEVICE_OBJECT     SourceDevice,
	_In_ PDEVICE_OBJECT     TargetDevice
)
{
	//
	//  Detach from the file system's volume device object.
	//

	IoDetachDevice(TargetDevice);
	IoDeleteDevice(SourceDevice);
}

BOOLEAN FsFilterFastIoQueryNetworkOpenInfo(
	_In_ PFILE_OBJECT       FileObject,
	_In_ BOOLEAN            Wait,
	__out PFILE_NETWORK_OPEN_INFORMATION Buffer,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, FastIoQueryNetworkOpenInfo))
	{
		return (fastIoDispatch->FastIoQueryNetworkOpenInfo)(
			FileObject,
			Wait,
			Buffer,
			IoStatus,
			nextDeviceObject);
	}

	return FALSE;
}

BOOLEAN FsFilterFastIoMdlRead(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ ULONG              Length,
	_In_ ULONG              LockKey,
	__out PMDL*             MdlChain,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, MdlRead))
	{
		return (fastIoDispatch->MdlRead)(
			FileObject,
			FileOffset,
			Length,
			LockKey,
			MdlChain,
			IoStatus,
			nextDeviceObject);
	}

	return FALSE;
}

BOOLEAN FsFilterFastIoMdlReadComplete(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PMDL               MdlChain,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, MdlReadComplete))
	{
		return (fastIoDispatch->MdlReadComplete)(
			FileObject,
			MdlChain,
			nextDeviceObject);
	}

	return FALSE;
}

BOOLEAN FsFilterFastIoPrepareMdlWrite(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ ULONG              Length,
	_In_ ULONG              LockKey,
	__out PMDL*             MdlChain,
	__out PIO_STATUS_BLOCK  IoStatus,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, PrepareMdlWrite))
	{
		return (fastIoDispatch->PrepareMdlWrite)(
			FileObject,
			FileOffset,
			Length,
			LockKey,
			MdlChain,
			IoStatus,
			nextDeviceObject);
	}

	return FALSE;
}

BOOLEAN FsFilterFastIoMdlWriteComplete(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ PMDL               MdlChain,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, MdlWriteComplete))
	{
		return (fastIoDispatch->MdlWriteComplete)(
			FileObject,
			FileOffset,
			MdlChain,
			nextDeviceObject);
	}

	return FALSE;
}

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
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, FastIoReadCompressed))
	{
		return (fastIoDispatch->FastIoReadCompressed)(
			FileObject,
			FileOffset,
			Length,
			LockKey,
			Buffer,
			MdlChain,
			IoStatus,
			CompressedDataInfo,
			CompressedDataInfoLength,
			nextDeviceObject);
	}

	return FALSE;
}

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
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, FastIoWriteCompressed))
	{
		return (fastIoDispatch->FastIoWriteCompressed)(
			FileObject,
			FileOffset,
			Length,
			LockKey,
			Buffer,
			MdlChain,
			IoStatus,
			CompressedDataInfo,
			CompressedDataInfoLength,
			nextDeviceObject);
	}

	return FALSE;
}

BOOLEAN FsFilterFastIoMdlReadCompleteCompressed(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PMDL               MdlChain,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, MdlReadCompleteCompressed))
	{
		return (fastIoDispatch->MdlReadCompleteCompressed)(
			FileObject,
			MdlChain,
			nextDeviceObject);
	}

	return FALSE;
}

BOOLEAN FsFilterFastIoMdlWriteCompleteCompressed(
	_In_ PFILE_OBJECT       FileObject,
	_In_ PLARGE_INTEGER     FileOffset,
	_In_ PMDL               MdlChain,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, MdlWriteCompleteCompressed))
	{
		return (fastIoDispatch->MdlWriteCompleteCompressed)(
			FileObject,
			FileOffset,
			MdlChain,
			nextDeviceObject);
	}

	return FALSE;
}

BOOLEAN FsFilterFastIoQueryOpen(
	_In_ PIRP               Irp,
	__out PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
	_In_ PDEVICE_OBJECT     DeviceObject
)
{
	//
	//  Pass through logic for this type of Fast I/O
	//

	PDEVICE_OBJECT    nextDeviceObject = ((PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject;
	PFAST_IO_DISPATCH fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

	if (VALID_FAST_IO_DISPATCH_HANDLER(fastIoDispatch, FastIoQueryOpen))
	{
		BOOLEAN result;
		PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

		//
		//  Before calling the next filter, we must make sure their device
		//  object is in the current stack entry for the given IRP
		//

		irpSp->DeviceObject = nextDeviceObject;

		result = (fastIoDispatch->FastIoQueryOpen)(
			Irp,
			NetworkInformation,
			nextDeviceObject);

		//
		//  Always restore the IRP back to our device object
		//

		irpSp->DeviceObject = DeviceObject;
		return result;
	}

	return FALSE;
}