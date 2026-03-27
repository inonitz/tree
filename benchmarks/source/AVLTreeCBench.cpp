#include <benchmark/benchmark.h>
#include <util2/C/base_type.h>
#include <util2/C/macro.h>
#include <tree/C/avl_tree.h>
#include <vector>
#include <random>


struct DummyRecord {
    DummyRecord() : m_id{0} {}
    DummyRecord(u64 id) : m_id{id} {}

    bool operator<(const DummyRecord& other) const { return m_id < other.m_id; }
    bool operator>(const DummyRecord& other) const { return m_id > other.m_id; }
    bool operator==(const DummyRecord& other) const { return m_id == other.m_id; }

private:
    u64 m_id;
    double m_values[8]{0};
    char   m_metadata[32]{0};
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
            return T{static_cast<u64>(i)};
        }
    }


    static T random_val() {
        static std::mt19937 gen(std::random_device{}());


        if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> dist(0, 1e9);
            return dist(gen);
        } else if constexpr (std::is_integral_v<T>) {
            std::uniform_int_distribution<u64> dist(0, UINT64_MAX);
            return static_cast<T>(dist(gen));
        } else {
            std::uniform_int_distribution<u64> dist(0, UINT64_MAX);
            return make(dist(gen));
        }
    }
};


template <typename T>
static void generateUniqueVectorSet(std::vector<T>& vec, size_t size) {
    static std::mt19937 generator(std::random_device{}());
    vec.clear();
    vec.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        vec.push_back(DataGen<T>::make(i));
    }
    std::shuffle(vec.begin(), vec.end(), generator);
}


// ----------------------------------------------------------------------------
// C++ Wrapper for the C AVL Tree Implementation
// ----------------------------------------------------------------------------
template <typename T>
static int GenericComparator(const void* a, const void* b) {
    const T& arg1 = *__scast(const T*, a);
    const T& arg2 = *__scast(const T*, b);
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

template <typename T>
class C_AVLTreeWrapper {
private:
    AVLTree m_tree;

public:
    C_AVLTreeWrapper() {
        AVLTreeCreate(&m_tree, (binaryTreeComparatorFunc)GenericComparator<T>, sizeof(T));
    }

    ~C_AVLTreeWrapper() {
        AVLTreeDestroy(&m_tree);
    }

    bool insert(const T& val) {
        T temp = val; // Create mutable copy if the C API does not use const void*
        // Assumes binaryTreeResult_t resolves to a success integer/enum (e.g., 0 or 1)
        return AVLTreeInsert(&m_tree, &temp) == BINARY_TREE_OP_SUCCESS; 
    }

    bool remove(const T& val) {
        T temp = val;
        return AVLTreeRemove(&m_tree, &temp) == BINARY_TREE_OP_SUCCESS;
    }

    bool search(const T& val) const {
        T temp = val;
        return AVLTreeSearch(&m_tree, &temp);
    }

    void clear() {
        AVLTreeDestroy(&m_tree);
        AVLTreeCreate(&m_tree, (binaryTreeComparatorFunc)GenericComparator<T>, sizeof(T));
        return;
    }
};




// ----------------------------------------------------------------------------
// Benchmarks
// ----------------------------------------------------------------------------
template <typename T> 
static void BM_AVLTreeCBenchInsertion(benchmark::State& state) {
    const u64 N = state.range(0);
    C_AVLTreeWrapper<T> tree;
    T valToInsert;
    bool status = false;
    u64 insertStatus[2] = { 0, 0 };

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
    state.SetBytesProcessed(int64_t(state.iterations()) * sizeof(T));
    state.SetComplexityN(N);
    return;
}


template <typename T> 
static void BM_AVLTreeCBenchDeletion(benchmark::State& state) {
    const u64 N = state.range(0);
    bool status = false;
    std::mt19937 gen(0);
    std::vector<T> original_data, working_set;
    C_AVLTreeWrapper<T> tree;
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

    state.SetBytesProcessed(int64_t(state.iterations()) * sizeof(T));
    state.SetComplexityN(N);
    return;
}


template <typename T> 
static void BM_AVLTreeCBenchSearch(benchmark::State& state) {
    const uint32_t N = state.range(0);
    C_AVLTreeWrapper<T> tree;
    std::vector<T> dataSet;
    
    generateUniqueVectorSet(dataSet, N);
    for(auto& elem : dataSet) {
        tree.insert(elem);
    }

    uint32_t i = 0;
    for (auto _ : state) {
        state.PauseTiming();
        const T& valToSearch = dataSet[i % N];
        ++i;
        state.ResumeTiming();
        
        benchmark::DoNotOptimize(tree.search(valToSearch));
    }

    state.SetBytesProcessed(int64_t(state.iterations()) * sizeof(T));
    state.SetComplexityN(N);
    return;
}


#define REGISTER_TYPED_AVL_TREE_C_BENCH(T) \
    BENCHMARK_TEMPLATE(BM_AVLTreeCBenchInsertion, T)->RangeMultiplier(2)->Range(1<<10, 1<<22)->Repetitions(2)->DisplayAggregatesOnly(true)->Complexity(); \
    BENCHMARK_TEMPLATE(BM_AVLTreeCBenchDeletion, T)->RangeMultiplier(2)->Range(1<<10, 1<<22)->Repetitions(2)->DisplayAggregatesOnly(true)->Complexity(); \
    BENCHMARK_TEMPLATE(BM_AVLTreeCBenchSearch, T)->RangeMultiplier(2)->Range(1<<10, 1<<22)->Repetitions(2)->DisplayAggregatesOnly(true)->Complexity();

REGISTER_TYPED_AVL_TREE_C_BENCH(u64)
REGISTER_TYPED_AVL_TREE_C_BENCH(std::string)
REGISTER_TYPED_AVL_TREE_C_BENCH(DummyRecord)