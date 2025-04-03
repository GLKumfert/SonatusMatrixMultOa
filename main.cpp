// Take parameters: p, q and r, each < 100
// Each element of the matrix is a random integers from 0 … 9
// Create matrix, A, of dimension p x q
// Create matrix, B, of dimension q x r
// Compute C = A x B
//multithread it
/*Each element of C should be computed in a thread of it’s own. The program should first create the number of threads needed for the computation. Note that the dimension of C is p x r.
The main thread will need to send the row and column necessary to compute a particular entry of the result. The result matrix should be updated with the value returned by each thread.
The program should print the input matrices and the result matrix in a readable format when it finishes.*/


//written and tested on a M1 Mac, hence use of pthread
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <pthread.h>

using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::duration_cast;

//for std:thread test
int calc_matrix_mult(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, int i, int j) {
    int sum = 0;
    for (int k = 0; k < A[0].size(); ++k) {
        sum += A[i][k] * B[k][j];
    }
    return sum;
}

int calc_matrix_mult_transposed(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, int i, int j) {
  int sum = 0;
  for (int k = 0; k < A[0].size(); ++k) {
      sum += A[i][k] * B[j][k];
  }
  return sum;
}

// //for pthread test
// class Task {
//     int id;
//     int ntasks;
//     int x; // ans to calculate
//     int y;
//     std::vector<std::vector<int>>* C;
//     std::vector<std::vector<int>>* A;
//     std::vector<std::vector<int>>* B;
//   public:
//     static void* runner(void* arg) {
//         reinterpret_cast<Task*>(arg)->execute();
//         return nullptr;
//     }

//     Task(int id, int ntasks, std::vector<std::vector<int>>* C, std::vector<std::vector<int>>* A, std::vector<std::vector<int>>* B, int x, int y)
//         : id(id), ntasks(ntasks), C(C), A(A), B(B), x(x), y(y) {}

//     void execute() {
//         auto& matrixA = *A;
//         auto& matrixB = *B;
//         auto& matrixC = *C;
//         matrixC[x][y] = 0;
//         for (int k = 0; k < matrixA[0].size(); ++k) {
//           matrixC[x][y] += matrixA[x][k] * matrixB[k][y];
//         }
//     }

// };
int main() {
    int p, q, r;
    std::cout << "Enter dimensions of matrix A (p x q), and B(q x r) as \"p q r \" (p,q,r < 100): ";
    std::cin >> p >> q >> r;
    std::vector<std::vector<int>> A(p, std::vector<int>(q));
    std::vector<std::vector<int>> B(q, std::vector<int>(r));
    //random init of matrix
    srand(time(0));
    for (int i = 0; i < p; ++i) {
        for (int j = 0; j < q; ++j) {
            A[i][j] = rand() % 10; // random ints from 0 to 9
        }
    }

    for (int i = 0; i < q; ++i) {
        for (int j = 0; j < r; ++j) {
            B[i][j] = rand() % 10; // random ints from 0 to 9
        }
    }

    // ---- transpose B with time(negligble)
    std::vector<std::vector<int>> B_transposed(r, std::vector<int>(q));
    high_resolution_clock::time_point begin_transpose = high_resolution_clock::now();

    for (int i = 0; i < q; ++i) {
        for (int j = 0; j < r; ++j) {
            B_transposed[j][i] = B[i][j];
        }
    }
    auto time_span_transpose = duration_cast<duration<double>>(high_resolution_clock::now() - begin_transpose);

    std::vector<std::thread> threads;
    // --- time without transposing B
    std::vector<std::vector<int>> C(p, std::vector<int>(r, 0));
    // start timing 
    high_resolution_clock::time_point begin = high_resolution_clock::now();

    for (int i = 0; i < p; ++i) {
        for (int j = 0; j < r; ++j) {
            threads.emplace_back([&, i, j]() {
                C[i][j] = calc_matrix_mult(A, B, i, j);
            });
        }
    }
    for (auto& thread : threads) {
        thread.join();
    }
    // end timing
    auto time_span = duration_cast<duration<double>>(high_resolution_clock::now() - begin);

    // --- time with transposing B
    std::vector<std::vector<int>> C_transposed(p, std::vector<int>(r, 0));
    // start timing 
    high_resolution_clock::time_point begin_mat_transpose = high_resolution_clock::now();
    threads.clear();
    for (int i = 0; i < p; ++i) {
        for (int j = 0; j < r; ++j) {
            threads.emplace_back([&, i, j]() {
                C_transposed[i][j] = calc_matrix_mult_transposed(A, B_transposed, i, j);
            });
        }
    }
    for (auto& thread : threads) {
        thread.join();
    }
    // end timing
    auto time_span_mat_transpose = duration_cast<duration<double>>(high_resolution_clock::now() - begin_mat_transpose);
    for(int i = 0; i < p; ++i) {
      for(int j = 0; j < r; ++j) {
        if(C[i][j] != C_transposed[i][j]) {
          std::cout << "Error in matrix multiplication at C[" << i << "][" << j << "]" << std::endl;
          return 1;
        }
      }
    }

    // output matrices
    std::cout << "Matrix A:" << std::endl;
    for (const auto& row : A) {
        for (const auto& elem : row) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "Matrix B:" << std::endl;
    for (const auto& row : B) {
        for (const auto& elem : row) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "Matrix C:" << std::endl;
    for (const auto& row : C) {
        for (const auto& elem : row) {
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }
    // std::cout <<"Matrix C (using transposed B):" << std::endl;
    // for (const auto& row : C_transposed) {
    //     for (const auto& elem : row) {
    //         std::cout << elem << " ";
    //     }
    //     std::cout << std::endl;
    // }

    //output timings
    std::cout << "Time taken for matrix multiplication (only threads): " << time_span.count() << " seconds" << std::endl;
    std::cout << "Time taken for matrix transpose: " << time_span_transpose.count() << " seconds" << std::endl;
    std::cout << "Time taken for matrix multiplication using transposed B: " << time_span_mat_transpose.count() << " seconds" << std::endl;
    std::cout << "Speed up: " << time_span.count() - time_span_transpose.count() - time_span_mat_transpose.count()<< " seconds, or " <<time_span.count()/time_span_mat_transpose.count()*100 << "%" << std::endl;

    return 0;
}