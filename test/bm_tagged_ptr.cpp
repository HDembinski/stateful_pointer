#include "stateful_pointer/tagged_ptr.hpp"
#include "benchmark/benchmark.h"
#include "array"
#include "memory"

namespace sp = stateful_pointer;

template <typename T>
static void unique_ptr_creation(benchmark::State& state) {
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(std::unique_ptr<T>(new T()));
    }
}

template <typename T>
static void tagged_ptr_creation(benchmark::State& state) {
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(sp::make_tagged<T, 4>());
    }
}

template <typename T>
static void unique_ptr_access(benchmark::State& state) {
    auto p = std::unique_ptr<T>(new T());
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(p.get());
    }
}

template <typename T>
static void tagged_ptr_access(benchmark::State& state) {
    auto p = sp::make_tagged<T, 4>();
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(p.get());
    }
}

BENCHMARK_TEMPLATE(unique_ptr_creation, char);
BENCHMARK_TEMPLATE(tagged_ptr_creation, char);
BENCHMARK_TEMPLATE(unique_ptr_creation, std::array<char, 256>);
BENCHMARK_TEMPLATE(tagged_ptr_creation, std::array<char, 256>);
BENCHMARK_TEMPLATE(unique_ptr_access, char);
BENCHMARK_TEMPLATE(tagged_ptr_access, char);
BENCHMARK_TEMPLATE(unique_ptr_access, std::array<char, 256>);
BENCHMARK_TEMPLATE(tagged_ptr_access, std::array<char, 256>);

BENCHMARK_MAIN();
