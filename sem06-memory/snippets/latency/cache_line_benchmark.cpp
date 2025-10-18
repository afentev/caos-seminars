#include <string>
#include <chrono>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
//#include <immintrin.h>
#include "alligned_allocator.hpp"

// Prevent optimization
static void escape(void *p) {
    asm volatile("" : : "g"(p) : "memory");
}

static void clobber() {
    asm volatile("" : : : "memory");
}

// Cache line size (typically 64 bytes on modern systems)
constexpr size_t CACHE_LINE_SIZE = 64;
constexpr size_t INTS_PER_CACHE_LINE = CACHE_LINE_SIZE / sizeof(int);

// 1. Varying stride within cache line
void benchmark_stride(int* memory, size_t size, int stride, int iterations) {
    for (int iter = 0; iter < iterations; iter++) {
        for (size_t i = 0; i < size; i += stride) {
            memory[i]++;
        }
        clobber();
    }
    escape(memory);
}

// 2. Access every element in cache line before moving to next
void benchmark_cache_line_fully(int* memory, size_t size, int iterations) {
    size_t elements = size / sizeof(int);
    for (int iter = 0; iter < iterations; iter++) {
        for (size_t block = 0; block < elements; block += INTS_PER_CACHE_LINE) {
            for (size_t offset = 0; offset < INTS_PER_CACHE_LINE && (block + offset) < elements; offset++) {
                memory[block + offset]++;
            }
        }
        clobber();
    }
    escape(memory);
}

// 3. Access only one element per cache line
void benchmark_one_per_cache_line(int* memory, size_t size, int iterations) {
    size_t elements = size / sizeof(int);
    for (int iter = 0; iter < iterations; iter++) {
        for (size_t i = 0; i < elements; i += INTS_PER_CACHE_LINE) {
            memory[i]++;
        }
        clobber();
    }
    escape(memory);
}

// 4. Random access within cache lines
void benchmark_random_within_cache_line(int* memory, size_t size, int iterations, int* offsets) {
    size_t elements = size / sizeof(int);
    size_t cache_lines = elements / INTS_PER_CACHE_LINE;
    
    for (int iter = 0; iter < iterations; iter++) {
        for (size_t line = 0; line < cache_lines; line++) {
            size_t base = line * INTS_PER_CACHE_LINE;
            int offset = offsets[line % INTS_PER_CACHE_LINE];
            if (base + offset < elements) {
                memory[base + offset]++;
            }
        }
        clobber();
    }
    escape(memory);
}

// 5. False sharing scenario - multiple threads would contend on these
void benchmark_false_sharing_pattern(int* memory, size_t size, int iterations) {
    // Simulate false sharing by accessing same cache line from different "thread" positions
    size_t elements = size / sizeof(int);
    constexpr int false_sharing_positions = 4; // Like 4 threads accessing same cache line
    
    for (int iter = 0; iter < iterations; iter++) {
        for (size_t line = 0; line < elements / INTS_PER_CACHE_LINE; line++) {
            for (int thread = 0; thread < false_sharing_positions; thread++) {
                size_t idx = line * INTS_PER_CACHE_LINE + thread;
                if (idx < elements) {
                    memory[idx]++;
                }
            }
        }
        clobber();
    }
    escape(memory);
}

// // 6. AVX-optimized cache line access
// void benchmark_avx_cache_line(int* memory, size_t size, int iterations) {
//     const __m256i ones = _mm256_set1_epi32(1);
//     size_t elements = size / sizeof(int);
    
//     for (int iter = 0; iter < iterations; iter++) {
//         // Process entire cache lines with AVX (64 bytes = 2x AVX registers)
//         for (size_t i = 0; i + 15 < elements; i += 16) {
//             // First half of cache line
//             __m256i data1 = _mm256_load_si256((__m256i*)&memory[i]);
//             data1 = _mm256_add_epi32(data1, ones);
//             _mm256_store_si256((__m256i*)&memory[i], data1);
            
