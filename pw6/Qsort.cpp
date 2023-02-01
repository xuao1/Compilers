#include<iostream>
#include<cstdio>
#include<cstdlib>

using namespace std;

int n;
int arr[1000];

int swap(int i, int j) {
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

int partition(int left, int right) {
    int pivot = left;
    int index = pivot + 1;
    int i = index;
    while(i <= right){
    	for(int i=0;i<n;i++){
    		printf("%d ",arr[i]);
		}
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
}

int Read()
{
    //n = getint();
    //getfarray(arr);
    scanf("%d",&n);
    for(int i=0;i<n;i++){
    	scanf("%d",&arr[i]);
	}
}
 
int main()
{
    Read();
    //quickSort(0, n-1);
	printf("%d",partition(0,n-1)); 
    //putfarray(n, arr);
    for(int i=0;i<n;i++){
    	printf("%d ",arr[i]);
	}
    return 0;
}