// test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define BUFFSIZE 50000

int main(int argc, char* argv[])
{
	/* ------File------ */
	FILE* ptrA = NULL;
	FILE* ptrB = NULL;
	/* ------String------ */
	char strA[BUFFSIZE];
	char strB[BUFFSIZE];
	/* ------String Part------ */
	char* tokenA;
	char* tokenB;
	/* ------File------ */
	float** dataA;
	float** dataB;
	const char s[3] = " \n";
	/* ------Row/Col------ */
	int rA = 0;
	int cA = 0;
	int rB = 0;
	int cB = 0;
	/* ------Block------ */
	int n = 0;
	int nP = 0; /* Number of processes */
	int id; /* Process rank*/

	/* ---------increment variable-------- */
	int i = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &nP);
	
	if (id == 0)
	{
		ptrA = fopen("matAsmall.txt", "r");

		fgets(strA, BUFFSIZE, ptrA);
		sscanf(strA, "%d %d", &cA, &rA);

		fgets(strB, BUFFSIZE, ptrB);
		sscanf(strB, "%d %d", &cB, &rB);

		n = ceil( (float) rA / (float) nP);

		printf("%d %d|%d and %d|%d\n", n, cA, rA, cB, rB);

		
		dataA = (float**)malloc(rA * sizeof(float*));
		for (i = 0; i < rA; i++)
			dataA[i] = (float*)malloc(cA * sizeof(float));

		dataB = (float**)malloc(rA * sizeof(float*));
		for (i = 0; i < rB; i++)
			dataB[i] = (float*)malloc(cA * sizeof(float));

		while (fgets(strA, BUFFSIZE, ptrA) && fgets(strB, BUFFSIZE, ptrB))
		{
			
		}
	}
	else if (id == 1)
	{
		ptrB = fopen("matBsmall.txt", "r");

		fgets(strA, BUFFSIZE, ptrA);
		sscanf(strA, "%d %d", &cA, &rA);

		fgets(strB, BUFFSIZE, ptrB);
		sscanf(strB, "%d %d", &cB, &rB);

		n = ceil((float)rA / (float)nP);

		printf("%d %d|%d and %d|%d\n", n, cA, rA, cB, rB);


		dataA = (float**)malloc(rA * sizeof(float*));
		for (i = 0; i < rA; i++)
			dataA[i] = (float*)malloc(cA * sizeof(float));

		dataB = (float**)malloc(rA * sizeof(float*));
		for (i = 0; i < rB; i++)
			dataB[i] = (float*)malloc(cA * sizeof(float));

		while (fgets(strA, BUFFSIZE, ptrA) && fgets(strB, BUFFSIZE, ptrB))
		{

		}
	}
	else
	{

	}

	MPI_Finalize();
	/* -------------------End Naa Jaa------------------ */
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

