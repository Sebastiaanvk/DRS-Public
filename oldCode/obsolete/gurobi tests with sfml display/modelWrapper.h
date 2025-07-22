#ifndef MODEL_WRAPPER_H 
#define MODEL_WRAPPER_H 

#include <iostream>
#include "/home/sebas/gurobi1102/linux64/include/gurobi_c++.h"
#include <set>
#include <map>
#include <utility>
#include <list>
#include "graphs.h"

GRBEnv initialEnvironment();

class ModelWrapper{
private:
    bool relaxed;
    MultipartiteGraph G;
    GRBModel model;
    std::map<VertexIndex,GRBVar> vars;
    std::map<VertexIndex,GRBVar> vars_scaled;
    std::map<VertexIndex, GRBVar> varsAsNeighbors;
    std::map<VertexIndex, GRBVar> varsAsNeighbors_scaled;
    GRBVar S;
    std::map<VertexIndex,std::list<int>> forward_neighbor;
    std::map<VertexIndex,std::list<int>> backward_neighbor;
    GRBLinExpr sum_of_scaled_nbs;

public:
    ModelWrapper(const MultipartiteGraph& graph, bool relaxed, const GRBEnv& env);

    void addBackwardNeighborConstraints();
    void addForwardNeighborConstraints();
    void addPathConstraints();

    void addStandardObjectiveFunction();
    
    double optimize();

    bool vertexPicked(const VertexIndex& v) const;
    bool vertexPickedAsNeighbor(const VertexIndex& v) const;

    void printVariables();
};



#endif // MODEL_WRAPPER_H