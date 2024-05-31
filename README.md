# Compile o código com mpicc

mpicc -o matrix_mult main.c

# Execute o código com mpirun, especificando o número de processos (-np)

# Aqui usamos 4 processos (1 mestre e 3 escravos)

mpirun -np 4 ./matrix_mult
