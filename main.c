#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_SIZE 1024*1024*2
#define START_SIZE 1024

int main(int argc, char *argv[])
{
	int rank, p;

	struct timeval t1, t2;
	char *filename = NULL, m[MAX_SIZE] = {0}, m_recv[MAX_SIZE];
	int i, size, iterations = 2000;
	double t;
	FILE *outfile;
	MPI_Status status;

	for (i = 0; i < MAX_SIZE; i++) {
		/* Populate message buffer just so it's not all the same
		 * character */
		m[i] = i % 256;
	}

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&p);

	assert(p>=2);

	printf("my rank=%d\n",rank);
	printf("Rank=%d: number of processes =%d\n",rank,p);

	/* Create and open csv outputfile with default permissions */
	asprintf(&filename, "output_rank_%d.csv", rank);
	outfile = fopen(filename, "w");

	fprintf(outfile, "Size,");
	for (size = START_SIZE; size <= MAX_SIZE; size*=2) {
		fprintf(outfile, "%d,", size);
	}
	fprintf(outfile, "\nTime,");

	for (size = START_SIZE; size <= MAX_SIZE; size *=2) {
		printf("%s messages of size %d bytes...\n", rank ? "Sending" :
			"Receiving", size);

		gettimeofday(&t1, NULL);
		for (i = 0; i < iterations; i++) {
			if (rank == 1)
				MPI_Send(m, size, MPI_CHARACTER, 0, 0,
					MPI_COMM_WORLD);
			else
				MPI_Recv(m_recv, size, MPI_CHARACTER,
					MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
					&status);

		}
		gettimeofday(&t2, NULL);

		/* Get time in milliseconds to send one message on average */
		t = (t2.tv_sec-t1.tv_sec)*1000 + ((double) t2.tv_usec-t1.tv_usec)/1000;
		t /= iterations;

#if 0
		t = ((double)((t2.tv_sec-t1.tv_sec)*1000 + (t2.tv_usec-t1.tv_usec)/1000))/
		    (1000*iterations);
#endif
		fprintf(outfile, "%lf,", t);
	}

	fclose(outfile);
	printf("Time results for %s messages recorded in output file '%s'.\n",
		rank ? "sending" : "receiving", filename);
	free(filename);
	MPI_Finalize();
}
