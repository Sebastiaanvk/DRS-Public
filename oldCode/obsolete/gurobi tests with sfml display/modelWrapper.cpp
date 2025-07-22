#include "modelWrapper.h"


GRBEnv initialEnvironment(){
//setting up the environment and model
    GRBEnv env = GRBEnv(true);

// Set the log file parameter
    env.set("LogFile", "naive.log");
    env.start();

    return env;
}

ModelWrapper::ModelWrapper( const MultipartiteGraph& graph, bool relaxed, const GRBEnv& env)
    : model(env)
{
    G = graph;
    this->relaxed = relaxed;



    char BINARY_OR_CONTINUOUS = 'B';    
    if(relaxed){
        BINARY_OR_CONTINUOUS = 'C';
    }
    float optimal_value;
    try{
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

    //Scaling the variables
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



    } catch(GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }
}


void ModelWrapper::addBackwardNeighborConstraints(){
    try{

        std::cout << "Backward neighbor constraints\n";

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
    } catch(GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }
}

void ModelWrapper::addForwardNeighborConstraints(){
    try{

        std::cout << "Forward neighbor constraints\n";

        for(int i=1; i<G.layers-1; i++){
            for(int j=0; j<G.elementsPerLayer.at(i); j++){
                for(auto& k: forward_neighbor.at({i,j})){
                    model.addConstr(varsAsNeighbors[{i+1,k}]>=vars[{i,j}]);
                }
            }
        }
    } catch(GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }
}

void ModelWrapper::addPathConstraints(){
    try{
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

    } catch(GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }
}



void ModelWrapper::addStandardObjectiveFunction(){
    try{
        std::cout << "Setting the objective function\n";
    
        GRBLinExpr sumXWeights;
        for(int i=1;i<G.layers;i++){
            for(int j=0; j<G.elementsPerLayer.at(i); j++){
                sumXWeights += G.values.at({i,j})*vars_scaled[{i,j}];
            }
        }
        model.setObjective(sumXWeights,GRB_MAXIMIZE);

    } catch(GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }
}


double ModelWrapper::optimize(){
    try{
        model.optimize();
    } catch(GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }

    return model.get(GRB_DoubleAttr_ObjVal);
}


void ModelWrapper::printVariables(){
    try{
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
    } catch(GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }
}



bool ModelWrapper::vertexPicked(const VertexIndex& vi) const {
    return vars.at(vi).get(GRB_DoubleAttr_X)>0.5;

}
bool ModelWrapper::vertexPickedAsNeighbor(const VertexIndex& vi) const {
    return varsAsNeighbors.at(vi).get(GRB_DoubleAttr_X)>0.5;

}

