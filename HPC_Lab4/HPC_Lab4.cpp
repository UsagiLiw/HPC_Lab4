// HPC_Lab4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_DEPRECATE


#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


float** alloc_2d(int rows, int cols) {
	float* data = (float*)malloc(rows * cols * sizeof(float));
	float** array = (float**)malloc(rows * sizeof(float*));
	for (int i = 0; i < rows; i++)
		array[i] = &(data[cols * i]);

	return array;
}

int main(int argc, char** argv)
{
	// start MPI
	MPI_Init(NULL, NULL);

	// get size of mpi
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	// get rank of mpi
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	// get processer name
/*har processor_name[MPI_MAX_PROCESSOR_NAME];
	int name_len;
	MPI_Get_processor_name(processor_name, &name_len);*/

	int i = 0;
	int j = 0;
	int k = 0;
	int tag = 1;
	long matdim1;				// row dimension of file A
	long matdim2;				// column dimension of file A
	long matdim3;				// row dimension of file B
	long matdim4;				// column dimension of file B
	double StartTime;			// Start Timer
	double EndTime;				// End timer
	// allocate count to prevent race condition
	int* count = (int*)malloc(sizeof(int*) * world_size);

	char* token = NULL;
	FILE* fp1 = NULL;
	FILE* fp2 = NULL;
	FILE* fp3 = NULL;
	MPI_Status status;
	char str[60000];
	char filename1[256] = "matAlarge.txt";
	char filename2[256] = "matBlarge.txt";
	char filename3[256] = "testOut.txt";


	// open files
	fp1 = fopen(filename1, "r");
	fp2 = fopen(filename2, "r");
	if (world_rank == 0)
		fp3 = fopen(filename3, "w+");

	// check for accessing file 1 and 2.
	if (fp1 == NULL)
	{
		printf("\nCould not open file %s\n", filename1);
		return 1;
	}
	if (fp2 == NULL)
	{
		printf("\nCould not open file %s\n", filename2);
		return 1;
	}
	if (fp3 == NULL && world_rank == 0)
	{
		printf("\nCould not open file %s\n", filename3);
		return 1;
	}

	// Check first row of each file and check for dimension.
	// if cannot found first row ; return value 1
	fgets(str, 2000, fp1);		// read file A
	if (str == NULL)
	{
		printf("\nNo data found in %s\n", filename1);
		return 1;
	}
	else
	{
		char* counttok = strtok(str, " ");
		matdim1 = atoi(counttok);
		counttok = strtok(NULL, " ");
		matdim2 = atoi(counttok);
	}
	fgets(str, 2000, fp2);		// read file B
	if (str == NULL)
	{
		printf("\nNo data found in %s\n", filename2);
		return 1;
	}
	else
	{
		char* counttok = strtok(str, " ");
		matdim3 = atoi(counttok);
		counttok = strtok(NULL, " ");
		matdim4 = atoi(counttok);
	}

	// check for both matrix have same dimention
	// if not ; return value 2
	if (matdim1 != matdim3 || matdim2 != matdim4)
	{
		printf("\nERROR occur!! the dimension of matrix are not same.\n");
		return 2;
	}

	// calculate for the threshold 
	int threshold = matdim1 / world_size;
	printf("\n\n\nThe threshold is %d.\n", threshold);


	// allocate matrix to read file using alloc_2d function
	float** matrix = alloc_2d(matdim1, matdim2);
	float** matrix2 = alloc_2d(matdim1, matdim2);
	float** ansmat = alloc_2d(matdim1, matdim2);

	/* this is the old way to allocate matrix
	float** matrix = (float**)malloc(sizeof(float*) * matdim1);
	for (int x = 0; x < matdim1; x++)
	{
		matrix[x] = (float*)malloc(matdim2 * sizeof(matrix[0]);
	}
	float** matrix2 = (float**)malloc(sizeof(float*) * matdim1);
	for (int x = 0; x < matdim1; x++)
	{
		matrix2[x] = (float*)malloc(matdim2 * sizeof(matrix2[0]));
	}
	float** ansmat = (float**)malloc(sizeof(float*) * matdim1);
	for (int x = 0; x < matdim1; x++)
	{
		ansmat[x] = (float*)malloc(matdim2 * sizeof(ansmat[0]));
	}
	*/


	int worked = threshold * world_size;
	int julong = matdim1 - worked;


	// Start parallel
	if (world_rank == 0)
	{
		printf("I am rank 0\n");

		// Check if it stand alone.
		if (world_size == 1)
		{
			// read file 1
			printf("Start reading file1...\n");
			count[world_rank] = 0;
			while (fgets(str, 60000, fp1))
			{
				char* token = strtok(str, " ");
				i = 0;
				//printf("\nBefore loop row%d\n", count);
				while (token != NULL)
				{
					matrix[count[world_rank]][i] = atof(token);
					//printf("%0.1f ", matrix[count[world_rank]][i]);
					token = strtok(NULL, " ");
					i++;
				}
				count[world_rank]++;
			}
			printf("Data from file 1 readed\n");

			// read file 2
			printf("Start reading file2...\n");
			count[world_rank] = 0;
			while (fgets(str, 60000, fp2))
			{
				char* token = strtok(str, " ");
				i = 0;
				//printf("\nBefore loop row%d\n", count);
				while (token != NULL)
				{
					matrix2[count[world_rank]][i] = atof(token);
					//printf("%0.1f ", matrix2[count[world_rank]][i]);
					token = strtok(NULL, " ");
					i++;
				}
				count[world_rank]++;
			}
			printf("Data from file 2 readed\n");

			// compile data
			printf("Start compiling...\n");
			StartTime = MPI_Wtime();
			count[world_rank] = 0;
			while (count[world_rank] < matdim1)
			{
				i = 0;
				while (i < matdim2)
				{
					ansmat[count[world_rank]][i] = matrix[count[world_rank]][i] + matrix2[count[world_rank]][i];
					i++;
				}
				count[world_rank]++;
			}
			printf("Complied\n");
			EndTime = MPI_Wtime();
			printf("Time : %f seconds\n", EndTime - StartTime);

			// print output to file
			printf("Starting print out to file...\n");
			count[world_rank] = 0;
			fprintf(fp3, "%d %d\n", matdim1, matdim2);
			while (count[world_rank] < matdim1)
			{
				i = 0;
				while (i < matdim2)
				{
					//printf("%.01f", ansmat[count[world_rank]][i]);
					fprintf(fp3, "%.01f", ansmat[count[world_rank]][i]);
					i++;
					if (i < (matdim2))
					{
						fprintf(fp3, " ");
						//printf(" ");
					}
				}
				fprintf(fp3, "\n");
				//printf("\n");
				count[world_rank]++;
			}
			printf("File printed.\n");

		}
		else // if there are more than 1
		{
			// read file 1
			printf("Start reading file1 at row%d ...\n", (threshold * world_rank));
			count[world_rank] = 0;
			while (fgets(str, 60000, fp1) && count[world_rank] < threshold * (world_rank + 1))
			{
				char* token = strtok(str, " ");
				i = 0;
				//printf("\nBefore loop row%d\n",count);
				while (token != NULL)
				{
					matrix[count[world_rank]][i] = atof(token);
					//printf("%0.2f ", matrix[count[world_rank]][i]);
					token = strtok(NULL, " ");
					i++;
				}
				count[world_rank]++;
			}
			printf("Datain file1 readed\n");

			// read file 2
			printf("Start reading file2 at row%d ...\n", (threshold * world_rank));
			count[world_rank] = 0;
			while (fgets(str, 60000, fp2) && count[world_rank] < threshold * (world_rank + 1))
			{
				char* token = strtok(str, " ");
				i = 0;
				//printf("\nBefore loop row%d\n", count);
				while (token != NULL)
				{
					matrix2[count[world_rank]][i] = atof(token);
					//printf("%0.2f ", matrix2[count[world_rank]][i]);
					token = strtok(NULL, " ");
					i++;
				}
				count[world_rank]++;
			}
			printf("Data in file2 readed\n");

			// compile data
			printf("Start computing...\n");
			StartTime = MPI_Wtime();
			count[world_rank] = 0;
			while (count[world_rank] < threshold * (world_rank + 1))
			{
				i = 0;
				while (i < matdim2)
				{
					ansmat[count[world_rank]][i] = matrix[count[world_rank]][i] + matrix2[count[world_rank]][i];
					/*printf("%0.1f-", matrix[count[world_rank]][i]);
					printf("%0.1f-", matrix2[count[world_rank]][i]);
					printf("%0.1f ", ansmat[count[world_rank]][i]);*/
					i++;
				}
				count[world_rank]++;
			}
			count[world_rank] = threshold * world_size;
			printf("Computed\n");
			EndTime = MPI_Wtime();
			printf("Time : %f\n seconds", EndTime - StartTime);

			// recieve data
			printf("Start receiving data...\n");
			count[world_rank] = threshold * (world_rank + 1);
			while (count[world_rank] < matdim1)
			{
				MPI_Recv(&(ansmat[count[world_rank]][0]), matdim2, MPI_FLOAT, MPI_ANY_SOURCE, count[world_rank], MPI_COMM_WORLD, &status);
				count[world_rank]++;
			}
			printf("Data received\n");


			// print output to file
			printf("Starting print out to file...\n");
			count[world_rank] = 0;
			fprintf(fp3, "%d %d\n", matdim1, matdim2);
			while (count[world_rank] < matdim1)
			{
				i = 0;
				while (i < matdim2)
				{
					//printf("%.01f", ansmat[count[world_rank]][i]);
					fprintf(fp3, "%.01f", ansmat[count[world_rank]][i]);
					i++;
					if (i < (matdim2))
					{
						//printf(" ");
						fprintf(fp3, " ");
					}
				}
				//printf("\n");
				fprintf(fp3, "\n");
				count[world_rank]++;
			}
			printf("File printed.\n");

			printf("End rank 0\n");
		}
	}
	else if (world_rank == (world_size - 1)) // the last rank will do the rest
	{
		printf("I am rank %d\n", world_rank);

		// read file 1
		printf("Start reading file1 at row%d ...\n", (threshold * world_rank));
		count[world_rank] = world_rank * threshold;
		i = 0;
		while (i < count[world_rank])
		{
			fgets(str, 60000, fp1);
			i++;
		}
		while (fgets(str, 60000, fp1))
		{
			char* token = strtok(str, " ");
			i = 0;
			//printf("\nBefore loop row%d from rank %d\n", count[world_rank]);
			while (token != NULL)
			{
				matrix[count[world_rank]][i] = atof(token);
				//printf("%0.2f ", matrix[count[world_rank]][i]);
				token = strtok(NULL, " ");
				i++;
			}
			count[world_rank]++;
		}
		printf("Data from file1 readed\n");

		// read file 2
		printf("Start reading file2 at row%d ...\n", (threshold * world_rank));
		count[world_rank] = world_rank * threshold;
		i = 0;
		while (i < count[world_rank])
		{
			fgets(str, 60000, fp2);
			i++;
		}
		while (fgets(str, 60000, fp2))
		{
			char* token = strtok(str, " ");
			i = 0;
			//printf("\nBefore loop row%d from rank %d\n", count[world_rank]);
			while (token != NULL)
			{
				matrix2[count[world_rank]][i] = atof(token);
				//printf("%0.2f ", matrix2[count[world_rank]][i]);
				token = strtok(NULL, " ");
				i++;
			}
			count[world_rank]++;
		}
		printf("Data from file2 readed\n");

		// compile data
		printf("Start compile\n");
		count[world_rank] = world_rank * threshold;
		while (count[world_rank] < matdim1)
		{
			i = 0;
			while (i < matdim2)
			{
				ansmat[count[world_rank]][i] = matrix[count[world_rank]][i] + matrix2[count[world_rank]][i];
				//printf("%0.1f+", matrix[count[world_rank]][i]);
				//printf("%0.1f=", matrix2[count[world_rank]][i]);
				//printf("%0.1f ", ansmat[count[world_rank]][i]);
				i++;
			}
			//printf("\n");
			count[world_rank]++;
		}
		printf("Complied\n");

		// send data to rank 0
		printf("Start sending data...\n");
		count[world_rank] = (threshold * world_rank);
		while (count[world_rank] < matdim1)
		{
			MPI_Send(&(ansmat[count[world_rank]][0]), matdim2, MPI_FLOAT, 0, count[world_rank], MPI_COMM_WORLD);
			count[world_rank]++;
		}
		printf("Data sended\n");

		printf("End rank %d\n", world_rank);
	}
	else
	{
		printf("I am rank %d in else\n", world_rank);

		// read file 1
		printf("Start reading file1 at row%d ...\n", (threshold * world_rank));
		count[world_rank] = world_rank * threshold;
		k = 0;
		while (k < count[world_rank])
		{
			fgets(str, 60000, fp1);
			k++;
		}
		while (fgets(str, 60000, fp1) && count[world_rank] < threshold * (world_rank + 1))
		{
			char* token = strtok(str, " ");
			k = 0;
			//printf("\nBefore loop row%d from rank %d\n", count[world_rank], world_rank);
			while (token != NULL)
			{
				matrix[count[world_rank]][k] = atof(token);
				//printf("%0.2f ", matrix[count[world_rank]][k]);
				token = strtok(NULL, " ");
				k++;
			}
			count[world_rank]++;
		}
		printf("Data from file1 readed\n");

		// read file 2
		printf("Start reading file2 at row%d ...\n", (threshold * world_rank));
		count[world_rank] = world_rank * threshold;
		k = 0;
		for (int k = 0; k < count[world_rank]; k++)
		{
			fgets(str, 60000, fp2);
		}
		while (fgets(str, 60000, fp2) && count[world_rank] < threshold * (world_rank + 1))
		{
			char* token = strtok(str, " ");
			k = 0;
			//printf("\nBefore loop row%d\n", count[world_rank]);
			while (token != NULL)
			{
				matrix2[count[world_rank]][k] = atof(token);
				//printf("%0.2f ", matrix2[count[world_rank]][k]);
				token = strtok(NULL, " ");
				k++;
			}
			count[world_rank]++;
		}
		printf("Data from file2 readed\n");

		// compile data
		printf("Start compile\n");
		count[world_rank] = world_rank * threshold;
		while (count[world_rank] < threshold * (world_rank + 1))
		{
			for (int x = 0; x < matdim2; x++)
			{
				ansmat[count[world_rank]][x] = matrix[count[world_rank]][x] + matrix2[count[world_rank]][x];
				//printf("%0.1f+", matrix[count[world_rank]][x]);
				//printf("%0.1f=", matrix2[count[world_rank]][x]);
				//printf("%0.1f ", ansmat[count[world_rank]][x]);
			}
			count[world_rank]++;
		}
		printf("Complied\n");

		printf("Start sending data...\n");
		count[world_rank] = threshold * (world_rank);
		while (count[world_rank] < threshold * (world_rank + 1))
		{
			MPI_Send(&(ansmat[count[world_rank]][0]), matdim2, MPI_FLOAT, 0, count[world_rank], MPI_COMM_WORLD);
			//printf("%0.1f", ansmat[count[world_rank]][0]);
			count[world_rank]++;
		}
		printf("Data sended\n");

		printf("End rank %d\n", world_rank);
	}


	// close files that we open 
	fclose(fp1);
	fclose(fp2);
	if (world_rank == 0)
		fclose(fp3);

	// free matrix that we allocated
	free(matrix[0]);
	free(matrix);
	free(matrix2[0]);
	free(matrix2);
	free(ansmat[0]);
	free(ansmat);

	// close MPI
	MPI_Finalize();
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

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
