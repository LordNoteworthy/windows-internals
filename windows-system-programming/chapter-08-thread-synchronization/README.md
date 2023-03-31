# Chapter 08: Thread Synchronization

- The big problem:
    - We need to synchronize thread execution in order to ensure that only one thread at a time executes any of the critical regions for a data item.
- Defective workaround:
    - Give each thread its own copy of the variable.
    - But: ‼️ An optimizing compiler might leave the value of `N` in a register rather than storing it back in `N`.
        - ▶️ Use the ANSI C `volatile` storage qualifier, which ensures that the variable will be stored in memory after modification and will always be fetched from memory before use.
        - Even the `volatile` modifier does not assure that changes are visible to other processors in a specific order, because a processor might hold the value in cache before committing it to main memory and alter the order in which different processes see the changed values.
        - To assure that changes are visible to other processors in the desired order, use memory barriers (or “fences”); the interlocked functions provide a memory barrier, as do all the synchronization functions in this chapter
- Summary:
    - Variables that are local to the thread should not be global and should be on the thread’s stack or in a data structure or TLS that only the individual thread can access directly.
    - Threads should not, in general, change the process environment because that would affect all threads.
    - Variables shared by all threads should be `static` or in global storage and protected with the synchronization or interlocked mechanisms that create a memory barrier.

## Critical Sections

- Threads enter and leave a CS, and only one thread at a time can be in a specific CS.
- `CRITICAL_SECTION` (CS) objects are initialized and deleted but do not have **handles** and are not **shared** with other **processes**.
- Always be certain to leave a CS; failure to do so will cause other threads to wait forever, even if the **owning thread terminates**.
- CS are **recursive**.
- CS have the advantage of **not being kernel objects** and are maintained in user space. This almost always provides performance improvements compared to using a Windows `mutex` kernel object with similar functionality.
- Waiting for a CS to be released is *time consuming*. On MP systems, you can require that the thread try again (that is, **spin**) before blocking, as the owning thread may be running on a separate processor and could release the CS at any time.
- Any variable that is accessed and modified with **interlocked** instructions is **volatile**.
- Termination handlers ensure that the CS is released. This technique helps to ensure that later code modifications do not inadvertently skip the `LeaveCriticalSection` call. It is important, however, that the `__try` statement is
immediately after the `EnterCriticalSection` so that there is no possibility of an exception or other transfer of control between the call to `EnterCriticalSection` and the `__try` block.

## Mutexes

- A mutex (“mutual exclusion”) object provides locking functionality beyond that of CSs.
- Because mutexes can be named and have handles, they can also be used for **IPC synchronization** between threads in separate processes.
- Mutex objects are similar to CSs, but in addition to being **process-sharable**, mutexes allow **time-out** values and become **signaled** when abandoned by a **terminating** thread.
- A thread can acquire a specific mutex **several times**; the thread will not block if it already has ownership. Ultimately, it must release the mutex the **same number of times**.
- CSs are almost always considerably **faster** than mutexes.

### Mutexes, CSs, and Deadlocks

- Mutexes and CSs must be used carefully to avoid **deadlocks**, in which two threads become blocked while each is waiting for a resource owned by the other thread.
- 💁 Each thread owns a mutex the other requires, and neither thread can proceed to the `ReleaseMutex` call that would unblock the other thread.
- 🖐️ One way to avoid deadlock is the “**try and back off**” strategy, whereby a thread calls `WaitForSingleObject` with a finite time-out value and, when detecting an owned mutex, “**backs off**” by yielding the processor or sleeping for a brief time before trying again.
- 👍 A far simpler and superior method, is to specify a “**mutex hierarchy**” such that all threads are programmed to assure that they **acquire** the mutexes in exactly the **same order** and **release** them in the **opposite** order.
- 👍 Another technique to reduce deadlock potential is to put the **two mutex handles** in an **array** and use `WaitForMultipleObjects` with the flag set to `TRUE` so that a thread acquires either both or neither of the mutexes in an atomic operation

### Heap Locking

- A pair of functions (`HeapLock` and `HeapUnlock`) is available to synchronize heap access.
- ▶️ No other thread can allocate or free memory (or walk it using `HeapWalk`) from the heap while a thread owns the heap lock.

## Semaphores

- Semaphores maintain a **count**, and the semaphore object is in the signaled state when the count is greater than 0.
- The semaphore is **unsignaled** when the count is 0.
- When a waiting thread is released, the semaphore’s count is **decremented** by 1.
- A semaphore release (`ReleaseSemaphore`) can **increment** its count by any value up to the maximum.
- While it is tempting to think of a mutex as a special case of a semaphore with a maximum value of 1, this would be misleading because there is **no semaphore ownership**. Any thread can release a semaphore, not just the one that performed the wait. Likewise, since there is **no ownership**, there is no concept of an abandoned semaphore.
- 💁 The classic semaphore application regards the semaphore count as representing the number of available resources, such as the **number of messages waiting in a queue**. The semaphore maximum then represents the maximum queue size.
    - Thus, a **producer** would place a message in the buffer and call `ReleaseSemaphore`, usually
    with a release count of 1.
    - Consumer threads would wait on the semaphore, consuming a message and decrementing the semaphore count.
- ▶️ While semaphores can be **convenient**, they are **redundant** in the sense that mutexes and events used together, are more
powerful than semaphores.
- There is still an important limitation with the Windows semaphore implementation.
    - ❓ How can a thread request that the count be decremented by two or more?
    - The thread can wait twice in succession but this would not be an atomic operation because the thread could be preempted between waits. But this could cause a *deadlock* ! We can use a CS to solve this issue.