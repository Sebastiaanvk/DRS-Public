#ifndef GUROBI_SINGLE_PATH_MODEL_WRAPPER_H
#define GUROBI_SINGLE_PATH_MODEL_WRAPPER_H


#include "/home/sebas/gurobi1102/linux64/include/gurobi_c++.h"
#include "mpSetGraph.h"
#include "modelInterface.h"
#include <list>


GRBEnv initialEnvironment();

class GurobiSinglePathModelWrapper : public OptimizableInterface {
private:
    bool relaxed;
    MultipartiteSetGraph G;
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
    GurobiSinglePathModelWrapper(const MultipartiteSetGraph& graph, bool relaxed, const GRBEnv& env);

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






#endif //GUROBI_SINGLE_PATH_MODEL_WRAPPER_H