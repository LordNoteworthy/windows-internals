#include "Driver.h"


// Globals
PDEVICE_OBJECT g_KeyboardDevObj = NULL;
ULONG PendingKey = 0;



NTSTATUS
DispatchPassThrough(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP Irp
)
{
	NTSTATUS status = STATUS_SUCCESS;
	IoCopyCurrentIrpStackLocationToNext(Irp);
	status = IoCallDriver((((PDEVICE_EXTENSION)DeviceObject->DeviceExtension)->LowerKeyboardDevice), Irp);
	return status;
}

NTSTATUS
CompletionRoutine(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP Irp,
	_In_ PVOID Context
)
{	
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Context);

	PCHAR KeyFlag[] = { "KeyDown", "KeyUp", "E0", "E1" };
	NTSTATUS status = STATUS_SUCCESS;

	PKEYBOARD_INPUT_DATA Keys = (PKEYBOARD_INPUT_DATA)Irp->AssociatedIrp.SystemBuffer;
	INT NumKeys = (INT)(Irp->IoStatus.Information / sizeof(KEYBOARD_INPUT_DATA));
	if (Irp->IoStatus.Status == STATUS_SUCCESS) {
		for (INT i = 0; i < NumKeys; i++) {
			KdPrint(("Key pressed is %x (%s)\n", Keys->MakeCode, KeyFlag[Keys->Flags]));
		}
	}

	if (Irp->PendingReturned) {
		IoMarkIrpPending(Irp);
	}

	PendingKey--;
	status = Irp->IoStatus.Status;
	return status;
}

NTSTATUS
DispatchRead(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP Irp
)
{
	NTSTATUS status = STATUS_SUCCESS;

	IoCopyCurrentIrpStackLocationToNext(Irp);
	IoSetCompletionRoutine(Irp, CompletionRoutine, NULL, TRUE, TRUE, TRUE);

	// Track the count of I/O Read Req sent to the keyboard driver
	PendingKey++;

	status = IoCallDriver((((PDEVICE_EXTENSION)DeviceObject->DeviceExtension)->LowerKeyboardDevice), Irp);
	return status;
}

VOID
Unload(
	_In_ PDRIVER_OBJECT DriverObject
)
{
	PDEVICE_OBJECT DeviceObject = DriverObject->DeviceObject;
	LARGE_INTEGER Interval = { 0 };
	
	// Let's first detach our filter driver
	IoDetachDevice(((PDEVICE_EXTENSION)DeviceObject->DeviceExtension)->LowerKeyboardDevice);

	// Make sure all keys were processed
	Interval.QuadPart = -10 * 1000 * 1000;
	while (PendingKey) {
		KeDelayExecutionThread(KernelMode, FALSE, &Interval);
	}

	// Now we can safely delete the device
	IoDeleteDevice(g_KeyboardDevObj);
	KdPrint(("Driver Unload \r\n"));

}


NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING TargetDevice = RTL_CONSTANT_STRING(L"\\Device\\KeyboardClass0");


	// Setup our unload routine
	DriverObject->DriverUnload = Unload;

	// Create device we want to attach to the keyboard device
	status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION), NULL, FILE_DEVICE_KEYBOARD, 0, FALSE, &g_KeyboardDevObj);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create device ! \r\n"));
		return status;
	}

	// Propagate some flags
	g_KeyboardDevObj->Flags |= DO_BUFFERED_IO;
	g_KeyboardDevObj->Flags &= DO_DEVICE_INITIALIZING;

	RtlSecureZeroMemory(g_KeyboardDevObj, sizeof(DEVICE_EXTENSION));

	// Do the attachements
	status = IoAttachDevice(g_KeyboardDevObj, &TargetDevice, &((PDEVICE_EXTENSION)g_KeyboardDevObj->DeviceExtension)->LowerKeyboardDevice);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to attach device ! \r\n"));
		IoDeleteDevice(g_KeyboardDevObj);
		return status;
	}


	for (UCHAR i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		DriverObject->MajorFunction[i] = DispatchPassThrough;
	}
	DriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;

	KdPrint(("Driver load succeeded. \r\n"));
	return status;
}