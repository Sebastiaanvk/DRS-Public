#ifndef ALGORITHM_WRAPPERS_H 
#define ALGORITHM_WRAPPERS_H


#include "greedyHNSNPathDirected.h"
#include "greedyWithDynPaths.h"
#include "singlePathDyn.h"
#include "mpGraphInterface.h"
#include "altGurobi.h"

#include <set>
#include <string>
#include <functional>
#include <unordered_map>
#include <list>


namespace Algorithms{

    struct Solution{
            float objectiveValue;
            std::set<VertexIndex> pickedNodes;
            std::set<VertexIndex> neighbors;
            bool solvedToCompletion;
    };

    // Checks whether a solution given by an algorithm implementation is correct
    // twoDir=false for PDRS and twoDir=true for NDRS
    bool checkSolution(const Solution& sol, const MultipartiteSetGraph& graph, bool twoDir);

    // 'MILPAlt': Exact algorithm for PDRS using gurobi and the alternative formulation
    Solution altGurobiUni(const MultipartiteSetGraph& graph);

    // 'MILPAlt': Exact algorithm for NDRS using gurobi and the alternative formulation
    Solution altGurobiBi(const MultipartiteSetGraph& graph);

    // 'Dynamic Path': the exact dynamic programming implementation of PDRS 
    Solution dynSinglePathUni(const MultipartiteSetGraph& graph);

    // 'Dynamic Path': the exact dynamic programming implementation of NDRS 
    Solution dynSinglePathBi(const MultipartiteSetGraph& graph);

    // 'Greedy Single Paths': heuristic algorithm implementation for PDRS using the dyn programming exact algorithm for single paths.
    Solution greedySinglePathsUni(const MultipartiteSetGraph& graph);
    // 'Greedy Single Paths': heuristic algorithm implementation for NDRS using the dyn programming exact algorithm for single paths.
    Solution greedySinglePathsBi(const MultipartiteSetGraph& graph);

    // 'Greedy Peeling': heuristic algorithm implementation for PDRS using greedy peeling.
    Solution greedyPeelingUni(const MultipartiteSetGraph& graph);
    // 'Greedy Peeling': heuristic algorithm implementation for NDRS using greedy peeling.
    Solution greedyPeelingBi(const MultipartiteSetGraph& graph);

    // 'Pick Everything': heuristic baseline algorithm implementation that selects the maximum reachable subgraph for the PDRS problem
    Solution baseLinePickEverythingUni(const MultipartiteSetGraph& graph);
    // 'Pick Everything': heuristic baseline algorithm implementation that selects the maximum reachable subgraph for the NDRS problem
    Solution baseLinePickEverythingBi(const MultipartiteSetGraph& graph);


}

#endif // ALGORITHM_WRAPPERS_H