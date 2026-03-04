#include <benchmark/benchmark.h>
#include <tree/AVLTree.hpp>
#include <vector>
#include <random>
#include <algorithm>
#include <cassert>


uint32_t generateRandomNumber() {
    static std::random_device g_rd;
    static std::mt19937 g_generator(g_rd());
    static std::uniform_int_distribution<> g_val_dist(0, 100000);


    return g_val_dist(g_generator);
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




static void BM_TreeDeletion3(benchmark::State& state) {
    AVLTree testTree;
    std::vector<uint32_t> testVector;
    uint32_t N           = state.range(0);
    uint32_t valToRemIdx = 0;
    uint32_t valToRem    = 0;
    
    generateUniqueVectorSet(testVector, N);
    for(auto& elem : testVector) {
        testTree.insert(elem);
    }


    for (auto _ : state) {
        state.PauseTiming();
        valToRemIdx = generateRandomNumber() % testVector.size();
        valToRem = testVector[valToRemIdx];
        testVector.erase(testVector.begin() + valToRemIdx);
        state.ResumeTiming();
        benchmark::DoNotOptimize(testTree.remove(valToRem));
    }

    state.SetComplexityN(N);
    return;
}


static void BM_TreeDeletion4(benchmark::State& state) {
    const uint32_t N = state.range(0);
    
    std::random_device    rd;
    std::mt19937          gen(rd());
    AVLTree               testTree;
    std::vector<uint32_t> testVec;


    testVec.clear();
    generateUniqueVectorSet(testVec, N);
    while (state.KeepRunningBatch(N)) 
    {
        state.PauseTiming();

        testTree.clear();
        for(auto& elem : testVec) {
            testTree.insert(elem);
        }
        std::shuffle(testVec.begin(), testVec.end(), gen);
        
        state.ResumeTiming();
        
        
        for (uint32_t i = 0; i < N; ++i) {
            benchmark::DoNotOptimize(testTree.remove(testVec[i]));
        }
    }

    state.SetComplexityN(N);
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


static void BM_TreeDeletion_Single(benchmark::State& state) {
    const uint32_t N = state.range(0);
    std::mt19937 gen(42);
    
    AVLTree testTree;
    std::vector<uint32_t> testVec;
    generateUniqueVectorSet(testVec, N);

    printf("state.range(0) ->%u\n", N);
    fflush(stdout);
    for (auto _ : state) {
        state.PauseTiming();
        testTree.clear();
        for(auto& x : testVec) {
            testTree.insert(x);
        }

        uint32_t target = testVec[gen() % N];
        state.ResumeTiming();

        benchmark::DoNotOptimize(testTree.remove(target));

    }
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
    testVecIndices.resize(N);
    for(auto& idx : testVecIndices) {
        idx = generateRandomNumber() % N;
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


// BENCHMARK(BM_TreeInsertion)->Range(1<<10, 1<<20)->Complexity();
// BENCHMARK(BM_TreeDeletion)->Range(1<<10, 1<<20)->Complexity();
BENCHMARK(BM_TreeDeletion_Single)
    ->RangeMultiplier(2)
    ->Range(1<<10, 1<<20)
    ->Repetitions(4)
    ->DisplayAggregatesOnly(true)
    ->Complexity();

BENCHMARK(BM_TreeSearch)->RangeMultiplier(2)->Range(1<<10, 1<<20)->Complexity();
