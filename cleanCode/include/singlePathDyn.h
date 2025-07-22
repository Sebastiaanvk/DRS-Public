#ifndef SINGLE_PATH_DYN_H
#define SINGLE_PATH_DYN_H

#include "mpGraphInterface.h"
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <iostream>

typedef std::vector<std::vector<std::vector<int>>> MPArcList;


struct SinglePathDynSol{
    float max_value;
    std::set<VertexIndex> picked_nodes;
    std::set<VertexIndex> picked_neighbors;
};

// The implementation of the exact dynamic programming algorithm for calculating the path with the highest objective value for the PDRS problem
SinglePathDynSol singlePathDyn1DirNbs(const MPGraphInterface& mpGraph);

// The implementation of the exact dynamic programming algorithm for calculating the path with the highest objective value for the NDRS problem
SinglePathDynSol singlePathDynNbs(const MPGraphInterface& mpGraph);



#endif //SINGLE_PATH_DYN_H
