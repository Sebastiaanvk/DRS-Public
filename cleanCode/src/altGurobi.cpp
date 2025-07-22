#include "../include/altGurobi.h"


ProcessedGraph minimalProcessing(const MultipartiteSetGraph& G, bool biDirectional){
    ProcessedGraph graph;
    graph.nr_layers = G.getLayers();
    for(int layer=0; layer<G.getLayers(); layer++){
        for(int x=0; x<G.getElementsInLayer(layer); x++){
            graph.nodes.insert({layer,x});
            graph.neighbors.insert({layer,x});
            graph.node_values[{layer,x}] = G.getValue({layer,x});
            graph.neighbor_connections[{layer,x}].push_back({layer,x});
            graph.backward_neighbor[{layer,x}] = std::list<VertexIndex>(0);
            graph.forward_neighbor[{layer, x}] = std::list<VertexIndex>(0);
        }
    }

    for(auto& edge:G.edges){
        VertexIndex v1 = edge.first;
        VertexIndex v2 = edge.second;
        if(v1.layer<v2.layer){
            graph.forward_neighbor[v1].push_back(v2);
            graph.backward_neighbor[v2].push_back(v1);
            graph.neighbor_connections[v1].push_back(v2);
            if(biDirectional){
                graph.neighbor_connections[v2].push_back(v1);
            }
        } else {
            graph.forward_neighbor[v2].push_back(v1);
            graph.backward_neighbor[v1].push_back(v2);
            graph.neighbor_connections[v2].push_back(v1);
            if(biDirectional){
                graph.neighbor_connections[v1].push_back(v2);
            }
        }
    }
    return graph;
}




GurobiAltSolution altGurobiOptimize(const ProcessedGraph& graph,float maxTimeout){

    GurobiAltSolution solution;
//setting up the environment and model
    GRBEnv env = GRBEnv(true);

// Set the log file parameter
    env.set("LogFile", "output/logs/gurobi.log");
    env.start();

    GRBModel model(env);
    model.set(GRB_IntParam_OutputFlag, 0);
    model.set( GRB_IntAttr_ModelSense  , GRB_MAXIMIZE);
    model.set(GRB_DoubleParam_TimeLimit, maxTimeout);
    try {

       //Creating variables 
        std::map<VertexIndex,GRBVar> node_vars;
        std::map<VertexIndex,GRBVar> node_vars_scaled;
        std::map<VertexIndex, GRBVar> neighbor_vars_scaled;
        GRBVar S = model.addVar(0.0,1.0,0.0, 'C', "S");

        for(auto node : graph.nodes){
            node_vars[node] = model.addVar(0.0,1.0,0.0, 'B', "x_" + std::to_string(node.layer) + "_" + std::to_string(node.element));
            node_vars_scaled[node] = model.addVar(0.0,1.0,graph.node_values.at(node),'C', "x_" + std::to_string(node.layer) + "_" + std::to_string(node.element)+"_scaled" );
            model.addConstr(node_vars_scaled[node]<=node_vars[node]);       // x=0 => x^=0
            model.addConstr(0 <=S - node_vars_scaled[node]);                     // 0 <=S - x^ 
            model.addConstr(S - node_vars_scaled[node]<= 1 - node_vars[node]);   // S - x^ <= 1-x
        }

        GRBLinExpr sum_of_scaled_nbs;

        for(auto nb:graph.neighbors){
            neighbor_vars_scaled[nb] = model.addVar(0.0,1.0,0.0,'C', "y_" + std::to_string(nb.layer) + "_" + std::to_string(nb.element)+"_scaled" );
            sum_of_scaled_nbs += neighbor_vars_scaled[nb];
            if(graph.neighbor_connections.count(nb)){
                for(auto node : graph.neighbor_connections.at(nb)){
                    model.addConstr(neighbor_vars_scaled[nb] >= node_vars_scaled[node] );
                }
            }

            if(graph.nodes.count(nb)){
                model.addConstr(neighbor_vars_scaled[nb]>=node_vars_scaled[nb]);
            }

        }

        model.addConstr(sum_of_scaled_nbs==1);

        for(auto node : graph.nodes){
            if(node.layer>0){
                GRBLinExpr backward_path;        
                if(graph.backward_neighbor.count(node)){
                    for(auto backnode : graph.backward_neighbor.at(node)){
                        backward_path += node_vars[backnode];
                    }
                }

                model.addConstr(backward_path>=node_vars[node]);
            }

            if(node.layer<graph.nr_layers-1){
                GRBLinExpr forward_path;        
                if(graph.forward_neighbor.count(node)){
                    for(auto forwardnode : graph.forward_neighbor.at(node)){
                        forward_path += node_vars[forwardnode];
                    }
                }

                model.addConstr(forward_path>=node_vars[node]);
            }
        }

        model.optimize();

        solution.value = model.get(GRB_DoubleAttr_ObjVal);
        for(auto node : graph.nodes){
            if(node_vars[node].get(GRB_DoubleAttr_X) > 0.5){
                solution.nodes.insert(node);
            }
        }

        float s = S.get(GRB_DoubleAttr_X);
        for(auto nb : graph.neighbors){
            if(neighbor_vars_scaled[nb].get(GRB_DoubleAttr_X) > s*0.5) {
                solution.neighbors.insert(nb);
            }
        }
        solution.solvedToCompletion = !(model.get(GRB_IntAttr_Status)==GRB_TIME_LIMIT);

    } catch(GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }

    return solution;
}