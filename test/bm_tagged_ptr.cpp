#include "stateful_pointer/tagged_ptr.hpp"
#include "benchmark/benchmark.h"
#include "string"
#include "memory"

namespace sp = stateful_pointer;

struct Small {
    char x = 1;
};

struct Large {
    std::string x = "abcdefghijklmnopqrstuvwxyz";
};

template <typename T>
static void unique_ptr_creation(benchmark::State& state) {
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(std::unique_ptr<T>(new T()));
    }
}

template <typename T>
static void tagged_ptr_creation(benchmark::State& state) {
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(sp::make_tagged_ptr<T, 4>());
    }
}

template <typename T>
static void unique_ptr_access(benchmark::State& state) {
    auto p = std::unique_ptr<T>(new T());
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(p->x);
        benchmark::ClobberMemory();
    }
}

template <typename T>
static void tagged_ptr_access(benchmark::State& state) {
    auto p = sp::make_tagged_ptr<T, 4>();
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(p->x);
        benchmark::ClobberMemory();
    }
}

BENCHMARK_TEMPLATE(unique_ptr_creation, Small);
BENCHMARK_TEMPLATE(tagged_ptr_creation, Small);
BENCHMARK_TEMPLATE(unique_ptr_creation, Large);
BENCHMARK_TEMPLATE(tagged_ptr_creation, Large);
BENCHMARK_TEMPLATE(unique_ptr_access, Small);
BENCHMARK_TEMPLATE(tagged_ptr_access, Small);
BENCHMARK_TEMPLATE(unique_ptr_access, Large);
BENCHMARK_TEMPLATE(tagged_ptr_access, Large);

BENCHMARK_MAIN();
