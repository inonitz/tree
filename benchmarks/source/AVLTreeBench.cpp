#include <benchmark/benchmark.h>
#include <cstring>
#include <tmp/AVLTreeDraft.hpp>
#include <vector>
#include <random>
#include <algorithm>
#include <cassert>


uint32_t generateRandomNumber() {
    thread_local std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);

    return dist(gen);
}


void constructRandomTree(
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
}


void generateUniqueVectorSet(std::vector<uint32_t>& vec, size_t size) {
    static std::mt19937 generator(std::random_device{}());
    
    vec.resize(size);
    std::iota(vec.begin(), vec.end(), 0); 
    std::shuffle(vec.begin(), vec.end(), generator);
    return;
}


void generateIndexBufferFromVector(
    std::vector<uint32_t> const& vecIn, 
    std::vector<uint32_t>&       vecOut
) {
    uint64_t i = 0;

    vecOut.reserve(vecIn.size());
    for(auto& val : vecIn) {
        vecOut.push_back(i);
        ++i;
    }


    return;
}




static void BM_TreeInsertion(benchmark::State& state) {
    AVLTree testTree;
    std::vector<uint32_t> testVec;
    std::vector<uint32_t> testVecIndices;
    uint32_t valToInsertIdx = 0;
    uint32_t valToInsert    = 0;

    
    generateUniqueVectorSet(testVec, state.range(0));


    for (auto _ : state) {
        state.PauseTiming();

        testVecIndices.clear();
        testTree.clear();
        generateIndexBufferFromVector(testVec, testVecIndices);
        
        state.ResumeTiming();


        /* Only Inserting Unique Values from a pre-generated buffer, but picking indices randomly */
        for(int j = 0; j < state.range(0); ++j) {
            state.PauseTiming();
            
            valToInsertIdx = generateRandomNumber() % testVecIndices.size();
            valToInsertIdx = (valToInsertIdx == 0) ? 0 : valToInsertIdx - 1;
            valToInsertIdx = testVecIndices[valToInsertIdx];  
            testVecIndices.erase(testVecIndices.begin() + valToInsertIdx);
            
            valToInsert    = testVec[valToInsertIdx];
            state.ResumeTiming();

            benchmark::DoNotOptimize(testTree.insert(valToInsert));
        }
    }

    state.SetComplexityN(state.range(0)); 
    return;
}


static void BM_TreeDeletion(benchmark::State& state) {
    AVLTree testTree;
    std::vector<uint32_t> testVector;
    uint32_t valToRemIdx = 0;
    uint32_t valToRemove = 0;


    for (auto _ : state) {
        state.PauseTiming();

        testVector.clear();
        testTree.clear();
        generateUniqueVectorSet(testVector, state.range(0));
        for(auto& elem : testVector) {
            testTree.insert(elem);
        }

        state.ResumeTiming();


        for(int j = 0; j < state.range(0); ++j) {
            state.PauseTiming();
            valToRemIdx = generateRandomNumber() % testVector.size();
            valToRemIdx = (valToRemIdx == 0) ? 0 : valToRemIdx - 1;
            valToRemove = testVector[valToRemIdx];
            testVector.erase(testVector.begin() + valToRemIdx);
            state.ResumeTiming();

            benchmark::DoNotOptimize(testTree.remove(valToRemove));
        }
    }

    state.SetComplexityN(state.range(0)); 
    return;
}



static void BM_TreeInsertion2(benchmark::State& state) {
    AVLTree testTree;
    std::vector<uint32_t> testVec;
    std::vector<uint32_t> testVecIndices;
    uint32_t valToInsertIdx = 0;
    uint32_t valToInsert    = 0;

    
    generateUniqueVectorSet(testVec, state.range(0));


    for (auto _ : state) {
        state.PauseTiming();

        testVecIndices.clear();
        testTree.clear();
        generateIndexBufferFromVector(testVec, testVecIndices);
        
        state.ResumeTiming();


        /* Only Inserting Unique Values from a pre-generated buffer, but picking indices randomly */
        for(int j = 0; j < state.range(0); ++j) {
            state.PauseTiming();
            
            valToInsertIdx = generateRandomNumber() % testVecIndices.size();
            valToInsertIdx = (valToInsertIdx == 0) ? 0 : valToInsertIdx - 1;
            valToInsertIdx = testVecIndices[valToInsertIdx];  
            testVecIndices.erase(testVecIndices.begin() + valToInsertIdx);
            
            valToInsert    = testVec[valToInsertIdx];
            state.ResumeTiming();

            benchmark::DoNotOptimize(testTree.insert(valToInsert));
        }
    }

    state.SetComplexityN(state.range(0)); 
    return;
}



static void BM_TreeDeletion_Fixed(benchmark::State& state) {
    const uint32_t N = state.range(0);
    // const uint32_t numToDelete = std::max<uint32_t>(1, N / 10); // 10% slice
    const uint32_t numToDelete = 1000;
    printf("10%% of data is %u\n", N / 10);
    std::mt19937          gen(42);
    AVLTree               testTree;
    std::vector<uint32_t> testVec;
    generateUniqueVectorSet(testVec, N);

    for (auto _ : state) {
        state.PauseTiming();
        testTree.clear();
        for(auto& elem : testVec) { 
            testTree.insert(elem); 
        }
        std::shuffle(testVec.begin(), testVec.end(), gen);
        state.ResumeTiming();


        for (uint32_t i = 0; i < numToDelete; ++i) {
            benchmark::DoNotOptimize(testTree.remove(testVec[i]));
        }
        state.SetItemsProcessed(numToDelete);
    }
    
    state.SetComplexityN(N);
    return;
}




