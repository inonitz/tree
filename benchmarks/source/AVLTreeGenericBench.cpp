#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>
#include <string>
#include <cstring>
#include <util2/C/macro.h>
#include <util2/C/base_type.h>
#include <tree/AVLTreeImpl.hpp>


struct DummyRecord 
{
    DummyRecord() : m_id{DEFAULT64} {}
    DummyRecord(uint64_t id) : m_id{id} {}

    bool operator<(const DummyRecord& other) const {
        return m_id < other.m_id;
    }
    bool operator>(const DummyRecord& other) const {
        return m_id > other.m_id;
    }
    bool operator==(const DummyRecord& other) const {
        return m_id == other.m_id;
    }

private:
    uint64_t m_id;
    double   m_values[8]{0};
    char     m_metadata[32]{0};
};



template <typename T>
struct DataGen {
    static T make(size_t i) {
        if constexpr (std::is_arithmetic_v<T>) {
            return static_cast<T>(i);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return std::to_string(i);
        } else {
            // Assume DummyRecord has a constructor taking a numeric value
            return T{static_cast<uint64_t>(i)};
        }
    }


    static T random_val() {
        static std::mt19937 gen(std::random_device{}());


        if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> dist(0, 1e9);
            return dist(gen);
        } else if constexpr (std::is_integral_v<T>) {
            std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
            return static_cast<T>(dist(gen));
        } else {
            std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
            return make(dist(gen));
        }
    }
};


template <typename T>
void generateUniqueVectorSet(std::vector<T>& vec, size_t size) {
    static std::mt19937 generator(42);
    vec.clear();
    vec.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        vec.push_back(DataGen<T>::make(i));
    }
    std::shuffle(vec.begin(), vec.end(), generator);
    return;
}


template <typename T> static void BM_TreeInsertion_SeemsSimple(benchmark::State& state) {
    const uint64_t N = state.range(0);
    AVLTree<T> tree;
    T valToInsert;
    bool status = false;
    uint64_t insertStatus[2] = { 0, 0 };


    for (auto _ : state) {
        state.PauseTiming();
        valToInsert = DataGen<T>::random_val();
        ++insertStatus[status];
        state.ResumeTiming();
        
        benchmark::DoNotOptimize(status = tree.insert(valToInsert));
    }

    --insertStatus[0];
    state.counters["Failure"] = benchmark::Counter(static_cast<double>(insertStatus[0]));
    state.counters["Success"] = benchmark::Counter(static_cast<double>(insertStatus[1]));
    state.SetBytesProcessed(int64_t(state.range(0)) * sizeof(T));
    state.SetComplexityN(N);
    return;
}


template <typename T> static void BM_TreeDeletion_UniqueTail(benchmark::State& state) {
    const uint64_t N = state.range(0);
    bool status = false;
    std::mt19937 gen(0);
    std::vector<T> original_data, working_set;
    AVLTree<T> tree;
    T valToDelete{};

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


    state.SetBytesProcessed(int64_t(state.range(0)) * sizeof(T));
    state.SetComplexityN(N);
    return;
}


template <typename T> static void BM_TreeDeletion_Rev1(benchmark::State& state) {
    const uint64_t N = state.range(0);
    std::mt19937 gen(0);
    std::vector<T> original_data, working_set;
    AVLTree<T> tree;

    generateUniqueVectorSet(original_data, N);
    working_set.reserve(N);

    for (auto _ : state) {
        state.PauseTiming();
        if (working_set.empty()) {
            tree.clear();
            working_set = original_data;
            std::shuffle(working_set.begin(), working_set.end(), gen);
            for(auto& val : working_set) tree.insert(val);
        }
        T valToDelete = working_set.back();
        working_set.pop_back();
        state.ResumeTiming();

        benchmark::DoNotOptimize(tree.remove(valToDelete));
    }


    state.SetBytesProcessed(int64_t(state.range(0)) * sizeof(T));
    state.SetComplexityN(N);
    return;
}


template <typename T> static void BM_TreeSearch(benchmark::State& state) {
    const uint32_t N = state.range(0);
    AVLTree<T> testTree;
    std::vector<T> testVec;
    

    generateUniqueVectorSet(testVec, N);
    for(auto& elem : testVec) {
        testTree.insert(elem);
    }


    uint32_t i = 0;
    for (auto _ : state) {
        state.PauseTiming();
        const T& valToSearch = testVec[i % N];
        ++i;
        state.ResumeTiming();
        
        benchmark::DoNotOptimize(testTree.search(valToSearch));
    }


    state.SetBytesProcessed(int64_t(state.range(0)) * sizeof(T));
    state.SetComplexityN(N);
    return;
}


#define REGISTER_TYPED_AVL_TREE_BENCH(T) \
    BENCHMARK_TEMPLATE(BM_TreeInsertion_SeemsSimple, T)->RangeMultiplier(2)->Range(1<<10, 1<<22)->Repetitions(2)->DisplayAggregatesOnly(true)->Complexity(); \
    BENCHMARK_TEMPLATE(BM_TreeDeletion_UniqueTail, T)->RangeMultiplier(2)->Range(1<<10, 1<<22)->Repetitions(2)->Complexity(); \
    BENCHMARK_TEMPLATE(BM_TreeSearch, T)->RangeMultiplier(2)->Range(1<<10, 1<<22)->Repetitions(2)->Complexity();


REGISTER_TYPED_AVL_TREE_BENCH(u64)
REGISTER_TYPED_AVL_TREE_BENCH(u32)
// REGISTER_TYPED_AVL_TREE_BENCH(u16)
// REGISTER_TYPED_AVL_TREE_BENCH(u8)
// REGISTER_TYPED_AVL_TREE_BENCH(int64_t)
// REGISTER_TYPED_AVL_TREE_BENCH(int32_t)
// REGISTER_TYPED_AVL_TREE_BENCH(int16_t)
// REGISTER_TYPED_AVL_TREE_BENCH(int8_t)
// REGISTER_TYPED_AVL_TREE_BENCH(float)
REGISTER_TYPED_AVL_TREE_BENCH(double)
REGISTER_TYPED_AVL_TREE_BENCH(std::string)
REGISTER_TYPED_AVL_TREE_BENCH(DummyRecord)