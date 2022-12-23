/*INF1029 - INT ARQUITETURA COMPUTADORES - 2022.2 - 3WA
Trabalho 3 - Módulo avançado (AVX/FMA) para operações com matrizes
Nome: Eric Leão     Matrícula: 2110694
Nome: Pedro Machado Peçanha    Matrícula: 2110535*/

#include <immintrin.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct matrix {
  unsigned long int height;
  unsigned long int width;
  float *rows;
} Matrix;

struct matrix *matrix1;
struct matrix *matrix2;
struct matrix *matrix3;
float scalar;
int nthreads = 1;

void *sm(void *a) {
  unsigned long i = (long)a * (matrix1->width * matrix1->height) / nthreads;
  unsigned long f =
      ((long)a + 1) * (matrix1->width * matrix1->height) / nthreads;
  __m256 vec1;
  __m256 vec2;
  __m256 soma;
  for (; i < f; i += 8) {
    vec1 = _mm256_load_ps(&matrix1->rows[i]);
    vec2 = _mm256_set1_ps(scalar);
    soma = _mm256_mul_ps(vec1, vec2);
    _mm256_store_ps(&matrix1->rows[i], soma);
  }
  pthread_exit(NULL); /*not necessary*/
}

void *mm(void *a) {
  unsigned long i = (long)a * matrix1->height / nthreads;
  unsigned long f = ((long)a + 1) * matrix1->height / nthreads;
  __m256 vec0;
  __m256 vec1;
  __m256 vec2;
  __m256 vecZero = _mm256_setzero_ps();
  // printf("i = %d\na = %d\n", (long)a * matrix1->width/nthreads,((long)a+1) *
  // matrix1->width/nthreads);
  for (; i < f; i++) {
    for (long j = 0; j < matrix1->width; j++) {
      vec1 = _mm256_set1_ps(*(matrix1->rows + i * matrix1->width + j));
      for (long k = 0; k < matrix2->width; k += 8) {
        if (j == 0) { // testa se esta passando pela primeira vez no elemento
                      // damatrix
          _mm256_store_ps(matrix3->rows + i * matrix3->width + k, vecZero);
        }
        vec0 = _mm256_load_ps(matrix3->rows + i * matrix3->width + k);
        vec2 = _mm256_load_ps(matrix2->rows + j * matrix2->width + k);
        vec0 = _mm256_fmadd_ps(vec1, vec2, vec0);
        _mm256_store_ps(matrix3->rows + i * matrix3->width + k, vec0);
      }
    }
  }
  pthread_exit(NULL); /*not necessary*/
}

void set_number_threads(int num_threads) { nthreads = num_threads; }

int scalar_matrix_mult(float scalar_value, struct matrix *matrix) {
  if (matrix == NULL)
    return 0;
  if ((matrix->width * matrix->height) % 8)
    return 0;
  pthread_t threads[nthreads];
  matrix1 = matrix;
  scalar = scalar_value;
  for (long a = 0; a < nthreads; a++) {
    pthread_create(&threads[a], NULL, sm, a);
  }
  for (long i = 0; i < nthreads; i++) {
    pthread_join(threads[i], NULL);
  }
  return 1;
}

int matrix_matrix_mult(struct matrix *matrixA, struct matrix *matrixB,
                       struct matrix *matrixC) {
  if (matrixA == NULL)
    return 0; // matrx A diferente de NULL
  if (matrixB == NULL)
    return 0; // matrx B diferente de NULL
  if (matrixC == NULL)
    return 0; // matrx C diferente de NULL
  if (matrixA->width != matrixB->height)
    return 0; // matrx A precisa ter largura igual à altura da matrix B
  if (matrixA->height != matrixC->height || matrixB->width != matrixC->width)
    return 0; // matriz C tem que ter altura e largura compativeis com a amatriz
              // A e B
  matrix1 = matrixA;
  matrix2 = matrixB;
  matrix3 = matrixC;
  pthread_t threads[nthreads];
  if ((matrixA->height % 8) != 0 || (matrixB->height % 8) != 0 ||
      (matrixB->width % 8) != 0)
    return 0;
  for (long a = 0; a < nthreads; a++) {
    pthread_create(&threads[a], NULL, mm, a);
  }
  for (long i = 0; i < nthreads; i++) {
    pthread_join(threads[i], NULL);
  }
  return 1;
}
