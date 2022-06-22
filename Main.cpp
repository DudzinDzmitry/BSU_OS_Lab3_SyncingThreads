#include <iostream>
#include <windows.h>

CRITICAL_SECTION cs;

int *workArray;
int workArrayLength;

HANDLE *threads;
int threadCount;

HANDLE *waiting;
HANDLE *terminated;

HANDLE run;
HANDLE answer;

DWORD WINAPI marker(LPVOID lpParam) {
    int threadIndex = (long long) lpParam;
    srand(threadIndex + 1);

    int markedCount = 0;
    while (WaitForSingleObject(terminated[threadIndex], 0) != WAIT_OBJECT_0) {
        WaitForSingleObject(run, INFINITE);

        int index = rand() % workArrayLength;

        EnterCriticalSection(&cs);
        if (workArray[index] == 0) {
            Sleep(5);
            workArray[index] = threadIndex + 1;
            ++markedCount;
            Sleep(5);
            LeaveCriticalSection(&cs);
        } else {
            LeaveCriticalSection(&cs);
            std::cout << "поток с порядковым номером № " << threadIndex + 1 << " пометил " << markedCount
                      << " элементов, но не смог пометить элемент с индексом " << index << "\n";

            SetEvent(waiting[threadIndex]);
            WaitForSingleObject(answer, INFINITE);

            ResetEvent(waiting[threadIndex]);
        }
    }
    int i = 0;
    while (i < workArrayLength) {
        if (workArray[i] == threadIndex + 1) workArray[i] = 0;
        ++i;
    }
    SetEvent(waiting[threadIndex]);
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    std::cout << "Данная программа запускает несколько потоков, которые помечают элементы массива.\n";

    std::cout << "Введите количество элементов в массиве:";
    std::cin >> workArrayLength;

    workArray = new int[workArrayLength];
    int i = 0;
    while (i < workArrayLength) {
        workArray[i] = 0;
        ++i;
    }

    std::cout << "Введите количество экземпляров потока marker, которые будут запущены:";
    std::cin >> threadCount;

    run = CreateEvent(NULL, TRUE, FALSE, NULL);

    threads = new HANDLE[threadCount];
    waiting = new HANDLE[threadCount];
    terminated = new HANDLE[threadCount];

    i = 0;
    while (i < threadCount) {
        threads[i] = CreateThread(NULL, 0, marker, (LPVOID) i, 0, NULL);
        waiting[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        terminated[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        ++i;
    }
    std::cout << threadCount << " потока запущены и готовы к работе.\n";

    InitializeCriticalSection(&cs);

    std::cout << "Сигнал к началу работы подан.\n";
    SetEvent(run);

    answer = CreateEvent(NULL, TRUE, FALSE, NULL);

    int runningThreadCount = threadCount;
    while (runningThreadCount != 0) {
        WaitForMultipleObjects(threadCount, waiting, TRUE, INFINITE);
        ResetEvent(run);
        std::cout << "После окончания работы " << runningThreadCount << " потока(-ов) marker массив имеет вид:\n";
        i = 0;
        while (i < workArrayLength - 1) {
            std::cout << workArray[i] << " ";
            ++i;
        }
        std::cout << workArray[i] << "\n";

        std::cout << "Введите порядковый номер потока, который завершит свою работу:";
        int terminationIndex;
        std::cin >> terminationIndex;

        std::cout << "Сигнал к завершению работы потока № " << terminationIndex << " был подан.\n";
        SetEvent(terminated[--terminationIndex]);
        SetEvent(answer);

        WaitForSingleObject(threads[terminationIndex], INFINITE);
        --runningThreadCount;

        std::cout << "После завершения работы потока номер " << terminationIndex + 1 << "массив имеет вид: ";
        i = 0;
        while (i < workArrayLength - 1) {
            std::cout << workArray[i] << " ";
            ++i;
        }
        std::cout << workArray[i] << "\n";
        std::cout << "Сигнал к возобновлению работы " << runningThreadCount << " потоков был подан.\n";
        ResetEvent(answer);
        SetEvent(run);
    }

    DeleteCriticalSection(&cs);
    delete[] workArray;
    delete[] threads;
    delete[] waiting;
    delete[] terminated;

    return 0;
}