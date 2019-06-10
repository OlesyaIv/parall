#include "stdio.h"
#include "mpi.h"
#include "fstream"
double startT, stopT;


int* mergeArrays(int* v1, int n1, int* v2, int n2)
{
	int i, j, k;
	int* result;

	result = new int[n1 + n2];
	i = 0;
	j = 0;
	k = 0;

	while (i < n1 && j < n2)
		if (v1[i] < v2[j]) {
			result[k] = v1[i];
			i++;
			k++;
		}
		else {
			result[k] = v2[j];
			j++;
			k++;
		}

	if (i == n1)
		while (j < n2) {
			result[k] = v2[j];
			j++;
			k++;
		}
	if (j == n2)
		while (i < n1) {
			result[k] = v1[i];
			i++;
			k++;
		}

	return result;
}

void swap(int* v, int i, int j)
{
	int t;
	t = v[i];
	v[i] = v[j];
	v[j] = t;
}

void sort(int* v, int n)
{
	int i, j;

	for (i = n - 2; i >= 0; i--)
		for (j = 0; j <= i; j++)
			if (v[j] > v[j + 1])
				swap(v, j, j + 1);
}

using namespace std;

int main(int argc, char ** argv)
{
	int* data = 0;            //Our unsorted array
	int* resultant_array; //Sorted Array
	int* sub;

	int m, n;
	int id, p;
	int r;
	int s;
	int i;
	int move;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	srand(unsigned int(MPI_Wtime()));

	//Task Of The Master Processor
	if (id == 0) {
		n = 20000;
		s = n / p;
		r = n % p;
		data = new int[n + s - r];

		
		ofstream file("input");

		for (i = 0; i < n; i++)
		{
			data[i] = rand() % 15000;
			file << data[i] << " ";
		}

		file.close();

		if (r != 0) {
			for (i = n; i < n + s - r; i++)
				data[i] = 0;

			s = s + 1;
		}

		startT = MPI_Wtime();    //Start Time.                           
		MPI_Bcast(&s, 1, MPI_INT, 0, MPI_COMM_WORLD);
		resultant_array = new int[s];
		MPI_Scatter(data, s, MPI_INT, resultant_array, s, MPI_INT, 0, MPI_COMM_WORLD);
		sort(resultant_array, s);
	}
	else {
		MPI_Bcast(&s, 1, MPI_INT, 0, MPI_COMM_WORLD);
		resultant_array = new int[s];
		MPI_Scatter(data, s, MPI_INT, resultant_array, s, MPI_INT, 0, MPI_COMM_WORLD);
		sort(resultant_array, s); 
	}

	move = 1;

	//Merging the sub sorted arrays to obtain resultant sorted array
	while (move < p) {
		if (id % (2 * move) == 0) {
			if (id + move < p) {                 
				MPI_Recv(&m, 1, MPI_INT, id + move, 0, MPI_COMM_WORLD, &status);
				sub = new int[m];
				MPI_Recv(sub, m, MPI_INT, id + move, 0, MPI_COMM_WORLD, &status);
				resultant_array = mergeArrays(resultant_array, s, sub, m);
				s = s + m;
			}
		}
		else {
			int near = id - move;
			MPI_Send(&s, 1, MPI_INT, near, 0, MPI_COMM_WORLD);
			MPI_Send(resultant_array, s, MPI_INT, near, 0, MPI_COMM_WORLD);
			break;
		}

		move = move * 2;
	}

	//Results
	if (id == 0) {
		
		stopT = MPI_Wtime();
		double parallelTime = stopT - startT;	

		printf("\n\n\nTime: %f", parallelTime);

		startT = MPI_Wtime();
		sort(data, n);
		stopT = MPI_Wtime();

		double poslTime = stopT - startT;
		printf("\n Time: %f \n", stopT - startT);
		printf("\n SpeedUp: %f \n\n\n", poslTime / parallelTime);

		ofstream file1("result");
		file1 << parallelTime << " - Parallel Time \n";
		file1 << poslTime << " - Posledov Time \n";
		file1 << poslTime / parallelTime << " - KPD \n";
		file1.close();

		ofstream file("output");
		for (i = 0; i < n; i++)
		{
			file << resultant_array[i] << " ";
		}
		file.close();

		ofstream file2("output1");
		for (i = 0; i < n; i++)
		{
			file2 << data[i] << " ";
		}
		for (i = 0; i < n + 1;)
		{
			if (i == n) {
				file2 << ("\n Test succesfull \n");
				break;
			}
			if (resultant_array[i] == data[i])
				i++;
			else file2 << ("\n Test not succesfull \n");
		}

		file2.close();
	}

	MPI_Finalize();
	system("pause");
}