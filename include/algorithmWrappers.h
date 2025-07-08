#ifndef ALGORITHM_WRAPPERS_H 
#define ALGORITHM_WRAPPERS_H


#include "greedyHNSNPathDirected.h"
#include "greedyWithDynPaths.h"
#include "gurobiModelWrapper.h"
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

    bool checkSolution(const Solution& sol, const MultipartiteSetGraph& graph, bool twoDir);

    Solution normalGurobiUni(const MultipartiteSetGraph& graph);
    Solution normalGurobiBi(const MultipartiteSetGraph& graph);

    Solution altGurobiUni(const MultipartiteSetGraph& graph);
    Solution altGurobiBi(const MultipartiteSetGraph& graph);

    Solution dynSinglePathUni(const MultipartiteSetGraph& graph);
    Solution dynSinglePathBi(const MultipartiteSetGraph& graph);

    Solution greedySinglePathsUni(const MultipartiteSetGraph& graph);
    Solution greedySinglePathsBi(const MultipartiteSetGraph& graph);

    Solution greedyPeelingUni(const MultipartiteSetGraph& graph);
    Solution greedyPeelingBi(const MultipartiteSetGraph& graph);

    Solution baseLinePickEverythingUni(const MultipartiteSetGraph& graph);
    Solution baseLinePickEverythingBi(const MultipartiteSetGraph& graph);

//    std::string algToString(std::function<Solution(const MultipartiteSetGraph& graph)> algorithm);

}

#endif // ALGORITHM_WRAPPERS_H