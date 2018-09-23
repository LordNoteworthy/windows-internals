#include <ntddk.h>

PDEVICE_OBJECT DeviceObject = NULL;
UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\dummydriver");
UNICODE_STRING SymLinkName = RTL_CONSTANT_STRING(L"\\??\\dummydriverlink");

VOID
Unload(
	_In_ PDRIVER_OBJECT DriverObject
)
{
	UNREFERENCED_PARAMETER(DriverObject);

	IoDeleteSymbolicLink(&SymLinkName);
	IoDeleteDevice(DeviceObject);

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
	status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status)) {
		KdPrint((L"Failed to create device ! \r\n"));
		return status;
	}

	IoCreateSymbolicLink(&SymLinkName, &DeviceName);
	if (!NT_SUCCESS(status)) {
		KdPrint((L"Failed to create symlink ! \r\n"));
		IoDeleteDevice(DeviceObject);
		return status;

	}

	KdPrint((L"Driver load succeeded. \r\n"));
	return status;
}