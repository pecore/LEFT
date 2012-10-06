#ifndef _PTHREAD_WINAPI_H_
#define _PTHREAD_WINAPI_H_


#ifdef _WIN32
#define Lock(mutex) WaitForSingleObject(mutex, 0xFFFFFFFF)
#define Unlock(mutex) ReleaseMutex(mutex)
#define CloseMutex(mutex) CloseHandle(mutex)
#define CloseEvent(event) CloseHandle(event)
#define WaitForEvent(event) WaitForSingleObject(event, INFINITE)
#define WaitForThread(thread) WaitForSingleObject(thread, INFINITE)
#else
#include <pthread.h>
#include <stdlib.h>

#define FALSE 0
#define TRUE 1

#define VK_SPACE 0x20
#define VK_SHIFT 1
#define MK_LBUTTON 0
#define MK_RBUTTON 3

#define WINAPI *
#define INVALID_HANDLE_VALUE 0
#define HANDLE void*
#define DWORD unsigned int
#define LONGLONG unsigned long long
typedef union _LARGE_INTEGER {
  struct {
    DWORD LowPart;
    DWORD HighPart;
  };
  struct {
    DWORD LowPart;
    DWORD HighPart;
  } u;
  LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

#define Sleep(x) sleep(0);
#define GetTickCount()  time(NULL)
//#define Lock(mutex) printf("Lock %s:%d\t%p\n", __FILE__, __LINE__, mutex); pthread_mutex_lock((pthread_mutex_t *) mutex)
//#define Unlock(mutex) printf("Unlock %s:%d\t%p\n", __FILE__, __LINE__, mutex); pthread_mutex_unlock((pthread_mutex_t *) mutex)
#define Lock(mutex) pthread_mutex_lock((pthread_mutex_t *) mutex)
#define Unlock(mutex) pthread_mutex_unlock((pthread_mutex_t *) mutex)

typedef struct {
  pthread_cond_t cond;
  pthread_mutex_t mutex;
} pthread_event_t;

void * CreateMutex(void * lpMutexAttributes, int bInitialOwner, const char * pName);
void CloseMutex(void * mutex);
void * CreateEvent(void * lpEventAttributes, int bManualReset, int bInitialState, const char * pName);
void CloseEvent(void * vevent);
void PulseEvent(void * vevent);
void WaitForEvent(void * vevent);
void QueryPerformanceCounter(LARGE_INTEGER * pCounter);
void QueryPerformanceFrequency(LARGE_INTEGER * pFreq);
void * CreateThread(int lpThreadAttributes, int dwStackSize, unsigned int *(*lpStartAddress)(void*), void * lpParameter, int dwCreationFlags, int lpThreadId);
void WaitForThread(void * thread);
#endif

#endif
