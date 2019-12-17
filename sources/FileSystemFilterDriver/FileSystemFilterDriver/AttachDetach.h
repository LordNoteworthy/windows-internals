#pragma once

#include "Driver.h"


// Defines
#define DELAY_ONE_MICROSECOND   (-10)
#define DELAY_ONE_MILLISECOND   (DELAY_ONE_MICROSECOND * 1000)
#define DELAY_ONE_SECOND        (DELAY_ONE_MILLISECOND * 1000)


VOID FsFilterNotificationCallback(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_ BOOLEAN        FsActive
);

NTSTATUS FsFilterAttachToFileSystemDevice(
	_In_ PDEVICE_OBJECT DeviceObject
);

VOID FsFilterDetachFromFileSystemDevice(
	_In_ PDEVICE_OBJECT DeviceObject
);

NTSTATUS FsFilterEnumerateFileSystemVolumes(
	_In_ PDEVICE_OBJECT DeviceObject
);

BOOLEAN FsFilterIsAttachedToDevice(
	_In_ PDEVICE_OBJECT DeviceObject
);

NTSTATUS FsFilterAttachToDevice(
	_In_ PDEVICE_OBJECT         DeviceObject,
	__out_opt PDEVICE_OBJECT*   pFilterDeviceObject
);

VOID FsFilterDetachFromDevice(
	_In_ PDEVICE_OBJECT DeviceObject
);

BOOLEAN FsFilterIsMyDeviceObject(
	_In_ PDEVICE_OBJECT DeviceObject
);