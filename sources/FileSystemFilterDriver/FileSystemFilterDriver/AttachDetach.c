#include "AttachDetach.h"


VOID FsFilterNotificationCallback(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_ BOOLEAN        FsActive
)
{
/*
	This routine is invoked whenever a file system has either registered
	or unregistered itself as an active file system.
*/

	//  Handle attaching/detaching from the given file system.
	if (FsActive)
	{
		FsFilterAttachToFileSystemDevice(DeviceObject);
	}
	else
	{
		FsFilterDetachFromFileSystemDevice(DeviceObject);
	}
}


NTSTATUS FsFilterAttachToFileSystemDevice(
	_In_ PDEVICE_OBJECT DeviceObject
)
{
/*
	This will attach to the given file system device object
*/

	NTSTATUS        status = STATUS_SUCCESS;
	PDEVICE_OBJECT  filterDeviceObject = NULL;

	if (!FsFilterIsAttachedToDevice(DeviceObject))
	{
		status = FsFilterAttachToDevice(DeviceObject, &filterDeviceObject);

		if (!NT_SUCCESS(status))
		{
			return status;
		}

		//  Enumerate all the mounted devices that currently exist for this file system and attach to them.
		status = FsFilterEnumerateFileSystemVolumes(DeviceObject);

		if (!NT_SUCCESS(status))
		{
			FsFilterDetachFromDevice(filterDeviceObject);
			return status;
		}
	}

	return STATUS_SUCCESS;
}


VOID FsFilterDetachFromFileSystemDevice(
	_In_ PDEVICE_OBJECT DeviceObject
)
{
	/*
	This will detach us from the chain
	*/

	PDEVICE_OBJECT device = NULL;

	for (device = DeviceObject->AttachedDevice; NULL != device; device = device->AttachedDevice)
	{
		if (FsFilterIsMyDeviceObject(device))
		{
			//  Detach us from the object just below us. Cleanup and delete the object.
			FsFilterDetachFromDevice(device);

			break;
		}
	}
}


NTSTATUS FsFilterEnumerateFileSystemVolumes(
	_In_ PDEVICE_OBJECT DeviceObject
)
{
/*
	Enumerate all the mounted devices that currently exist for the given file
	system and attach to them
*/

	NTSTATUS        status = STATUS_SUCCESS;
	ULONG           ActualNumberDeviceObjects = 0;
	PDEVICE_OBJECT  DeviceObjectList[DEVOBJ_LIST_SIZE];

	//
	//  Now get the list of devices.
	//

	status = IoEnumerateDeviceObjectList(DeviceObject->DriverObject, DeviceObjectList, sizeof(DeviceObjectList), &ActualNumberDeviceObjects);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	ActualNumberDeviceObjects = min(ActualNumberDeviceObjects, RTL_NUMBER_OF(DeviceObjectList));

	//  Walk the given list of devices and attach to them if we should.
	for (UCHAR i = 0; i < ActualNumberDeviceObjects; ++i)
	{
		//
		//  Do not attach if:
		//      - This is the control device object (the one passed in)
		//      - The device type does not match
		//      - We are already attached to it.
		//

		if (DeviceObjectList[i] != DeviceObject && DeviceObjectList[i]->DeviceType == DeviceObject->DeviceType
			&& !FsFilterIsAttachedToDevice(DeviceObjectList[i]))
		{
			status = FsFilterAttachToDevice(DeviceObjectList[i], NULL);
		}

		ObDereferenceObject(DeviceObjectList[i]);
	}

	return STATUS_SUCCESS;
}


