#include <iostream>
#include "/home/sebas/gurobi1102/linux64/include/gurobi_c++.h"
#include <set>
#include <map>
#include <utility>
#include <list>
#include "graphs.h"

//#define BINARY_OR_CONTINUOUS GRB_BINARY
#define BINARY_OR_CONTINUOUS GRB_CONTINUOUS



void find_optimum(const MultipartiteGraph& G){
    try{

    std::map<VertexIndex,std::list<int>> forward_neighbor;
    std::map<VertexIndex,std::list<int>> backward_neighbor;
    for( int i=0; i<G.layers; i++){
        for( int j=0; j<G.elementsPerLayer.at(i); j++){
            forward_neighbor[{i,j}] = {};
            backward_neighbor[{i,j}] = {};
        }
    }
    std::cout << "Before forward backward\n";
    for(auto& edge:G.edges){
        VertexIndex v1 = edge.first;
        VertexIndex v2 = edge.second;
        forward_neighbor[v1].push_back(v2.element);
        backward_neighbor[v2].push_back(v1.element);
    }

    std::cout << "After forward backward\n";
    
    GRBEnv env = GRBEnv(true);

// Set the log file parameter
    env.set("LogFile", "naive.log");
    env.start();

    GRBModel model = GRBModel(env);

    std::map<VertexIndex,GRBVar> vars;
    std::map<VertexIndex, GRBVar> varsAsNeighbors;

    GRBLinExpr sum_of_nbs;

// Create the variables:
    std::cout << "Creating the variables\n";

    for(int j =0; j<G.elementsPerLayer.at(0); j++){
        varsAsNeighbors[{0,j}] = model.addVar(0.0,1.0,0.0,BINARY_OR_CONTINUOUS, "y_0_" + std::to_string(j) );
        sum_of_nbs += varsAsNeighbors[{0,j}];
    }

    for(int i=1; i<G.layers-1; i++){
        for(int j=0; j<G.elementsPerLayer.at(i); j++){
            vars[{i,j}] = model.addVar(0.0,1.0,0.0,BINARY_OR_CONTINUOUS, "x_" + std::to_string(i) + "_" + std::to_string(j) );
            varsAsNeighbors[{i,j}] = model.addVar(0.0,1.0,0.0,BINARY_OR_CONTINUOUS, "y_" + std::to_string(i) + "_" + std::to_string(j) );
            sum_of_nbs += varsAsNeighbors[{i,j}]; 
        }
    }
    for(int j=0; j<G.elementsPerLayer.at(G.layers-1); j++){
        vars[{G.layers-1,j}] = model.addVar(0.0,1.0,0.0,BINARY_OR_CONTINUOUS, "x_" + std::to_string(G.layers-1) + "_" + std::to_string(j) );
        sum_of_nbs += vars[{G.layers-1,j}];


    }


// Neighbor constraints:
    std::cout << "Neighbor constraints\n";

    for(int i=1; i<G.layers; i++){
        for(int j=0; j<G.elementsPerLayer.at(i); j++){
            for(auto& k: backward_neighbor.at({i,j})){
//                std::cout << i << "," << j << std::endl;
//                std::cout << i-1 << "," << k << std::endl;
                model.addConstr(varsAsNeighbors[{i-1,k}]>=vars[{i,j}]);
            }
        }
    }

//Path constraints:
    std::cout << "Backward paths:\n";

    for(int i=2;i<G.layers;i++){
        for(int j=0; j<G.elementsPerLayer.at(i); j++){
            GRBLinExpr expr;
            for(auto& nbr:backward_neighbor.at({i,j})){
                expr += vars[{i-1,nbr}];
            }
            model.addConstr(expr>=vars[{i,j}]);
        }
    }
    std::cout << "Forward paths:\n";

    for(int i=1;i<G.layers-1;i++){
        for(int j=0; j<G.elementsPerLayer.at(i); j++){
            GRBLinExpr expr;
            for(auto& nbr:forward_neighbor.at({i,j})){
                expr += vars[{i+1,nbr}];
            }
            model.addConstr(expr>=vars[{i,j}]);
        }
    }
//objective function and t=1/(sum of nbs):
    std::cout << "Setting the objective function\n";
    GRBVar t = model.addVar(0.0,GRB_INFINITY, 0.0, GRB_CONTINUOUS,"t");
    
    GRBQuadExpr factor = t*sum_of_nbs;
    model.addConstr(sum_of_nbs,GRB_GREATER_EQUAL,1.0);
    model.addQConstr(factor,GRB_EQUAL,1.0);
    
    GRBLinExpr sumXWeights;
    for(int i=1;i<G.layers;i++){
        for(int j=0; j<G.elementsPerLayer.at(i); j++){
            sumXWeights += G.values.at({i,j})*vars[{i,j}];
        }
    }
    GRBQuadExpr objExpr = t*sumXWeights;
    model.setObjective(objExpr,GRB_MAXIMIZE);

//###############################################

//    GRBModel relaxed_model = model.relax();
//    relaxed_model.optimize();

//#############################################
    model.optimize();

    bool print_variable_values = false;

    if(print_variable_values && model.get(GRB_IntAttr_Status) == GRB_OPTIMAL){
        std::cout << "Variable values:" << std::endl;
        for(auto& x : vars){
            std::cout << x.second.get(GRB_StringAttr_VarName) << ": " << x.second.get(GRB_DoubleAttr_X) << std::endl;
        }
        for(auto& x : varsAsNeighbors){
            std::cout << x.second.get(GRB_StringAttr_VarName) << ": " << x.second.get(GRB_DoubleAttr_X) << std::endl;
        }
    }

    } catch(GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }/* catch(...) {
        std::cout << "Exception during optimization" << std::endl;
    }*/

   

}


void test1(){
    int nr_layers = 4;
    std::map<int,int> elementsPerLayer;

    for(int i=0; i<nr_layers; i++){
        elementsPerLayer[i] = 5;
    }
    int max_weight = 10;
    float edge_density =0.3;
    unsigned int seed =1;

    MultipartiteGraph G = generate_random_graph(nr_layers,elementsPerLayer,max_weight,edge_density,seed);


    display_graph(G);

    find_optimum(G);


}


int main(){
    test1();



    return 0;
}
