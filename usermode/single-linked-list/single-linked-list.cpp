// single-linked-list.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <malloc.h>
#include <stdio.h>

// Structure to be used for a list item; the first member is the 
// SLIST_ENTRY structure, and additional members are used for data.
// Here, the data is simply a signature for testing purposes. 


typedef struct _PROGRAM_ITEM {
	SLIST_ENTRY ItemEntry;
	ULONG Signature;
} PROGRAM_ITEM, *PPROGRAM_ITEM;

int main()
{
	ULONG Count;
	PSLIST_ENTRY pFirstEntry, pListEntry;
	PSLIST_HEADER pListHead;
	PPROGRAM_ITEM pProgramItem;

	// Initialize the list header to a MEMORY_ALLOCATION_ALIGNMENT boundary.
	pListHead = (PSLIST_HEADER)_aligned_malloc(sizeof(SLIST_HEADER),
		MEMORY_ALLOCATION_ALIGNMENT);
	if (NULL == pListHead)
	{
		printf("Memory allocation failed.\n");
		return -1;
	}
	InitializeSListHead(pListHead);

	// Insert 10 items into the list.
	for (Count = 1; Count <= 10; Count += 1)
	{
		pProgramItem = (PPROGRAM_ITEM)_aligned_malloc(sizeof(PROGRAM_ITEM),
			MEMORY_ALLOCATION_ALIGNMENT);
		if (NULL == pProgramItem)
		{
			printf("Memory allocation failed.\n");
			return -1;
		}
		pProgramItem->Signature = Count;
		pFirstEntry = InterlockedPushEntrySList(pListHead,
			&(pProgramItem->ItemEntry));
	}

	// Walk though the list and print the content.
	pFirstEntry = &(pListHead->Next);
	while (pFirstEntry->Next != NULL) {
		pFirstEntry = pFirstEntry->Next;
		pProgramItem = CONTAINING_RECORD(pFirstEntry, PROGRAM_ITEM, ItemEntry);
		printf("Signature is %d\n", pProgramItem->Signature);
	} 

	// Remove 10 items from the list and display the signature.
	for (Count = 10; Count >= 1; Count -= 1)
	{
		pListEntry = InterlockedPopEntrySList(pListHead);

		if (NULL == pListEntry)
		{
			printf("List is empty.\n");
			return -1;
		}

		pProgramItem = (PPROGRAM_ITEM)pListEntry;
		printf("Signature is %d\n", pProgramItem->Signature);

		// This example assumes that the SLIST_ENTRY structure is the 
		// first member of the structure. If your structure does not 
		// follow this convention, you must compute the starting address 
		// of the structure before calling the free function.

		_aligned_free(pListEntry);
	}

	// Flush the list and verify that the items are gone.
	pListEntry = InterlockedFlushSList(pListHead);
	pFirstEntry = InterlockedPopEntrySList(pListHead);
	if (pFirstEntry != NULL)
	{
		printf("Error: List is not empty.\n");
		return -1;
	}

	_aligned_free(pListHead);

	return 1;
}