BOOLEAN FsFilterIsAttachedToDevice(
	_In_ PDEVICE_OBJECT DeviceObject
)
{
	/*
	This determines whether we are attached to the given device
	*/

	PDEVICE_OBJECT nextDevObj = NULL;
	PDEVICE_OBJECT currentDevObj = IoGetAttachedDeviceReference(DeviceObject);

	//
	//  Scan down the list to find our device object.
	//

	do
	{
		if (FsFilterIsMyDeviceObject(currentDevObj))
		{
			ObDereferenceObject(currentDevObj);
			return TRUE;
		}

		//
		//  Get the next attached object.
		//

		nextDevObj = IoGetLowerDeviceObject(currentDevObj);

		//
		//  Dereference our current device object, before moving to the next one.
		//

		ObDereferenceObject(currentDevObj);
		currentDevObj = nextDevObj;
	} while (NULL != currentDevObj);

	return FALSE;
}


NTSTATUS FsFilterAttachToDevice(
	_In_ PDEVICE_OBJECT         DeviceObject,
	__out_opt PDEVICE_OBJECT*   pFilterDeviceObject
)
{
/*
	This will attach to a DeviceObject that represents a mounted volume
*/

	NTSTATUS                    status = STATUS_SUCCESS;
	PDEVICE_OBJECT              filterDeviceObject = NULL;
	PFSFILTER_DEVICE_EXTENSION  pDevExt = NULL;
	ULONG                       i = 0;

	ASSERT(!FsFilterIsAttachedToDevice(DeviceObject));

	//
	//  Create a new device object we can attach with.
	//

	status = IoCreateDevice(g_DriverObject, sizeof(FSFILTER_DEVICE_EXTENSION), NULL, DeviceObject->DeviceType, 0, FALSE, &filterDeviceObject);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	pDevExt = (PFSFILTER_DEVICE_EXTENSION)filterDeviceObject->DeviceExtension;

	//
	//  Propagate flags from Device Object we are trying to attach to.
	//

	if (FlagOn(DeviceObject->Flags, DO_BUFFERED_IO))
	{
		SetFlag(filterDeviceObject->Flags, DO_BUFFERED_IO);
	}

	if (FlagOn(DeviceObject->Flags, DO_DIRECT_IO))
	{
		SetFlag(filterDeviceObject->Flags, DO_DIRECT_IO);
	}

	if (FlagOn(DeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN))
	{
		SetFlag(filterDeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN);
	}

	//
	//  Do the attachment.
	//
	//  It is possible for this attachment request to fail because this device
	//  object has not finished initializing.  This can occur if this filter
	//  loaded just as this volume was being mounted.
	//

	for (i = 0; i < 8; ++i)
	{
		LARGE_INTEGER interval;

		status = IoAttachDeviceToDeviceStackSafe(filterDeviceObject, DeviceObject, &pDevExt->AttachedToDeviceObject);
		if (NT_SUCCESS(status))
			break;

		//
		//  Delay, giving the device object a chance to finish its
		//  initialization so we can try again.
		//

		interval.QuadPart = (500 * DELAY_ONE_MILLISECOND);
		KeDelayExecutionThread(KernelMode, FALSE, &interval);
	}

	if (!NT_SUCCESS(status))
	{
		//
		// Clean up.
		//

		IoDeleteDevice(filterDeviceObject);
		filterDeviceObject = NULL;
	}
	else
	{
		//
		// Mark we are done initializing.
		//

		ClearFlag(filterDeviceObject->Flags, DO_DEVICE_INITIALIZING);

		if (NULL != pFilterDeviceObject)
		{
			*pFilterDeviceObject = filterDeviceObject;
		}
	}

	return status;
}


VOID FsFilterDetachFromDevice(
	_In_ PDEVICE_OBJECT DeviceObject
)
{
	PFSFILTER_DEVICE_EXTENSION pDevExt = (PFSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

	IoDetachDevice(pDevExt->AttachedToDeviceObject);
	IoDeleteDevice(DeviceObject);
}



BOOLEAN FsFilterIsMyDeviceObject(
	_In_ PDEVICE_OBJECT DeviceObject
)
{
	return DeviceObject->DriverObject == g_DriverObject;
}
