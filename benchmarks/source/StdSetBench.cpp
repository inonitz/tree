#include <benchmark/benchmark.h>
#include <util2/C/base_type.h>
#include <random>
#include <vector>
#include <set>


struct DummyRecord {
    DummyRecord() : m_id{0} {}
    DummyRecord(u64 id) : m_id{id} {}

    bool operator<(const DummyRecord& other) const { return m_id < other.m_id; }
    bool operator>(const DummyRecord& other) const { return m_id > other.m_id; }
    bool operator==(const DummyRecord& other) const { return m_id == other.m_id; }

private:
    u64 m_id;
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
// Benchmarks
// ----------------------------------------------------------------------------
template <typename T> 
static void BM_StdSetInsertion(benchmark::State& state) {
    const u64 N = state.range(0);
    std::set<T> tree;
    T valToInsert;
    
    // Track success/failure similarly to the original code
    u64 success = 0;
    u64 failure = 0;
    for (auto _ : state) {
        state.PauseTiming();

        valToInsert = DataGen<T>::random_val();
        auto result = tree.insert(valToInsert);
        success += (result.second) ? 1 : 0;
        failure += !(result.second) ? 1 : 0;
        state.ResumeTiming();
        
        benchmark::DoNotOptimize(result);
    }

    state.counters["Failure"] = benchmark::Counter(static_cast<double>(failure));
    state.counters["Success"] = benchmark::Counter(static_cast<double>(success));
    state.SetBytesProcessed(int64_t(state.iterations()) * sizeof(T));
    state.SetComplexityN(N);
    return;
}


template <typename T> 
static void BM_StdSetDeletion(benchmark::State& state) {
    const u64 N = state.range(0);
    std::mt19937 gen(0);
    std::vector<T> original_data, working_set;
    std::set<T> tree;

    generateUniqueVectorSet(original_data, N);
    
    for (auto _ : state) {
        state.PauseTiming();
        // If the working set is empty, refill the set and the vector
        if (working_set.empty()) {
            tree.clear();
            working_set = original_data;
            std::shuffle(working_set.begin(), working_set.end(), gen);
            for(auto& val : working_set) tree.insert(val);
        }
        T valToDelete = working_set.back();
        working_set.pop_back();
        state.ResumeTiming();

        // std::set::erase returns 1 if element was found and removed
        benchmark::DoNotOptimize(tree.erase(valToDelete));
    }

    state.SetBytesProcessed(int64_t(state.iterations()) * sizeof(T));
    state.SetComplexityN(N);
    return;
}


template <typename T> 
static void BM_StdSetSearch(benchmark::State& state) {
    const uint32_t N = state.range(0);
    std::set<T> tree;
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
        
        benchmark::DoNotOptimize(tree.find(valToSearch));
    }

    state.SetBytesProcessed(int64_t(state.iterations()) * sizeof(T));
    state.SetComplexityN(N);
    return;
}


#define REGISTER_TYPED_SET_BENCH(T) \
    BENCHMARK_TEMPLATE(BM_StdSetInsertion, T)->RangeMultiplier(4)->Range(1<<10, 1<<22)->Repetitions(2)->DisplayAggregatesOnly(true)->Complexity(); \
    BENCHMARK_TEMPLATE(BM_StdSetDeletion, T)->RangeMultiplier(4)->Range(1<<10, 1<<22)->Repetitions(2)->DisplayAggregatesOnly(true)->Complexity(); \
    BENCHMARK_TEMPLATE(BM_StdSetSearch, T)->RangeMultiplier(4)->Range(1<<10, 1<<22)->Repetitions(2)->DisplayAggregatesOnly(true)->Complexity();


REGISTER_TYPED_SET_BENCH(u64)
REGISTER_TYPED_SET_BENCH(std::string)
REGISTER_TYPED_SET_BENCH(DummyRecord)