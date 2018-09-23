#include <ntddk.h>
#include <ntstrsafe.h>


// User mode controller
// HANDLE hDevice = CreateFile(L"\\\\.\\dummydriverlink", GENERIC_ALL, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0) ==> will trigger the IRP_MJ_CREATE.
// CloseHandle(hDevice) ==> will trigger the IRP_MJ_CLOSE.

// #include "winioctl.h" // so you can use the CTL_CODE macro
// WCHAR* szMessage = L"dummy message from userland";
//  == Send ioctl ==
// DeviceIoControl(hDevice, DEVICE_SEND, wcslen(szMessage)*sizeof(WCHAR)+2, NULL, 0, &ReturnLength, NULL);
// == Recv ioctl ==
// WCHAR szMessage [1024] = {0};
// DeviceIoControl(hDevice, DEVICE_RECV, NULL, 0, szMessage, 1024, &ReturnLength, NULL, );


// Supported IOCTLs
#define DEVICE_SEND CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_WRITE_DATA)
#define DEVICE_RECV CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_DATA)

// Globals
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
	default:
		status = STATUS_INVALID_PARAMETER;
		break;
	}

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS
DispatchIoctl(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION IoStack = IoGetCurrentIrpStackLocation(Irp);

	// ULONG InBufferLength = IoStack->Parameters.DeviceIoControl.InputBufferLength;
	// ULONG OutBufferLength = IoStack->Parameters.DeviceIoControl.OutputBufferLength;
	PVOID Buffer = Irp->AssociatedIrp.SystemBuffer;
	ULONG ReturnLength = 0;
	CONST WCHAR* SecondBuffer = L"message returned from the driver";
	size_t *pcchLength = NULL;

	switch (IoStack->Parameters.DeviceIoControl.IoControlCode) {
	case DEVICE_SEND:
		KdPrint(("sended data is %ws \r\n request \r\n", Buffer));
		RtlStringCbLengthW(Buffer, 511, pcchLength);
		ReturnLength = *pcchLength + 2;
		break;
	case DEVICE_RECV:
		wcsncpy(Buffer, SecondBuffer, 511);
		RtlStringCbLengthW(Buffer, 511, pcchLength);
		ReturnLength = *pcchLength + 2;
		KdPrint(("received data is %ws \r\n request \r\n"));
		break;
	default:
		status = STATUS_INVALID_PARAMETER;
	}

	Irp->IoStatus.Information = ReturnLength;
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
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CHANGE] = DispatchIoctl;

	KdPrint((L"Driver load succeeded. \r\n"));
	return status;
}