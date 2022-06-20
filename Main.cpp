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

DWORD WINAPI marker(LPVOID lpParam) {}

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

    DeleteCriticalSection(&cs);
    delete[] workArray;

    return 0;
}