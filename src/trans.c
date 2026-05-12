/*
 * trans.c - 矩阵转置B=A^T
 *每个转置函数都必须具有以下形式的原型：
 *void trans（int M，int N，int a[N][M]，int B[M][N]）；
 *通过计算，块大小为32字节的1KB直接映射缓存上的未命中数来计算转置函数。
 */
#include <stdio.h>
#include "cachelab.h"
int is_transpose(int M, int N, int A[N][M], int B[M][N]);

char transpose_submit_desc[] = "Transpose submission";  //请不要修改“Transpose_submission”
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
  //                          请在此处添加代码
  //*************************************Begin********************************************************

  // s = 5, E = 1, b = 5

  int i, j, row, col;
  int a0, a1, a2, a3, a4, a5, a6, a7;
  if (M == 32 && N == 32) {
    // 8x8 tile for 32x32 matrix
    for (i = 0; i < N; i += 8) {
      for (j = 0; j < M; j += 8) {
        for (row = i; row < i + 8; ++row) {
          for (col = j; col < j + 8; ++col) {
            if (row != col) {
              B[col][row] = A[row][col];
            } else {
              // special handling for diagonal elements to avoid cache conflict
              a0 = A[row][col];
              a1 = row;
            }
          }
          if (i == j)
            B[a1][a1] = a0;
        }
      }
    }
  } else if (M == 64 && N == 64) {
    for (i = 0; i < N; i += 8) {
      for (j = 0; j < M; j += 8) {
        // stage 1: copy A11^T to B11 and store A12^T in B12 temporarily
        for (row = i; row < i + 4; ++row) {
          // fetch the whole block cache data
          // A11 part
          a0 = A[row][j + 0];
          a1 = A[row][j + 1];
          a2 = A[row][j + 2];
          a3 = A[row][j + 3];
          // A12 part
          a4 = A[row][j + 4];
          a5 = A[row][j + 5];
          a6 = A[row][j + 6];
          a7 = A[row][j + 7];

          // store A11^T into B11
          B[j + 0][row] = a0;
          B[j + 1][row] = a1;
          B[j + 2][row] = a2;
          B[j + 3][row] = a3;

          // store A12^T into B12
          B[j + 0][row + 4] = a4;
          B[j + 1][row + 4] = a5;
          B[j + 2][row + 4] = a6;
          B[j + 3][row + 4] = a7;
        }
        // stage 2: copy A12^T from B12 to B21 and store A12^T in B12
        for (col = j; col < j + 4; ++col) {
          // fetch A12^T part from B12
          a0 = B[col][i + 4];
          a1 = B[col][i + 5];
          a2 = B[col][i + 6];
          a3 = B[col][i + 7];

          // fetch A21^T part
          a4 = A[i + 4][col];
          a5 = A[i + 5][col];
          a6 = A[i + 6][col];
          a7 = A[i + 7][col];

          // store A21^T into B12
          B[col][i + 4] = a4;
          B[col][i + 5] = a5;
          B[col][i + 6] = a6;
          B[col][i + 7] = a7;

          // store A12^T into B21
          B[col + 4][i + 0] = a0;
          B[col + 4][i + 1] = a1;
          B[col + 4][i + 2] = a2;
          B[col + 4][i + 3] = a3;
        }
        // stage 3: copy A22^T to B22
        for (row = i + 4; row < i + 8; ++row) {
          for (col = j + 4; col < j + 8; ++col)
            B[col][row] = A[row][col];
        }
      }
    }
  } else if (M == 61 && N == 67) {
    // 61x67 matrix
    for (i = 0; i < N; i += 16) {
      for (j = 0; j < M; j += 16) {
        for (row = i; row < i + 16 && row < N; ++row) {
          for (col = j; col < j + 16 && col < M; ++col) {
            B[col][row] = A[row][col];
          }
        }
      }
    }
  }
  return ;

  //**************************************End**********************************************************
}

/*
 * 我们在下面定义了一个简单的方法来帮助您开始，您可以根据下面的例子把上面值置补充完整。
 */

 /*
  * 简单的基线转置功能，未针对缓存进行优化。
  */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
  int i, j, tmp;

  for (i = 0; i < N; i++) {
    for (j = 0; j < M; j++) {
      tmp = A[i][j];
      B[j][i] = tmp;
    }
  }

}

/*
 * registerFunctions-此函数向驱动程序注册转置函数。
 *在运行时，驱动程序将评估每个注册的函数并总结它们的性能。这是一种试验不同转置策略的简便方法。
 */
void registerFunctions() {
  /* 注册解决方案函数  */
  registerTransFunction(transpose_submit, transpose_submit_desc);

  /* 注册任何附加转置函数 */
  registerTransFunction(trans, trans_desc);

}

/*
 * is_transpose - 函数检查B是否是A的转置。在从转置函数返回之前，可以通过调用它来检查转置的正确性。
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
  int i, j;

  for (i = 0; i < N; i++) {
    for (j = 0; j < M; ++j) {
      if (A[i][j] != B[j][i]) {
        return 0;
      }
    }
  }
  return 1;
}

