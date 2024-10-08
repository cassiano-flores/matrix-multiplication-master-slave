#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAG_WORK 1
#define TAG_RESULT 2
#define TAG_SUICIDE 3

void mestre(int proc_n, int N);
void escravo(int my_rank, int N);

int main(int argc, char *argv[])
{
  int my_rank, proc_n, N;
  double start_time, end_time;

  if (argc != 2)
  {
    printf("ERROR! Usage: mpiexec -np <num_processes> matrix_mult.exe <matrix_size>\n");
    exit(EXIT_FAILURE);
  }

  N = atoi(argv[1]);

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

  // se for o primeiro processo, ele será o mestre
  if (my_rank == 0)
  {
    start_time = MPI_Wtime();
    mestre(proc_n, N);
    end_time = MPI_Wtime();
    printf("Execution time: %fs\n", end_time - start_time);
  }
  else
    escravo(my_rank, N); // todos demais são escravos

  MPI_Finalize();
  return 0;
}

// método do mestre, responsável por todo o gerenciamento de envio e recebimento
void mestre(int proc_n, int N)
{
  int i, j;
  int **A, **B, **C;
  MPI_Status status;

  // aloca as matrizes
  A = malloc(N * sizeof(int *));
  B = malloc(N * sizeof(int *));
  C = malloc(N * sizeof(int *));
  for (i = 0; i < N; i++)
  {
    A[i] = malloc(N * sizeof(int));
    B[i] = malloc(N * sizeof(int));
    C[i] = calloc(N, sizeof(int));
  }

  // inicializa as matrizes com valores aleatórios (entre 1 e 99)
  srand(time(NULL));
  for (i = 0; i < N; i++)
  {
    for (j = 0; j < N; j++)
    {
      A[i][j] = rand() % 99 + 1;
      B[i][j] = rand() % 99 + 1;
    }
  }

  // distribui as linhas da matriz A e as colunas da matriz B para os escravos
  int linhas_distribuidas = 0;
  for (i = 1; i < proc_n && linhas_distribuidas < N * N; i++)
  {
    int data[2 * N + 2];
    data[0] = linhas_distribuidas / N;
    data[1] = linhas_distribuidas % N;
    for (j = 0; j < N; j++)
    {
      data[2 + j] = A[data[0]][j];
      data[2 + N + j] = B[j][data[1]];
    }
    // envia os dados para o escravo
    MPI_Send(data, 2 * N + 2, MPI_INT, i, TAG_WORK, MPI_COMM_WORLD);
    linhas_distribuidas++;
  }

  // recebe os resultados dos escravos e atualiza a matriz C (resultado da multiplicação)
  for (i = 0; i < N * N; i++)
  {
    int result[3];
    MPI_Recv(result, 3, MPI_INT, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);
    C[result[0]][result[1]] = result[2];

    // envia mais trabalho para o escravo que terminou, se ainda houver trabalho
    if (linhas_distribuidas < N * N)
    {
      int data[2 * N + 2];
      data[0] = linhas_distribuidas / N;
      data[1] = linhas_distribuidas % N;
      for (j = 0; j < N; j++)
      {
        data[2 + j] = A[data[0]][j];
        data[2 + N + j] = B[j][data[1]];
      }
      MPI_Send(data, 2 * N + 2, MPI_INT, status.MPI_SOURCE, TAG_WORK, MPI_COMM_WORLD);
      linhas_distribuidas++;
    }
    else
      // se não houver mais trabalho, envia um sinal para o escravo terminar
      MPI_Send(result, 0, MPI_INT, status.MPI_SOURCE, TAG_SUICIDE, MPI_COMM_WORLD);
  }

  // caso queira ver os valores gerados, basta descomentar, imprime as matrizes A, B e C
  /*   printf("Matriz A:\n");
    for (i = 0; i < N; i++)
    {
      for (j = 0; j < N; j++)
      {
        printf("%d ", A[i][j]);
      }
      printf("\n");
    }

    printf("Matriz B:\n");
    for (i = 0; i < N; i++)
    {
      for (j = 0; j < N; j++)
      {
        printf("%d ", B[i][j]);
      }
      printf("\n");
    }

    printf("Matriz C:\n");
    for (i = 0; i < N; i++)
    {
      for (j = 0; j < N; j++)
      {
        printf("%d ", C[i][j]);
      }
      printf("\n");
    } */

  for (i = 0; i < N; i++)
  {
    free(A[i]);
    free(B[i]);
    free(C[i]);
  }
  free(A);
  free(B);
  free(C);
}

// método do escravo, apenas recebe linha e coluna, trabalha, resolve
void escravo(int my_rank, int N)
{
  MPI_Status status;
  while (1)
  {
    int data[2 * N + 2];
    MPI_Recv(data, 2 * N + 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    // quando receber sinal de término, encerra
    if (status.MPI_TAG == TAG_SUICIDE)
      break;

    int row = data[0];
    int col = data[1];
    int result = 0;

    // trabalho
    for (int k = 0; k < N; k++)
      result += data[2 + k] * data[2 + N + k];

    int result_data[3] = {row, col, result};
    // envia o resultado
    MPI_Send(result_data, 3, MPI_INT, 0, TAG_RESULT, MPI_COMM_WORLD);
  }
}
