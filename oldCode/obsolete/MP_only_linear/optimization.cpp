#include "optimization.h"



double find_optimum_linearized(const MultipartiteGraph& G, bool relaxed, bool path_constraints, bool print_variable_values){
    char BINARY_OR_CONTINUOUS = 'B';    
    if(relaxed){
        BINARY_OR_CONTINUOUS = 'C';
    }
    float optimal_value;
    try{

        std::map<VertexIndex,std::list<int>> forward_neighbor;
        std::map<VertexIndex,std::list<int>> backward_neighbor;
        for( int i=0; i<G.layers; i++){
            for( int j=0; j<G.elementsPerLayer.at(i); j++){
                forward_neighbor[{i,j}] = {};
                backward_neighbor[{i,j}] = {};
            }
        }
        for(auto& edge:G.edges){
            VertexIndex v1 = edge.first;
            VertexIndex v2 = edge.second;
            forward_neighbor[v1].push_back(v2.element);
            backward_neighbor[v2].push_back(v1.element);
        }

    
    //setting up the environment and model
        GRBEnv env = GRBEnv(true);

    // Set the log file parameter
        env.set("LogFile", "naive.log");
        env.start();

        GRBModel model = GRBModel(env);

        std::map<VertexIndex,GRBVar> vars;
        std::map<VertexIndex,GRBVar> vars_scaled;
        std::map<VertexIndex, GRBVar> varsAsNeighbors;
        std::map<VertexIndex, GRBVar> varsAsNeighbors_scaled;

        GRBVar S;


//        GRBLinExpr sum_of_nbs;

    // Create the basic variables:
        std::cout << "Creating the variables\n";

        S = model.addVar(0.0,1.0,0.0, 'C', "S");

        for(int j =0; j<G.elementsPerLayer.at(0); j++){
            varsAsNeighbors[{0,j}] = model.addVar(0.0,1.0,0.0,BINARY_OR_CONTINUOUS, "y_0_" + std::to_string(j) );

            varsAsNeighbors_scaled[{0,j}] = model.addVar(0.0,1.0,0.0,'C', "y_0_" + std::to_string(j)+"_scaled" );
        }

        for(int i=1; i<G.layers; i++){
            for(int j=0; j<G.elementsPerLayer.at(i); j++){
                vars[{i,j}] = model.addVar(0.0,1.0,0.0,BINARY_OR_CONTINUOUS, "x_" + std::to_string(i) + "_" + std::to_string(j) );
                vars_scaled[{i,j}] = model.addVar(0.0,1.0,0.0,'C', "x_" + std::to_string(i) + "_" + std::to_string(j)+"_scaled" );
               
                varsAsNeighbors[{i,j}] = model.addVar(0.0,1.0,0.0,BINARY_OR_CONTINUOUS, "y_" + std::to_string(i) + "_" + std::to_string(j) );
                varsAsNeighbors_scaled[{i,j}] = model.addVar(0.0,1.0,0.0,'C', "y_" + std::to_string(i) + "_" + std::to_string(j) + "_scaled" );
            }
        }
        /*
        for(int j=0; j<G.elementsPerLayer.at(G.layers-1); j++){
            vars[{G.layers-1,j}] = model.addVar(0.0,1.0,0.0,BINARY_OR_CONTINUOUS, "x_" + std::to_string(G.layers-1) + "_" + std::to_string(j) );
            sum_of_nbs += vars[{G.layers-1,j}];


        }
        */



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

    // Making sure the picked nodes also get added to the denominator
        for(int i=1; i<G.layers; i++){
            for(int j=0; j<G.elementsPerLayer.at(i); j++){
                model.addConstr( varsAsNeighbors[{i,j}] >= vars[{i,j}]);
            }
        }


    //Path constraints:
        if(path_constraints){
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
        }


    //Scaling the variables
        GRBLinExpr sum_of_scaled_nbs;
        for(int i=0; i<G.layers; i++){
            for( int j=0; j<G.elementsPerLayer.at(i); j++){
                VertexIndex ind = {i,j};
                model.addConstr(0<=varsAsNeighbors_scaled[ind]);
                model.addConstr(varsAsNeighbors_scaled[ind]<=varsAsNeighbors[ind]);
                model.addConstr(0<=S-varsAsNeighbors_scaled[ind]);
                model.addConstr(S-varsAsNeighbors_scaled[ind]<=1-varsAsNeighbors[ind]);
                sum_of_scaled_nbs += varsAsNeighbors_scaled[ind];
            }
        }
        model.addConstr(sum_of_scaled_nbs==1);
        
        for(int i=1; i<G.layers; i++){
            for( int j=0; j<G.elementsPerLayer.at(i); j++){
                VertexIndex ind = {i,j};
                model.addConstr(0<=vars_scaled[ind]);
                model.addConstr(vars_scaled[ind]<=vars[ind]);
                model.addConstr(0<=S-vars_scaled[ind]);
                model.addConstr(S-vars_scaled[ind]<=1-vars[ind]);
            }
        }
 
    //objective function and t=1/(sum of nbs):
        std::cout << "Setting the objective function\n";
    
        GRBLinExpr sumXWeights;
        for(int i=1;i<G.layers;i++){
            for(int j=0; j<G.elementsPerLayer.at(i); j++){
                sumXWeights += G.values.at({i,j})*vars_scaled[{i,j}];
            }
        }
        model.setObjective(sumXWeights,GRB_MAXIMIZE);

        model.optimize();


        if(print_variable_values && model.get(GRB_IntAttr_Status) == GRB_OPTIMAL){
            std::cout << "Variable values:" << std::endl;
            for(int i=1; i<G.layers; i++){
                for(int j=0; j<G.elementsPerLayer.at(i); j++){
                    GRBVar x = vars[{i,j}];
                    std::cout << x.get(GRB_StringAttr_VarName) << ": " << x.get(GRB_DoubleAttr_X) << ", ";
                    x = vars_scaled[{i,j}];
                    std::cout << x.get(GRB_StringAttr_VarName) << ": " << x.get(GRB_DoubleAttr_X) << std::endl;
                }
            }
            for(int i=0; i<G.layers; i++){
                for(int j=0; j<G.elementsPerLayer.at(i); j++){
                    GRBVar x = varsAsNeighbors[{i,j}];
                    std::cout << x.get(GRB_StringAttr_VarName) << ": " << x.get(GRB_DoubleAttr_X) << ", ";
                    x = varsAsNeighbors_scaled[{i,j}];
                    std::cout << x.get(GRB_StringAttr_VarName) << ": " << x.get(GRB_DoubleAttr_X) << std::endl;
                }
            }
        }
    optimal_value = model.get(GRB_DoubleAttr_ObjVal);

    } catch(GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }/* catch(...) {
        std::cout << "Exception during optimization" << std::endl;
    }*/

   return optimal_value; 

}