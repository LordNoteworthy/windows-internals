#pragma once

#include <ntifs.h>
#include "FastIo.h"
#include "AttachDetach.h"

// Globals
PDRIVER_OBJECT g_DriverObject;

// Structures
typedef struct _FSFILTER_DEVICE_EXTENSION
{
	PDEVICE_OBJECT AttachedToDeviceObject;
} FSFILTER_DEVICE_EXTENSION, *PFSFILTER_DEVICE_EXTENSION;


// Defines
#define DEVOBJ_LIST_SIZE        64


//  Macro to test if FAST_IO_DISPATCH handling routine is valid
#define VALID_FAST_IO_DISPATCH_HANDLER(_FastIoDispatchPtr, _FieldName) \
    (((_FastIoDispatchPtr) != NULL) && \
    (((_FastIoDispatchPtr)->SizeOfFastIoDispatch) >= \
    (FIELD_OFFSET(FAST_IO_DISPATCH, _FieldName) + sizeof(void *))) && \
    ((_FastIoDispatchPtr)->_FieldName != NULL))



// Prototypes
NTSTATUS DispatchPassThrough(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP Irp
);

NTSTATUS DispatchCreate(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP           Irp
);

VOID Unload(
	_In_ PDRIVER_OBJECT DriverObject
);