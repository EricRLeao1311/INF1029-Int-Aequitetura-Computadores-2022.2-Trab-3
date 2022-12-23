/*INF1029 - INT ARQUITETURA COMPUTADORES - 2022.2 - 3WA
Trabalho 3 - Implementação do algoritmo otimizado para o produto de matrizes
Nome: Eric Leão     Matrícula: 2110694
Nome: Pedro Machado Peçanha    Matrícula: 2110535*/

#include "matrix_lib.h"
#include "timer.h"
#include <assert.h>
#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>

void fill_matrix(struct matrix *matrixX, FILE *arquivoBinX);

void fill_binary(struct matrix *matrixX, FILE *arquivoBinX);

void print_matrix(struct matrix matrix, char matrixChar);
int main(int argc, char *argv[]) {
  int height1 = atoi(argv[2]), height2 = atoi(argv[4]), width1 = atoi(argv[3]),
      width2 = atoi(argv[5]);
  FILE *arquivoBin1, *arquivoBin2;
  float scalar = strtof(argv[1], NULL), diferencaTotal;
  //  float *matrixRows;
  char *filename1 = argv[7], *filename2 = argv[8], *filename3 = argv[9],
       *filename4 = argv[10];
  set_number_threads(atoi(argv[6]));
  struct timeval overall_t1, overall_t2, start, stop;
  // Mark overall start time
  gettimeofday(&overall_t1, NULL);
  struct matrix matrix1, matrix2, matrix3;
  // matrix 1
  {
    matrix1.height = height1;
    matrix1.width = width1;
    matrix1.rows = (float *)aligned_alloc(32, sizeof(float) * height1 * width1);
    assert(matrix1.rows);
  }
  // matrix 2
  {
    matrix2.height = height2;
    matrix2.width = width2;
    matrix2.rows = (float *)aligned_alloc(32, sizeof(float) * height2 * width2);
    assert(matrix2.rows);
  }
  // matrix 3
  {
    matrix3.height = height1;
    matrix3.width = width2;
    matrix3.rows = (float *)aligned_alloc(32, sizeof(float) * height1 * width2);
    assert(matrix3.rows);
  }

  arquivoBin1 = fopen(filename1, "rb");
  assert(arquivoBin1);
  arquivoBin2 = fopen(filename2, "rb");
  assert(arquivoBin2);
  fill_matrix(&matrix1, arquivoBin1);
  fill_matrix(&matrix2, arquivoBin2);
  int matrixSize = matrix3.height * matrix3.width;
  __m256 vec = _mm256_setzero_ps();
  for (int c = 0; c < matrixSize; c += 8) {
    _mm256_store_ps(matrix3.rows + c, vec);
  }
  print_matrix(matrix1, 'A');
  print_matrix(matrix2, 'B');
  print_matrix(matrix3, 'C');
  // Mark overall start time
  puts("Executing scalar_matrix_mult...");
  gettimeofday(&start, NULL);
  scalar_matrix_mult(scalar, &matrix1);
  gettimeofday(&stop, NULL);
  printf("Scalar Matrix Mult's time: %f ms\n",
         timedifference_msec(start, stop));
  print_matrix(matrix1, 'A');
  // Mark matrix matrix start time
  puts("Executing matrix_matrix_mult...");
  gettimeofday(&start, NULL);
  matrix_matrix_mult(&matrix1, &matrix2, &matrix3);
  gettimeofday(&stop, NULL);
  printf("Matrix Matrix Mult's time: %f ms\n",
         timedifference_msec(start, stop));
  FILE *arquivoBin3 = fopen(filename3, "wb");
  FILE *arquivoBin4 = fopen(filename4, "wb");
  fill_binary(&matrix1, arquivoBin3);
  fill_binary(&matrix3, arquivoBin4);
  fclose(arquivoBin1);
  fclose(arquivoBin2);
  fclose(arquivoBin3);
  fclose(arquivoBin4);
  print_matrix(matrix3, 'C');
int maxError = 0;
  int diffError = 0;
  for (int a = 0; a < matrix3.width * matrix3.height; a++)
    maxError =
        (maxError > (diffError = fabs((double)(matrix3.rows[a] - (20480)))))
            ? maxError
            : diffError;
  printf("erros com thread = %d\n", maxError);
  free(matrix1.rows);
  free(matrix2.rows);
  free(matrix3.rows);
  // Mark overall stop time
  gettimeofday(&overall_t2, NULL);
  // Show elapsed overall time
  diferencaTotal = timedifference_msec(overall_t1, overall_t2);
  printf("Overall time: %.2f ms\n", diferencaTotal);

  return 0;
}

void fill_matrix(struct matrix *matrixX, FILE *arquivoBinX) {
  float *matrixRows;
  float valorLido;
  int aux;
  for (int i = 0; i < (matrixX->width * matrixX->height); i++) {
    matrixRows = matrixX->rows + i;
    aux = fread(&valorLido, sizeof(float), 1, arquivoBinX);
    if (aux == 0) {
      fprintf(stderr, "error reading file\n");
      exit(1);
    }
    *matrixRows = valorLido;
  }
}

void fill_binary(struct matrix *matrixX, FILE *arquivoBinX) {
  float *matrixRows;
  int aux;
  for (int i = 0; i < (matrixX->width * matrixX->height); i++) {
    matrixRows = matrixX->rows + i;
    // printf("%f\n", *matrixRows);
    aux = fwrite(matrixRows, sizeof(float), 1, arquivoBinX);
    if (aux == 0) {
      fprintf(stderr, "error reading file\n");
      exit(1);
    }
  }
}

void print_matrix(struct matrix matrix, char matrixChar) {
  printf("------------- Matrix %c -------------", matrixChar);
  for (int c = 0; c < matrix.height * matrix.width; c++) {
    if (c % 16 == 0) {
      putchar('\n');
    }
    if (c == 256) {
      puts("Ooops...256 printing limit found...skipping printing...");
      break;
    }
    printf("%.2f ", matrix.rows[c]);
  }
  putchar('\n');
  return;
}
