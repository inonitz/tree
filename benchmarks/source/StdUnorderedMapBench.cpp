#include <benchmark/benchmark.h>
#include <util2/C/base_type.h>
#include <unordered_map>
#include <vector>
#include <random>
#include <string>


struct DummyRecord 
{
    DummyRecord() : m_id{0} {}
    DummyRecord(u64 id) : m_id{id} {}

    bool operator<(const DummyRecord& other) const {
        return m_id < other.m_id;
    }
    bool operator==(const DummyRecord& other) const {
        return m_id == other.m_id;
    }

    u64 id() const { return m_id; }

private:
    u64 m_id;
    double   m_values[8]{0};
    char     m_metadata[32]{0};
};

// Inject hash specialization for DummyRecord into std namespace
namespace std {
    template <>
    struct hash<DummyRecord> {
        size_t operator()(const DummyRecord& r) const {
            return std::hash<u64>{}(r.id());
        }
    };
}


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
static void BM_StdUnorderedMapBenchInsertion(benchmark::State& state) {
    const u64 N = state.range(0);
    std::unordered_map<T, T> map;
    T valToInsert;
    bool inserted = false;
    u64 insertStatus[2] = { 0, 0 };

    for (auto _ : state) {
        state.PauseTiming();
        valToInsert = DataGen<T>::random_val();
        insertStatus[inserted]++;
        state.ResumeTiming();
        
        // emplace returns a pair<iterator, bool>
        auto result = map.emplace(valToInsert, valToInsert);
        inserted = result.second;
        benchmark::DoNotOptimize(result);
    }

    state.counters["Failure"] = benchmark::Counter(static_cast<double>(insertStatus[0]));
    state.counters["Success"] = benchmark::Counter(static_cast<double>(insertStatus[1]));
    state.SetBytesProcessed(int64_t(state.iterations()) * sizeof(T) * 2);
    state.SetComplexityN(N);
    return;
}


template <typename T> 
static void BM_StdUnorderedMapBenchDeletion(benchmark::State& state) {
    const u64 N = state.range(0);
    std::mt19937 gen(0);
    std::vector<T> original_data, working_set;
    std::unordered_map<T, T> map;

    generateUniqueVectorSet(original_data, N);
    
    for (auto _ : state) {
        state.PauseTiming();
        if (working_set.empty()) {
            map.clear();
            working_set = original_data;
            std::shuffle(working_set.begin(), working_set.end(), gen);
            for(auto& val : working_set) map.emplace(val, val);
        }
        T valToDelete = working_set.back();
        working_set.pop_back();
        state.ResumeTiming();

        benchmark::DoNotOptimize(map.erase(valToDelete));
    }

    state.SetBytesProcessed(int64_t(state.iterations()) * sizeof(T));
    state.SetComplexityN(N);
    return;
}


template <typename T> 
static void BM_StdUnorderedMapBenchSearch(benchmark::State& state) {
    const uint32_t N = state.range(0);
    std::unordered_map<T, T> map;
    std::vector<T> testVec;
    
    generateUniqueVectorSet(testVec, N);
    for(auto& elem : testVec) {
        map.emplace(elem, elem);
    }

    uint32_t i = 0;
    for (auto _ : state) {
        state.PauseTiming();
        const T& valToSearch = testVec[i % N];
        ++i;
        state.ResumeTiming();
        
        benchmark::DoNotOptimize(map.find(valToSearch));
    }

    state.SetBytesProcessed(int64_t(state.iterations()) * sizeof(T));
    state.SetComplexityN(N);
    return;
}




#define REGISTER_TYPED_MAP_BENCH(T) \
    BENCHMARK_TEMPLATE(BM_StdUnorderedMapBenchInsertion, T)->RangeMultiplier(2)->Range(1<<10, 1<<22)->Repetitions(2)->DisplayAggregatesOnly(true)->Complexity(); \
    BENCHMARK_TEMPLATE(BM_StdUnorderedMapBenchDeletion, T)->RangeMultiplier(2)->Range(1<<10, 1<<22)->Repetitions(2)->DisplayAggregatesOnly(true)->Complexity(); \
    BENCHMARK_TEMPLATE(BM_StdUnorderedMapBenchSearch, T)->RangeMultiplier(2)->Range(1<<10, 1<<22)->Repetitions(2)->DisplayAggregatesOnly(true)->Complexity();

REGISTER_TYPED_MAP_BENCH(u64)
REGISTER_TYPED_MAP_BENCH(std::string)
REGISTER_TYPED_MAP_BENCH(DummyRecord)
