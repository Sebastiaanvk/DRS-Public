#ifndef GUROBI_MODEL_WRAPPER_H 
#define GUROBI_MODEL_WRAPPER_H 

#include <iostream>
#include "/home/sebas/gurobi1102/linux64/include/gurobi_c++.h"
#include <set>
#include <map>
#include <utility>
#include <list>
#include "mpSetGraph.h"
#include "modelInterface.h"

GRBEnv initialEnvironment();

class GurobiModelWrapper : public OptimizableInterface {
private:
    bool relaxed;
    MultipartiteSetGraph G;
    GRBModel model;
    bool print_stuff;
    std::map<VertexIndex,GRBVar> vars;
    std::map<VertexIndex,GRBVar> vars_scaled;
    std::map<VertexIndex, GRBVar> varsAsNeighbors;
    std::map<VertexIndex, GRBVar> varsAsNeighbors_scaled;
    GRBVar S;
    std::map<VertexIndex,std::list<int>> forward_neighbor;
    std::map<VertexIndex,std::list<int>> backward_neighbor;
    GRBLinExpr sum_of_scaled_nbs;

public:
    GurobiModelWrapper(const MultipartiteSetGraph& graph, bool relaxed, const GRBEnv& env);
    void turnOffOutput();
    void addBackwardNeighborConstraints();
    void addForwardNeighborConstraints();
    void addPathConstraints();
    void addSinglePathConstraint();

    void addStandardObjectiveFunction();
    
    double optimize() override;

    bool vertexPicked(const VertexIndex& v) const override ;
    bool vertexPickedAsNeighbor(const VertexIndex& v) const override;

    void printVariables();
};



#endif // GUROBI_MODEL_WRAPPER_H