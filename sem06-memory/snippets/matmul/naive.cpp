#include <iostream>
#include <chrono>
#include <vector>

template <int N>
using matrix = std::array<std::array<int, N>, N>;

template <int N>
void naive(matrix<N>& m1, matrix<N>& m2, matrix<N>& res) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            for (int k = 0; k < N; ++k) {
                res[i][j] = res[i][j] + m1[i][k] * m2[k][j];
            }
        }
    }
}

template <int N>
void fast(matrix<N>& m1, matrix<N>& m2, matrix<N>& res) {
    matrix<N> m2T;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            m2T[i][j] = m2[j][i];
        }
    }
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            for (int k = 0; k < N; ++k) {
                res[i][j] = res[i][j] + m1[i][k] * m2T[j][k];
            }
        }
    }
}

int main() {
    constexpr int N = 700;
    matrix<N> m1, m2, res;

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            m1[i][j] = rand();
            m2[i][j] = rand();
            res[i][j] = 0;
        }
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    fast<N>(m1, m2, res);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    uint64_t cnt = 0;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            cnt += res[i][j];
        }
    }
    std::cout << cnt << std::endl;

    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[µs]" << std::endl;

}