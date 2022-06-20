#include <iostream>
#include <windows.h>
#include <vector>

CRITICAL_SECTION cs;

long long *workArray;
long long workArrayLength;

struct MetaThread {
    long long index;
    HANDLE Thread;
    HANDLE isPaused;
    HANDLE isToBeTerminated;
};
std::vector<MetaThread> runningThreads;
long long threadCount;

HANDLE startEvent;

DWORD WINAPI marker(LPVOID lpParam) {
    WaitForSingleObject(startEvent, INFINITE);
    long long threadIndex;
    threadIndex = (long long) lpParam;
    srand(threadIndex + 1);
    long long markedCount = 0;
    while (true) {
        EnterCriticalSection(&cs);
        long long index = rand() % workArrayLength;
        if (workArray[index] == 0) {
            Sleep(5);
            workArray[index] = threadIndex + 1;
            ++markedCount;
            Sleep(5);
            LeaveCriticalSection(&cs);
        } else {
            LeaveCriticalSection(&cs);
            std::cout << "поток с порядковым номером № " << threadIndex + 1 << "пометил " << markedCount
                      << "элементов, но не смог пометить элемент с индексом" << index << "\n";
            SetEvent(runningThreads[threadIndex].isPaused);
            long long terminationIndex = runningThreads[
                    WaitForMultipleObjects(threadCount, &runningThreads[0].isToBeTerminated, FALSE, INFINITE) -
                    WAIT_OBJECT_0].index;
            if (terminationIndex == threadIndex) {
                int i = 0;
                while (i < workArrayLength) {
                    if (workArray[i] == threadIndex + 1)
                        workArray[i] = 0;
                    ++i;
                }
                runningThreads.erase(runningThreads.begin() + threadIndex - 1);
                break;
            } else ResetEvent(runningThreads[threadIndex].isPaused);
        }
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    std::cout << "Данная программа запускает несколько потоков, которые помечают элементы массива.\n";

    std::cout << "Введите количество элементов в массиве:";
    std::cin >> workArrayLength;

    workArray = new long long[workArrayLength];
    long long i = 0;
    while (i < workArrayLength) {
        workArray[i] = 0;
        ++i;
    }

    std::cout << "Введите количество экземпляров потока marker, которые будут запущены:";
    std::cin >> threadCount;

    InitializeCriticalSection(&cs);

    i = 0;
    while (i < threadCount) {
        runningThreads[i].index = i;
        runningThreads[i].Thread = CreateThread(NULL, 0, marker, (LPVOID) (i), NULL, NULL);
        runningThreads[i].isPaused = CreateEvent(NULL, TRUE, FALSE, NULL);
        runningThreads[i].isToBeTerminated = CreateEvent(NULL, TRUE, FALSE, NULL);
        ++i;
    }

    SetEvent(startEvent);

    while (threadCount != 0) {
        WaitForMultipleObjects(threadCount, &runningThreads[0].isPaused, TRUE, INFINITE);
        std::cout << "Массив имеет вид: ";
        i = 0;
        while (i < workArrayLength) {
            std::cout << workArray[i] << " ";
            ++i;
        }
        std::cout << "\n";
        std::cout << "Введите порядковый номер потока, который завершит свою работу";
        long long terminationIndex;
        std::cin >> terminationIndex;
        SetEvent(runningThreads[--terminationIndex].isToBeTerminated);
        WaitForSingleObject(runningThreads[terminationIndex].Thread, INFINITE);
        std::cout << "После завершения работы потока номер " << threadCount + 1 << "массив имеет вид: ";
        i = 0;
        while (i < workArrayLength) {
            std::cout << workArray[i] << " ";
            ++i;
        }
        std::cout << "\n";
    }

    DeleteCriticalSection(&cs);
    delete[] workArray;

    return 0;
}