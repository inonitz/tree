#include <benchmark/benchmark.h>
#include <tmp/AVLTreeDraft.hpp>
#include <util2/C/base_type.h>
#include <numeric>
#include <vector>
#include <random>
#include <algorithm>


static uint32_t generateRandomNumber() {
    thread_local std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);

    return dist(gen);
}


static void constructRandomTree(
    AVLTree& out,
    std::vector<uint32_t>& outSet,
    uint32_t nodeCount
) {
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> val_dist(0, 100000);


    uint32_t      seed = rd();
    uint32_t      val = 0;
    for(uint32_t i = 0; i < nodeCount; ++i) {
        val = val_dist(gen);
        if(out.search(val) == true) {
            continue;
        }
        out.insert(val);
        outSet.push_back(val);
    }

    return;
}


static void generateUniqueVectorSet(std::vector<uint32_t>& vec, size_t size) {
    static std::mt19937 generator(std::random_device{}());

    vec.resize(size);
    std::iota(vec.begin(), vec.end(), 0);
    std::shuffle(vec.begin(), vec.end(), generator);
    return;
}


static void generateIndexBufferFromVector(
    std::vector<uint32_t> const& vecIn,
    std::vector<uint32_t>&       vecOut
) {
    u64 i = 0;

    vecOut.reserve(vecIn.size());
    for(auto& val : vecIn) {
        vecOut.push_back(i);
        ++i;
    }


    return;
}




// ----------------------------------------------------------------------------
// Benchmarks
// ----------------------------------------------------------------------------
static void BM_AVLTreeBenchInsertion(benchmark::State& state) {
    const u64 N = state.range(0);
    AVLTree tree;
    uint32_t valToInsert;
    bool status = false;
    u64 insertStatus[2] = { 0, 0 };

    for (auto _ : state) {
        state.PauseTiming();
        valToInsert = generateRandomNumber();
        ++insertStatus[status];
        state.ResumeTiming();

        benchmark::DoNotOptimize(status = tree.insert(valToInsert));
    }

    --insertStatus[0];
    state.counters["Failure"] = benchmark::Counter(static_cast<double>(insertStatus[0]));
    state.counters["Success"] = benchmark::Counter(static_cast<double>(insertStatus[1]));
    state.SetBytesProcessed(int64_t(state.iterations()) * sizeof(uint32_t));
    state.SetComplexityN(N);
    return;
}


static void BM_AVLTreeBenchDeletion(benchmark::State& state) {
    const u64 N = state.range(0);
    bool status = false;
    std::mt19937 gen(0);
    std::vector<uint32_t> original_data, working_set;
    AVLTree tree;
    uint32_t valToDelete{};

    generateUniqueVectorSet(original_data, N);

    for (auto _ : state) {
        state.PauseTiming();

        if (working_set.empty()) {
            tree.clear();
            working_set = original_data;
            std::shuffle(working_set.begin(), working_set.end(), gen);
            for(auto& val : working_set) tree.insert(val);
        }
        valToDelete = working_set.back();
        working_set.pop_back();

        state.ResumeTiming();

        benchmark::DoNotOptimize(status = tree.remove(valToDelete));
    }

    state.SetBytesProcessed(int64_t(state.iterations()) * sizeof(uint32_t));
    state.SetComplexityN(N);
    return;
}


static void BM_AVLTreeBenchSearch(benchmark::State& state) {
    const uint32_t N = state.range(0);
    std::vector<uint32_t> dataSet;
    AVLTree tree;

    generateUniqueVectorSet(dataSet, N);
    for(auto& elem : dataSet) {
        tree.insert(elem);
    }

    uint32_t i = 0;
    for (auto _ : state) {
        state.PauseTiming();
        const uint32_t& valToSearch = dataSet[i % N];
        ++i;
        state.ResumeTiming();

        benchmark::DoNotOptimize(tree.search(valToSearch));
    }

    state.SetBytesProcessed(int64_t(state.iterations()) * sizeof(uint32_t));
    state.SetComplexityN(N);
    return;
}



#define REGISTER_AVL_TREE_BENCH() \
    BENCHMARK(BM_AVLTreeBenchInsertion)->RangeMultiplier(2)->Range(1<<10, 1<<22)->Repetitions(2)->DisplayAggregatesOnly(true)->Complexity(); \
    BENCHMARK(BM_AVLTreeBenchDeletion)->RangeMultiplier(2)->Range(1<<10, 1<<22)->Repetitions(2)->DisplayAggregatesOnly(true)->Complexity(); \
    BENCHMARK(BM_AVLTreeBenchSearch)->RangeMultiplier(2)->Range(1<<10, 1<<22)->Repetitions(2)->DisplayAggregatesOnly(true)->Complexity();


REGISTER_AVL_TREE_BENCH()