static void BM_TreeInsertion_SeemsSimple(benchmark::State& state) {
    const uint64_t N = state.range(0);

    // std::random_device rd;
    // std::mt19937 gen(42);
    // std::uniform_int_distribution<uint32_t> valueDist(0, UINT32_MAX);
    AVLTree  tree;
    uint32_t valToInsert = 0;
    bool     status = false;
    uint32_t insertStatus[2] = { 0, 0 };


    for (auto _ : state) {
        state.PauseTiming();
        
        valToInsert = generateRandomNumber();
        ++insertStatus[status];

        state.ResumeTiming();
        
        
        benchmark::DoNotOptimize(status = tree.insert(valToInsert));
    }

    
    --insertStatus[0];
    state.counters["Failure"] = benchmark::Counter(insertStatus[0]);
    state.counters["Success"] = benchmark::Counter(insertStatus[1]);
    state.counters["Rate"]    = benchmark::Counter(insertStatus[1] + insertStatus[0], benchmark::Counter::kIsRate);
    state.SetComplexityN(N);
    return;
}


static void BM_TreeDeletion_UniqueTail(benchmark::State& state) {
    const uint64_t N = state.range(0);
    bool status = false;
    std::mt19937 gen(0);
    uint32_t i = 0;
    uint32_t valToDelete = 0;
    uint32_t deletionStatus[2] = {0, 0};
    std::vector<uint32_t> original_data, working_set;
    AVLTree tree;


    
    generateUniqueVectorSet(original_data, N);
    for (auto _ : state) {
        state.PauseTiming();

        ++deletionStatus[ status ];
        if (working_set.empty()) {
            tree.clear();
            working_set = original_data;
            std::shuffle(working_set.begin(), working_set.end(), gen);

            for(auto& workingSetValue : working_set) {
                tree.insert(workingSetValue);
            }
        }
        valToDelete = working_set.back();
        working_set.pop_back();
        
        state.ResumeTiming();


        benchmark::DoNotOptimize(status = tree.remove(valToDelete));
    }

    --deletionStatus[0];
    state.counters["DeletionRate"] = benchmark::Counter(deletionStatus[0] + deletionStatus[1], benchmark::Counter::kIsRate);
    state.SetComplexityN(N);
    return;
}


static void BM_TreeDeletion_Rev1(benchmark::State& state) {
    const uint64_t N = state.range(0);
    bool status = false;
    std::mt19937 gen(0);
    uint32_t i = 0;
    uint32_t valToDelete = 0;
    uint32_t deletionStatus[2] = {0, 0};
    std::vector<uint32_t> original_data, working_set;
    AVLTree tree;


    
    generateUniqueVectorSet(original_data, N);
    working_set.reserve(N);
    for (auto _ : state) {
        state.PauseTiming();

        ++deletionStatus[ status ];
        if (working_set.empty()) {
            tree.clear();
            // working_set = original_data;
            working_set.resize(N);
            memcpy(working_set.data(), original_data.data(), sizeof(uint32_t) * original_data.size());
            std::shuffle(working_set.begin(), working_set.end(), gen);

            for(auto& workingSetValue : working_set) {
                tree.insert(workingSetValue);
            }
        }
        valToDelete = working_set.back();
        working_set.pop_back();
        
        state.ResumeTiming();


        benchmark::DoNotOptimize(status = tree.remove(valToDelete));
    }

    --deletionStatus[0];
    state.counters["DeletionRate"] = benchmark::Counter(deletionStatus[0] + deletionStatus[1], benchmark::Counter::kIsRate);
    state.SetComplexityN(N);
    return;
}


static void BM_TreeSearch(benchmark::State& state) {
    AVLTree testTree;
    std::vector<uint32_t> testVec;
    std::vector<uint32_t> testVecIndices;
    uint32_t N           = state.range(0);
    uint32_t i           = 0;
    uint32_t valToSearch = 0;
    

    generateUniqueVectorSet(testVec, N);
    testVecIndices.reserve(N);
    for(i = 0; i < N; ++i) {
        testVecIndices.push_back( generateRandomNumber() % N );
    }
    for(auto& elem : testVec) {
        testTree.insert(elem);
    }


    for (auto _ : state) {
        state.PauseTiming();
        valToSearch = testVec[ testVecIndices[i % N] ];
        ++i;
        state.ResumeTiming();
        benchmark::DoNotOptimize(testTree.search(valToSearch));
    }

    state.SetComplexityN(N);
    return;
}


// BENCHMARK(BM_TreeDeletion)->Range(1<<10, 1<<20)->Complexity();
// BENCHMARK(BM_TreeInsertion_SeemsSimple)
//     ->RangeMultiplier(2)
//     ->Range(1<<10, 1<<22)
//     ->Repetitions(2)
//     ->DisplayAggregatesOnly(true)
//     ->Complexity();

// BENCHMARK(BM_TreeDeletion_UniqueTail)
//     ->RangeMultiplier(2)
//     ->Range(1<<10, 1<<22)
//     ->Repetitions(2)
//     // ->DisplayAggregatesOnly(true)
//     ->Complexity();

// BENCHMARK(BM_TreeDeletion_Rev1)
//     ->RangeMultiplier(2)
//     ->Range(1<<10, 1<<22)
//     ->Repetitions(4)
//     // ->DisplayAggregatesOnly(true)
//     ->Complexity();

// BENCHMARK(BM_TreeSearch)
//     ->RangeMultiplier(2)
//     ->Range(1<<10, 1<<22)
//     ->Complexity();
