#include <benchmark/benchmark.h>
#include <tree/AVLTree.hpp>
#include <vector>
#include <random>
#include <algorithm>



static std::random_device g_rd;
static std::mt19937 g_generator(g_rd());
static std::uniform_int_distribution<> g_val_dist(0, 100000);

uint32_t generateRandomNumber() {
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


void generateUniqueVectorSet(std::vector<uint32_t>& vec, uint32_t count)
{
    for(; --count; ) {
        uint32_t valIsUnique = generateRandomNumber();
        while(find(vec.begin(), vec.end(), valIsUnique) != vec.end()) {
            valIsUnique = generateRandomNumber();
        }
        vec.push_back(valIsUnique);
    }
    return;
}



static void BM_TreeInsertion(benchmark::State& state) {
    AVLTree testTree;
    std::vector<uint32_t> testVector;

    generateUniqueVectorSet(testVector, state.range(0));
    for (auto _ : state) {
        for (int i = 0; i < state.range(0); ++i) {
            benchmark::DoNotOptimize(testTree.insert(testVector[i]));
        }
    }
}

static void BM_TreeDeletion(benchmark::State& state) {
    AVLTree testTree;
    std::vector<uint32_t> testVector;


    generateUniqueVectorSet(testVector, state.range(0));
    for(auto& elem : testVector) {
        testTree.insert(elem);
    }


    for (auto _ : state) {
        for(int j = 0; j < state.range(0); ++j) {
            state.PauseTiming();
            auto valToRemove = testVector.back();
            testVector.pop_back();
            state.ResumeTiming();

            benchmark::DoNotOptimize(testTree.remove(valToRemove));
        }
    }
}




static void BM_TreeSearch(benchmark::State& state) {
    AVLTree testTree;
    std::vector<uint32_t> testVector;


    generateUniqueVectorSet(testVector, state.range(0));
    for(auto& elem : testVector) {
        testTree.insert(elem);
    }


    for (auto _ : state) {
        for(int j = 0; j < state.range(0); ++j) {
            state.PauseTiming();
            auto valToSearchIdx = generateRandomNumber() % testVector.size();
            auto valToSearch    = testVector[valToSearchIdx];
            testVector.erase(testVector.begin() + valToSearchIdx);
            state.ResumeTiming();
            
            benchmark::DoNotOptimize(testTree.search(valToSearch));
        }
    }
}

BENCHMARK(BM_TreeInsertion)->Range(1<<10, 1<<16);
BENCHMARK(BM_TreeDeletion)->Range(1<<10, 1<<16);
BENCHMARK(BM_TreeSearch)->Range(1<<10, 1<<16);
