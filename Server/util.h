#pragma once
#include <stdio.h>
#include "iostream"
#include <stdint.h>

using namespace std;

/*
change int to number base 256
@Param Input
length : number
buff[] : char array storage length
@Param Output : no return value
*/
void add_length(int length, char* buff) {
	*(buff + 1) = length / 256;
	*(buff + 2) = length % 256;
}

/*	Change int to char[]
input : number
output : char array
*/
char* int_to_char(long long number) {
	if (number == 0) {
		char* arr = (char*)malloc(2);
		arr[0] = '0';
		arr[1] = 0;
		return arr;
	}
	int digit = (int)log10(number) + 2;
	char* arr = (char*)malloc(digit);
	char arr1[10];
	int index = 0;
	while (number) {
		arr1[index++] = number % 10 + '0';
		number /= 10;
	}
	int i;
	for (i = 0; i < index; i++) {
		arr[i] = arr1[index - 1 - i];
	}
	arr[i] = '\0';
	return arr;
}