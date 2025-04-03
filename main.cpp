// Take parameters: p, q and r, each < 100
// Each element of the matrix is a random integers from 0 … 9
// Create matrix, A, of dimension p x q
// Create matrix, B, of dimension q x r
// Compute C = A x B
//multithread it
/*Each element of C should be computed in a thread of it’s own. The program should first create the number of threads needed for the computation. Note that the dimension of C is p x r.
The main thread will need to send the row and column necessary to compute a particular entry of the result. The result matrix should be updated with the value returned by each thread.
The program should print the input matrices and the result matrix in a readable format when it finishes.*/

#include <iostream>
#include <vector>
#include <thread>

int calc_matrix_mult(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, int i, int j) {
    int sum = 0;
    for (int k = 0; k < A[0].size(); ++k) {
        sum += A[i][k] * B[k][j];
    }
    return sum;
}



int main() {
    int p, q, r;
    std::cout << "Enter dimensions of matrix A (p x q), and B(q x r) as \"p q r \": ";
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

    std::vector<std::thread> threads;
    std::vector<std::vector<int>> C(p, std::vector<int>(r, 0));
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

    return 0;
}