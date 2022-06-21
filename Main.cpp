#include <iostream>
#include <windows.h>
#include <vector>

CRITICAL_SECTION cs;

int *workArray;
int workArrayLength;

struct MetaInfo {
    int index;
    HANDLE isPaused;
    HANDLE isToBeTerminated;
};
std::vector<MetaInfo *> threadInfo;
std::vector<HANDLE> runningThreads;
int threadCount;

HANDLE startEvent;

DWORD WINAPI marker(LPVOID lpParam) {
    WaitForSingleObject(startEvent, INFINITE);
    MetaInfo *temp;
    temp = (MetaInfo *) lpParam;
    int threadIndex;
    threadIndex = temp->index;
    srand(threadIndex + 1);
    int markedCount = 0;
    while (true) {
        EnterCriticalSection(&cs);
        int index = rand() % workArrayLength;
        if (workArray[index] == 0) {
            Sleep(5);
            workArray[index] = threadIndex + 1;
            ++markedCount;
            Sleep(5);
            LeaveCriticalSection(&cs);
        } else {
            std::cout << "поток с порядковым номером № " << threadIndex + 1 << "пометил " << markedCount
                      << "элементов, но не смог пометить элемент с индексом" << index << "\n";
            SetEvent(temp->isPaused);
            WaitForMultipleObjects(threadCount, &threadInfo[0]->isToBeTerminated, FALSE, INFINITE);
            if (WaitForSingleObject(temp->isToBeTerminated, 0) == WAIT_OBJECT_0) {
                int i = 0;
                while (i < workArrayLength) {
                    if (workArray[i] == threadIndex + 1)
                        workArray[i] = 0;
                    ++i;
                }
                runningThreads.erase(runningThreads.begin() + threadIndex - 1);
                break;
            } else ResetEvent(temp->isPaused);
            LeaveCriticalSection(&cs);
        }
    }
    return 0;
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

    InitializeCriticalSection(&cs);

    startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    MetaInfo *tempInfo;
    HANDLE tempHandle;

    i = 0;
    while (i < threadCount) {
        tempInfo = new MetaInfo();
        tempInfo->index = i;
        tempInfo->isPaused = CreateEvent(NULL, TRUE, FALSE, NULL);
        tempInfo->isToBeTerminated = CreateEvent(NULL, TRUE, FALSE, NULL);
        threadInfo.push_back(tempInfo);
        tempHandle = CreateThread(NULL, 0, marker, (LPVOID) (tempInfo), 0, NULL);
        runningThreads.push_back(tempHandle);
        ++i;
    }
    std::cout << threadCount << " потока запущены и готовы к работе.\n";


    SetEvent(startEvent);
    std::cout << "Сигнал к началу работы подан.\n";

    while (threadCount != 0) {
        std::cout << "Поток main ожидает окончания работы " << threadCount << " потока(-ов).\n";
        WaitForMultipleObjects(threadCount, &threadInfo[0]->isPaused, TRUE, INFINITE);

        std::cout << "После окончания работы " << threadCount << " потока(-ов) marker массив имеет вид:\n";
        i = 0;
        while (i < workArrayLength - 1) {
            std::cout << workArray[i] << " ";
            ++i;
        }
        std::cout << workArray[i] << "\n";

        std::cout << "Введите порядковый номер потока, который завершит свою работу";
        long long terminationIndex;
        std::cin >> terminationIndex;

        SetEvent(threadInfo[--terminationIndex]->isToBeTerminated);
        std::cout << "Сигнал к завершению работы потока № " << terminationIndex + 1
                  << " и продолжению работы всех остальных потоков был подан.\n";

        WaitForSingleObject(runningThreads[terminationIndex], INFINITE);
        std::cout << "После завершения работы потока номер " << terminationIndex + 1 << "массив имеет вид: ";
        i = 0;
        while (i < workArrayLength - 1) {
            std::cout << workArray[i] << " ";
            ++i;
        }
        std::cout << workArray[i] << "\n";
    }

    DeleteCriticalSection(&cs);
    delete[] workArray;

    return 0;
}