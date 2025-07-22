#ifndef GREEDY_WITH_DYN_PATHS_H
#define GREEDY_WITH_DYN_PATHS_H

#include "mpGraphInterface.h"
#include "SFMLGraphs.h"
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>


struct GreedyWithDynPathsSol{
    float value;
    std::set<VertexIndex> picked_nodes;
    std::set<VertexIndex> picked_nbs;
};

// Greedy Single Paths implementatioin for PDRS
GreedyWithDynPathsSol greedyDynPaths1dir(const MPGraphInterface& mpgraph, bool print_iterations);

// Greedy Single Paths implementatioin for NDRS
GreedyWithDynPathsSol greedyDynPaths2dir(const MPGraphInterface& mpgraph, bool print_iterations);


#endif //GREEDY_WITH_DYN_PATHS_H
