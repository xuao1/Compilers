#include <stdio.h>
#include <stdlib.h>

void mysort(int length,int a[])
{
    for (int i = 1; i <= length; i++)
        for (int j = 1; j < length; j++) {
            if (a[j] > a[j + 1]) {
                int tmp = a[j + 1];
                a[j + 1] = a[j];
                a[j] = tmp;
            }
        }
}

int main()
{
	int n;
	int *num;
    scanf("%d", &n);
    num = (int*)malloc(sizeof(int)*n);
    for (int i = 1; i <= n; i++) {
        scanf("%d", &num[i]);
    }
    mysort(n, num);
    for (int i = 1; i <= n; i++) {
        printf("%d ", num[i]);
    }
    return 0;
}