//             // Second half of cache line  
//             __m256i data2 = _mm256_load_si256((__m256i*)&memory[i + 8]);
//             data2 = _mm256_add_epi32(data2, ones);
//             _mm256_store_si256((__m256i*)&memory[i + 8], data2);
//         }
//         clobber();
//     }
//     escape(memory);
// }

// 7. Your original pattern - varying D parameter
void benchmark_varying_stride(int* memory, size_t size, int stride, int iterations) {
    size_t elements = size / sizeof(int);
    for (int iter = 0; iter < iterations; iter++) {
        for (size_t i = 0; i < elements; i += stride) {
            memory[i]++;
        }
        clobber();
    }
    escape(memory);
}

int main(int argc, char** argv) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <size_bytes> <iterations> <benchmark_type> <parameter>" << std::endl;
        std::cerr << "benchmark_type: stride, full_line, one_per_line, random_line, false_sharing, avx_line, varying_stride" << std::endl;
        std::cerr << "parameter: for stride/varying_stride - stride value, for others - any number" << std::endl;
        return 1;
    }
    
    const size_t size_bytes = std::stoul(argv[1]);
    const int iterations = std::stoi(argv[2]);
    const std::string benchmark_type = argv[3];
    const int parameter = std::stoi(argv[4]);
    
    const size_t elements = size_bytes / sizeof(int);
    
    // Allocate aligned memory
    std::vector<int, aligned_allocator<int, 32>> memory(elements, 0);
    
    // Prepare random offsets for random_within_cache_line
    std::vector<int> random_offsets(INTS_PER_CACHE_LINE);
    for (size_t i = 0; i < INTS_PER_CACHE_LINE; i++) {
        random_offsets[i] = i;
    }
    std::shuffle(random_offsets.begin(), random_offsets.end(), std::default_random_engine(42));
    
    // Ensure alignment for AVX
    if (reinterpret_cast<uintptr_t>(memory.data()) % 32 != 0) {
        std::cerr << "Memory not aligned to 32 bytes!" << std::endl;
        return 1;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    if (benchmark_type == "stride") {
        benchmark_stride(memory.data(), size_bytes, parameter, iterations);
    } else if (benchmark_type == "full_line") {
        benchmark_cache_line_fully(memory.data(), size_bytes, iterations);
    } else if (benchmark_type == "one_per_line") {
        benchmark_one_per_cache_line(memory.data(), size_bytes, iterations);
    } else if (benchmark_type == "random_line") {
        benchmark_random_within_cache_line(memory.data(), size_bytes, iterations, random_offsets.data());
    } else if (benchmark_type == "false_sharing") {
        benchmark_false_sharing_pattern(memory.data(), size_bytes, iterations);
    // } else if (benchmark_type == "avx_line") {
    //     benchmark_avx_cache_line(memory.data(), size_bytes, iterations);
    } else if (benchmark_type == "varying_stride") {
        benchmark_varying_stride(memory.data(), size_bytes, parameter, iterations);
    } else {
        std::cerr << "Unknown benchmark type: " << benchmark_type << std::endl;
        return 1;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    
    // Calculate operations per second
    size_t operations;
    if (benchmark_type == "full_line") {
        operations = elements * iterations;
    } else if (benchmark_type == "one_per_line") {
        operations = (elements / INTS_PER_CACHE_LINE) * iterations;
    } else if (benchmark_type == "random_line") {
        operations = (elements / INTS_PER_CACHE_LINE) * iterations;
    } else if (benchmark_type == "false_sharing") {
        operations = (elements / INTS_PER_CACHE_LINE) * 4 * iterations; // 4 accesses per line
    } else if (benchmark_type == "avx_line") {
        operations = (elements / 16) * 16 * iterations; // 16 elements per AVX iteration
    } else {
        // stride and varying_stride
        operations = (elements / parameter) * iterations;
    }
    
    double ops_per_second = (static_cast<double>(operations) / duration.count()) * 1e9;
    
    std::cout << size_bytes << " " << ops_per_second << std::endl;
    
    return 0;
}