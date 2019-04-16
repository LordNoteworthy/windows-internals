#include "Driver.h"

// Our Fast I/O dispatch table
FAST_IO_DISPATCH g_FastIoDispatch =
{
	sizeof(FAST_IO_DISPATCH),
	FsFilterFastIoCheckIfPossible,
	FsFilterFastIoRead,
	FsFilterFastIoWrite,
	FsFilterFastIoQueryBasicInfo,
	FsFilterFastIoQueryStandardInfo,
	FsFilterFastIoLock,
	FsFilterFastIoUnlockSingle,
	FsFilterFastIoUnlockAll,
	FsFilterFastIoUnlockAllByKey,
	FsFilterFastIoDeviceControl,
	NULL,
	NULL,
	FsFilterFastIoDetachDevice,
	FsFilterFastIoQueryNetworkOpenInfo,
	NULL,
	FsFilterFastIoMdlRead,
	FsFilterFastIoMdlReadComplete,
	FsFilterFastIoPrepareMdlWrite,
	FsFilterFastIoMdlWriteComplete,
	FsFilterFastIoReadCompressed,
	FsFilterFastIoWriteCompressed,
	FsFilterFastIoMdlReadCompleteCompressed,
	FsFilterFastIoMdlWriteCompleteCompressed,
	FsFilterFastIoQueryOpen,
	NULL,
	NULL,
	NULL,
};

NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status = STATUS_SUCCESS;

	// Store our driver object
	g_DriverObject = DriverObject;

	// Setup driver's dispatch table
	for (UCHAR i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		DriverObject->MajorFunction[i] = DispatchPassThrough;
	}
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;

	// Setup Fast I/O dispatch table
	DriverObject->FastIoDispatch = &g_FastIoDispatch;

	// Register callback routine for file system changes
	status = IoRegisterFsRegistrationChange(DriverObject, FsFilterNotificationCallback);

	// Driver unload routine
	DriverObject->DriverUnload = Unload;

	KdPrint(("Driver load succeeded. \r\n"));
	return status;
}

NTSTATUS DispatchPassThrough(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP Irp
)
{
	PFSFILTER_DEVICE_EXTENSION pDevExt = (PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

	IoSkipCurrentIrpStackLocation(Irp);
	return IoCallDriver(pDevExt->AttachedToDeviceObject, Irp);
}

NTSTATUS DispatchCreate(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP           Irp
)
{
	PFILE_OBJECT pFileObject = IoGetCurrentIrpStackLocation(Irp)->FileObject;
	KdPrint(("%wZ\r\n", &pFileObject->FileName));

	return DispatchPassThrough(DeviceObject, Irp);
}

VOID Unload(
	_In_ PDRIVER_OBJECT DriverObject
)
{
	LARGE_INTEGER Interval = { 0 };
	ULONG NumDevices = 0;
	PDEVICE_OBJECT devList[DEVOBJ_LIST_SIZE];


	UNREFERENCED_PARAMETER(DriverObject);

	// Unregistered callback routine for file system changes.
	IoUnregisterFsRegistrationChange(DriverObject, FsFilterNotificationCallback);

	//  This is the loop that will go through all of the devices we are attached
	//  to and detach from them.
	for (;;)
	{
		IoEnumerateDeviceObjectList(
			DriverObject,
			devList,
			sizeof(devList),
			&NumDevices);

		if (0 == NumDevices)
		{
			break;
		}

		NumDevices = min(NumDevices, RTL_NUMBER_OF(devList));

		for (UCHAR i = 0; i < NumDevices; ++i)
		{
			FsFilterDetachFromDevice(devList[i]);
			ObDereferenceObject(devList[i]);
		}

		KeDelayExecutionThread(KernelMode, FALSE, &Interval);
	}

	KdPrint(("Driver Unload \r\n"));
}