#pragma once

#include <ntddk.h>
#include <ntstrsafe.h>


// Structs
typedef struct {
	PDEVICE_OBJECT LowerKeyboardDevice;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


typedef struct _KEYBOARD_INPUT_DATA {
	USHORT UnitId;
	USHORT MakeCode;
	USHORT Flags;
	USHORT Reserved;
	ULONG  ExtraInformation;
} KEYBOARD_INPUT_DATA, *PKEYBOARD_INPUT_DATA;