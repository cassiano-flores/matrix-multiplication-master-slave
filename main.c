#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 3 // Tamanho das matrizes (para simplicidade, usamos matrizes quadradas)

// Função para imprimir matrizes
void print_matrix(int matrix[N][N])
{
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      printf("%d ", matrix[i][j]);
    }
    printf("\n");
  }
}

int main(int argc, char *argv[])
{
  int my_rank, proc_n;
  MPI_Status status;

  // Inicializa o MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

  // Matrizes e vetores de trabalho
  int A[N][N], B[N][N], C[N][N];

  if (my_rank == 0)
  {
    int choice;
    printf("Do you want to insert the matrices or use the example model? \n1. Insert matrices \n2. Use example\n");
    scanf("%d", &choice);

    if (choice == 1)
    {
      // Solicita ao usuário para inserir os valores das matrizes
      printf("Enter the matrices with size: %dx%d in the format: 'a11 a12 ... a1%d a21 a22 ... a2%d ... a%d%d b11 b12 ... b1%d b21 b22 ... b2%d ... b%d%d'\n", N, N, N, N, N, N, N, N, N, N);
      for (int i = 0; i < N; i++)
      {
        for (int j = 0; j < N; j++)
        {
          scanf("%d", &A[i][j]);
        }
      }
      for (int i = 0; i < N; i++)
      {
        for (int j = 0; j < N; j++)
        {
          scanf("%d", &B[i][j]);
        }
      }
    }
    else
    {
      // Inicializa as matrizes A e B com valores de exemplo
      for (int i = 0; i < N; i++)
      {
        for (int j = 0; j < N; j++)
        {
          A[i][j] = i + j;
          B[i][j] = i * j;
        }
      }
    }

    // Imprime as matrizes A e B
    printf("Matrix A:\n");
    print_matrix(A);
    printf("Matrix B:\n");
    print_matrix(B);

    // Distribui linhas de A para os escravos
    for (int i = 0; i < N; i++)
    {
      int dest = (i % (proc_n - 1)) + 1;
      MPI_Send(A[i], N, MPI_INT, dest, i, MPI_COMM_WORLD);
    }

    // Recebe linhas calculadas de C dos escravos
    for (int i = 0; i < N; i++)
    {
      MPI_Recv(C[i], N, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }

    // Imprime a matriz resultante C
    printf("Matrix C (Result of A * B):\n");
    print_matrix(C);
  }
  else
  {
    // Papel dos escravos
    for (int i = my_rank - 1; i < N; i += (proc_n - 1))
    {
      int row[N];
      MPI_Recv(row, N, MPI_INT, 0, i, MPI_COMM_WORLD, &status);

      int result[N];
      for (int j = 0; j < N; j++)
      {
        result[j] = 0;
        for (int k = 0; k < N; k++)
        {
          result[j] += row[k] * B[k][j];
        }
      }
      MPI_Send(result, N, MPI_INT, 0, i, MPI_COMM_WORLD);
    }
  }

  // Finaliza o MPI
  MPI_Finalize();
  return 0;
}
