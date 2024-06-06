# Compile o código com mpicc

mpicc -o matrix_mult main.c

# Execute o código com mpiexec, especificando o número de processos

mpiexec -np <num_processes> matrix_mult.exe <matrix_size>
