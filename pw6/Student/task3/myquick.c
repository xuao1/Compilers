#include "runtime/io.h"
float arr[4096];

int swap(int i, int j) {
    float temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
    return 0;
}

int partition(int left, int right) {
    int pivot = left;
    int index = pivot + 1;
    int i = index;
    while(i <= right){
        if (arr[i] < arr[pivot]) {
            swap(i, index);
            index++;
        }
        i++;
    }
    swap(pivot, index - 1);
    return index - 1;
}

int quickSort(int left, int right) {
    if (left < right) {
        int partitionIndex = partition(left, right);
        quickSort(left, partitionIndex - 1);
        quickSort(partitionIndex + 1, right);
    }
    return 0;
}
 
int main()
{
    getfarray(arr);
    quickSort(0, n-1); 
    putfarray(n, arr);
    return 0;
}