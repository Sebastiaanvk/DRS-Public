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

bool checkSinglePath(const MPGraphInterface& mpgraph, const SinglePathDynSol& sol , bool twoDir);

SinglePathDynSol singlePathDynHeur(const MPGraphInterface& mpGraph);

SinglePathDynSol singlePathDynNbs(const MPGraphInterface& mpGraph);

SinglePathDynSol singlePathDyn1DirNbs(const MPGraphInterface& mpGraph);

SinglePathDynSol singlePath1DirNbsBruteForce(const MPGraphInterface& mpGraph);

SinglePathDynSol singlePathBruteForce(const MPGraphInterface& mpGraph);


#endif //SINGLE_PATH_DYN_H
