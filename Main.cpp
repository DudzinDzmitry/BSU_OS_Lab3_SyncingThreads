#include <iostream>
#include <windows.h>
#include <vector>

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

    return 0;
}