#include "../../include/algorithm/sort.h"



void exchangeValue(int&a, int&b) {
	int tmp = a;
	a = b;
	b = tmp;
}

void exchange(int* arr, int a, int b) {
	int tmp = arr[a];
	arr[a] = arr[b];
	arr[b] = tmp;
}

/*
√∞≈›≈≈–Ú
*/
void bubbleSort(int* arr, int length) {

	for (int i = 0; i < length; i++) {
		for (int j = length - 1; j > i; j--) {
			if (arr[j] < arr[j - 1]) {
				exchangeValue(arr[j], arr[j - 1]);
			}
		}
	}

}

/*
≤Â»Î≈≈–Ú
*/
void insertSort(int* arr, int length) {

	int j;

	for (int i = 1; i < length; i++) {

		int tmp = arr[i];

		for (j = i; j > 0 && tmp < arr[j - 1]; j--) {
			arr[j] = arr[j - 1];
		}

		arr[j] = tmp;
	}

}

/*
øÏÀŸ≈≈–Ú
*/
void quickSort(int* arr, int left, int right) {

	if (left < right) {

		int i = left, j = right, x = arr[left];
		while (i < j) {

			while (i < j && arr[j] >= x) j--;
			if (i < j) {
				arr[i++] = arr[j];
			}

			while (i < j && arr[i] < x) i++;
			if (i < j) {
				arr[j--] = arr[i];
			}
		}

		arr[i] = x;
		quickSort(arr, left, i - 1);
		quickSort(arr, i + 1, right);

	}
}