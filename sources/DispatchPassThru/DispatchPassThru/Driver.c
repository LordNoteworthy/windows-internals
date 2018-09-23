#include <ntddk.h>

// User mode controller
// HANDLE hDevice = CreateFile(L"\\\\.\\dummydriverlink", GENERIC_ALL, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0) ==> will trigger the IRP_MJ_CREATE.
// CloseHandle(hDevice) ==> will trigger the IRP_MJ_CLOSE.


PDEVICE_OBJECT g_DeviceObject = NULL;
UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\dummydriver");
UNICODE_STRING SymLinkName = RTL_CONSTANT_STRING(L"\\??\\dummydriverlink");

NTSTATUS
DispatchPassThru(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);

	switch (IoStack->MajorFunction) {
	case IRP_MJ_CREATE:
		KdPrint(("create request \r\n"));
		break;
	case IRP_MJ_CLOSE:
		KdPrint(("close request \r\n"));
		break;
	case IRP_MJ_READ:
		KdPrint(("read request \r\n"));
		break;
	default:
		break;
	}

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

VOID
Unload(
	_In_ PDRIVER_OBJECT DriverObject
)
{
	UNREFERENCED_PARAMETER(DriverObject);

	IoDeleteSymbolicLink(&SymLinkName);
	IoDeleteDevice(g_DeviceObject);

	KdPrint((L"Driver Unload \r\n"));

}


NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status = STATUS_SUCCESS;

	DriverObject->DriverUnload = Unload;
	status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &g_DeviceObject);
	if (!NT_SUCCESS(status)) {
		KdPrint((L"Failed to create device ! \r\n"));
		return status;
	}

	IoCreateSymbolicLink(&SymLinkName, &DeviceName);
	if (!NT_SUCCESS(status)) {
		KdPrint((L"Failed to create symlink ! \r\n"));
		IoDeleteDevice(g_DeviceObject);
		return status;

	}

	for (UCHAR i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		DriverObject->MajorFunction[i] = DispatchPassThru;
	}

	KdPrint((L"Driver load succeeded. \r\n"));
	return status;
}