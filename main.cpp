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
#include <numeric>

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

//for pthread test
class Task {
    int id;
    int ntasks;
    int x; // ans to calculate
    int y;
    std::vector<std::vector<int>>* C;
    std::vector<std::vector<int>>* A;
    std::vector<std::vector<int>>* B;
  public:
    static void* runner(void* arg) {
        reinterpret_cast<Task*>(arg)->execute();
        return nullptr;
    }

    Task(int id, int ntasks, std::vector<std::vector<int>>* C, std::vector<std::vector<int>>* A, std::vector<std::vector<int>>* B, int x, int y)
        : id(id), ntasks(ntasks), C(C), A(A), B(B), x(x), y(y) {}

    void execute() {
        auto& matrixA = *A;
        auto& matrixB = *B;
        auto& matrixC = *C;
        matrixC[x][y] = 0;
        for (int k = 0; k < matrixA[0].size(); ++k) {
          matrixC[x][y] += matrixA[x][k] * matrixB[y][k];
        }
    }
};
int main() {
  int p, q, r,times;
  std::cout << "Enter dimensions of matrix A (p x q), and B(q x r) as \"p q r \" (p,q,r < 100): ";
  std::cin >> p >> q >> r;
  std::cout <<"How many times to run (to test avg times), only \'1\' for matrix output: ";
  std::cin >> times;
  if (p > 100 || q > 100 || r > 100) {
      std::cerr << "Dimensions should be less than 100." << std::endl;
      return 1;
  }
  if (times < 1) {
      std::cerr << "Times should be greater than 0." << std::endl;
      return 1;
  }
  srand(time(0));
  std::vector<double>time_spans_mat(times);
  std::vector<double>time_spans_mat_transpose(times);
  std::vector<double>time_spans_pthreads(times);
  std::vector<std::vector<int>> A(p, std::vector<int>(q));
  std::vector<std::vector<int>> B(q, std::vector<int>(r));
  std::vector<std::vector<int>> C(p, std::vector<int>(r, 0));
  for(int i = 0; i < times; ++i) {
    //random init of matrix
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
    // std::vector<std::vector<int>> C(p, std::vector<int>(r, 0));
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
    //test pthreads
    std::vector<Task> tasks;
    std::vector<std::vector<int>> C_pthreads(p, std::vector<int>(r, 0));
    std::vector<pthread_t> p_threads(p * r);
    // pthread_barrier_t barrier;
    // pthread_barrier_init(&barrier, NULL, p * r);
    high_resolution_clock::time_point begin_pthreads = high_resolution_clock::now();
    //task assignment
    for (int i = 0; i < p; ++i) {
        for (int j = 0; j < r; ++j) {
            tasks.emplace_back(i * r + j, p * r, &C_pthreads, &A, &B_transposed, i, j);
        }
    }
    for(int i = 0; i < tasks.size(); ++i) {
        pthread_t thread;
        int status = ::pthread_create(&p_threads[i], nullptr, Task::runner, &tasks[i]);
        if (status != 0) {
            ::perror("thread create");
            return 1;
        }
    }
    // wait for pthread finish
    for(int i = 0; i < tasks.size(); ++i) {
        pthread_join(p_threads[i], nullptr);
    }
    // end timing
    auto time_span_pthreads = duration_cast<duration<double>>(high_resolution_clock::now() - begin_pthreads);
    //check for errors
    for(int i = 0; i < p; ++i) {
      for(int j = 0; j < r; ++j) {
        if(C[i][j] != C_pthreads[i][j]) {
          std::cout << "Error in pthread matrix multiplication at C[" << i << "][" << j << "]" << std::endl;
          return 1;
        }
      }
    }
    //store times
    time_spans_mat[i] = time_span.count();
    time_spans_mat_transpose[i] = time_span_mat_transpose.count();
    time_spans_pthreads[i] = time_span_pthreads.count();
  }
  if(times == 1) {
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
  }
    //calculate average times
    double avg_time_mat = std::accumulate(time_spans_mat.begin(), time_spans_mat.end(), 0.0) / times;
    double avg_time_mat_transpose = std::accumulate(time_spans_mat_transpose.begin(), time_spans_mat_transpose.end(), 0.0) / times;
    double avg_time_pthreads = std::accumulate(time_spans_pthreads.begin(), time_spans_pthreads.end(), 0.0) / times;
    //output timings
    std::cout << "Time taken for matrix multiplication (only threads): " << avg_time_mat << " seconds" << std::endl;
    // std::cout << "Time taken for matrix transpose: " << avg_time_mat_transpose.count() << " seconds" << std::endl;
    std::cout << "Time taken for matrix multiplication using transposed B: " << avg_time_mat_transpose<< " seconds" << std::endl;
    std::cout << "Speed up: " << avg_time_mat - avg_time_mat_transpose<< " seconds, or " <<avg_time_mat/avg_time_mat_transpose*100 << "%" << std::endl << std::endl;
    std::cout << "Time taken for pthreads: " << avg_time_pthreads << " seconds" << std::endl;
    std::cout << "Speed up(compared to transpose cause i transpoed the pthread): " << avg_time_mat_transpose - avg_time_pthreads << " seconds, or " <<avg_time_mat_transpose/avg_time_pthreads*100 << "%" << std::endl;

  return 0;
}