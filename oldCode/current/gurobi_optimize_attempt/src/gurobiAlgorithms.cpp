#include "../include/gurobiAlgorithms.h"






double new_gurobi_test_1dir_direct(const MultipartiteSetGraph& graph, bool print_variables){


    std::set<VertexIndex> nodes;
    std::set<VertexIndex> neighbors;
// In the preprocessed version, we won't add all the nodes and neighbors.
    for(int layer=0; layer<graph.getLayers(); layer++){
        for(int x=0; x<graph.getElementsInLayer(layer); x++){
            nodes.insert({layer,x});
            neighbors.insert({layer,x});
        }
    }


//setting up the environment and model
    GRBEnv env = GRBEnv(true);

// Set the log file parameter
    env.set("LogFile", "output/logs/gurobi.log");
    env.start();

    GRBModel model(env);
    model.set(GRB_IntParam_OutputFlag, 0);
    model.set( GRB_IntAttr_ModelSense  , GRB_MAXIMIZE);

    try {

        

        std::map<VertexIndex,std::list<VertexIndex>> forward_neighbor;
        std::map<VertexIndex,std::list<VertexIndex>> backward_neighbor;
    
        for(auto& edge:graph.edges){
            VertexIndex v1 = edge.first;
            VertexIndex v2 = edge.second;
            //We only add the edge if the edge is between two nodes or a neighbor and a node
            if( ( neighbors.count(v1) || nodes.count(v1) ) && (neighbors.count(v2) || nodes.count(v2) ) && ( nodes.count(v1) || nodes.count(v2) ) ){
                forward_neighbor[v1].push_back(v2);
                backward_neighbor[v2].push_back(v1);
            }
        }

        std::map<VertexIndex,GRBVar> node_vars;
        std::map<VertexIndex,GRBVar> node_vars_scaled;
        std::map<VertexIndex, GRBVar> neighbor_vars_scaled;
        GRBVar S = model.addVar(0.0,1.0,0.0, 'C', "S");

//        GRBLinExpr objectiveFunction;

        for(auto node : nodes){
            node_vars[node] = model.addVar(0.0,1.0,0.0, 'B', "x_" + std::to_string(node.layer) + "_" + std::to_string(node.element));
            node_vars_scaled[node] = model.addVar(0.0,1.0,graph.getValue(node),'C', "x_" + std::to_string(node.layer) + "_" + std::to_string(node.element)+"_scaled" );
//            objectiveFunction += node_vars_scaled[node]*graph.getValue(node);

        }

//       model.setObjective(objectiveFunction,GRB_MAXIMIZE);

        for(auto nb:neighbors){
            neighbor_vars_scaled[nb] = model.addVar(0.0,1.0,0.0,'C', "y_" + std::to_string(nb.layer) + "_" + std::to_string(nb.element)+"_scaled" );
        }
        for(auto node : nodes){
            model.addConstr(node_vars_scaled[node]<=node_vars[node]);       // x=0 => x^=0
            model.addConstr(node_vars_scaled[node]<=S);                     // x^ <=S 
            model.addConstr(node_vars_scaled[node]-S>=node_vars[node]-1);   // x^>=S-(1-x)
        }
        GRBLinExpr sum_of_scaled_nbs;

        for(auto nb : neighbors){
            sum_of_scaled_nbs += neighbor_vars_scaled[nb];

    // Right now we only consider backward nbs
            for(auto node : forward_neighbor[nb]){
                model.addConstr(neighbor_vars_scaled[nb] >= node_vars_scaled[node] );
            }
            if(nodes.count(nb)){
                model.addConstr(neighbor_vars_scaled[nb]>=node_vars_scaled[nb]);
            }
        }


        model.addConstr(sum_of_scaled_nbs==1);

        for(auto node : nodes){
            if(node.layer>0){
                GRBLinExpr backward_path;        
                for(auto backnode : backward_neighbor[node]){
                    backward_path += node_vars[backnode];
                }
                model.addConstr(backward_path>=node_vars[node]);
            }

            if(node.layer<graph.getLayers()-1){
                GRBLinExpr forward_path;        
                for(auto forwardnode : forward_neighbor[node]){
                    forward_path += node_vars[forwardnode];
                }
                model.addConstr(forward_path>=node_vars[node]);
            }
        }

        model.optimize();

        if(print_variables){

            std::cout << "Variable values:" << std::endl;
            for(auto node : nodes){
                GRBVar x = node_vars[node];
                std::cout << x.get(GRB_StringAttr_VarName) << ": " << x.get(GRB_DoubleAttr_X) << ", ";
                x = node_vars_scaled[node];
                std::cout << x.get(GRB_StringAttr_VarName) << ": " << x.get(GRB_DoubleAttr_X) << std::endl;
            }

            for(auto nb : neighbors){
                GRBVar x = neighbor_vars_scaled[nb];
                std::cout << x.get(GRB_StringAttr_VarName) << ": " << x.get(GRB_DoubleAttr_X) << std::endl;
            }
        }

    } catch(GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }

    return model.get(GRB_DoubleAttr_ObjVal);

}



// This is for 1dir neighbors, Might add a bool to indicate 2 dir or 1 direction
ProcessedGraph minimalProcessing(const MultipartiteSetGraph& G){
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
        //We only add the edge if the edge is between two nodes or a neighbor and a node
        if(v1.layer<v2.layer){
            graph.forward_neighbor[v1].push_back(v2);
            graph.backward_neighbor[v2].push_back(v1);
            graph.neighbor_connections[v1].push_back(v2);
        } else {
            graph.forward_neighbor[v2].push_back(v1);
            graph.backward_neighbor[v1].push_back(v2);
            graph.neighbor_connections[v2].push_back(v1);

        }

    }

    return graph;
}





Solution gurobi_optimize(const ProcessedGraph& graph, const std::map<VertexIndex,int> priorityValues ){
    Solution solution;
//setting up the environment and model
    GRBEnv env = GRBEnv(true);

// Set the log file parameter
    env.set("LogFile", "output/logs/gurobi.log");
    env.start();

    GRBModel model(env);
    model.set(GRB_IntParam_OutputFlag, 0);
    model.set( GRB_IntAttr_ModelSense  , GRB_MAXIMIZE);

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
            model.addConstr(node_vars_scaled[node]<=S);                     // x^ <=S 
            model.addConstr(node_vars_scaled[node]-S>=node_vars[node]-1);   // x^>=S-(1-x)
        }


        for( auto el : priorityValues){
            node_vars[el.first].set(GRB_IntAttr_BranchPriority,el.second); 
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

    } catch(GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }

    return solution;
}

Solution gurobi_optimize_start_values(const ProcessedGraph& graph, const std::map<VertexIndex,int> priorityValues, const std::set<VertexIndex> picked_nodes ){
    Solution solution;
//setting up the environment and model
    GRBEnv env = GRBEnv(true);

// Set the log file parameter
    env.set("LogFile", "output/logs/gurobi.log");
    env.start();

    GRBModel model(env);
    model.set(GRB_IntParam_OutputFlag, 0);
    model.set( GRB_IntAttr_ModelSense  , GRB_MAXIMIZE);

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
            model.addConstr(node_vars_scaled[node]<=S);                     // x^ <=S 
            model.addConstr(node_vars_scaled[node]-S>=node_vars[node]-1);   // x^>=S-(1-x)
        }


        for( auto el : priorityValues){
            node_vars[el.first].set(GRB_IntAttr_BranchPriority,el.second); 
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

        model.update();
        for(auto node : graph.nodes){
            if(picked_nodes.count(node)>0){
                node_vars[node].set(GRB_DoubleAttr_Start, 1 );
            } else {
                node_vars[node].set(GRB_DoubleAttr_Start, 0 );
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

    } catch(GRBException e) {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }

    return solution;

}    


double warm_start_heuristic_test(const MultipartiteSetGraph& G){

    GreedyWithDynPathsSol heuristic = greedyDynPaths1dir(G,false);

    ProcessedGraph graph = minimalProcessing(G);
    std::map<VertexIndex,int> priorityValues;

    Solution sol = gurobi_optimize_start_values(graph, priorityValues, heuristic.picked_nodes);

    return sol.value;
}


double priority_test(const MultipartiteSetGraph& G, bool print_variables){
    ProcessedGraph graph =  minimalProcessing(G);
    std::map<VertexIndex,int> priorityValues;
    int mid_layer = G.getLayers()/2;
    for(int x=0; x<G.getElementsInLayer(mid_layer); x++){
        priorityValues[{mid_layer,x}];
    }
    Solution sol = gurobi_optimize(graph, priorityValues);
    return sol.value;
}



void processingDfs(const VertexIndex& current_node, std::unordered_set<VertexIndex>& reached,
                                        const std::map<VertexIndex,std::list<VertexIndex>>& arcs){
    for(auto vi : arcs.at(current_node)){
        if(!reached.count(vi)){
            reached.insert(vi);
            processingDfs(vi, reached,arcs);
        }
    }
}

ProcessedGraph properPreprocessing(const MultipartiteSetGraph& G, bool two_dir_nbs){
    int nr_layers = G.getLayers();
    std::map<VertexIndex,std::list<VertexIndex>> forwardArcs;
    std::map<VertexIndex,std::list<VertexIndex>> backwardArcs;

    for(int layer=0; layer<G.getLayers(); layer++){
        for(int x=0; x<G.getElementsInLayer(layer); x++){
            forwardArcs[{layer, x}] = std::list<VertexIndex>(0);
            backwardArcs[{layer,x}] = std::list<VertexIndex>(0);
        }
    }
    
    for(auto& edge:G.edges){
        VertexIndex v1 = edge.first;
        VertexIndex v2 = edge.second;
        forwardArcs[v1].push_back(v2);
        backwardArcs[v2].push_back(v1);
    }

    std::unordered_set<VertexIndex> forwardReached;
    for(int x=0; x<G.getElementsInLayer(0); x++){
        forwardReached.insert({0,x});
        processingDfs({0,x}, forwardReached, forwardArcs);
    }
    std::unordered_set<VertexIndex> backwardReached;
    for(int x=0; x<G.getElementsInLayer(nr_layers-1); x++){
        backwardReached.insert({nr_layers-1,x});
        processingDfs({nr_layers-1,x}, backwardReached, backwardArcs);
    }

    ProcessedGraph graph;
    graph.nr_layers = nr_layers;
    for(int layer=0; layer<nr_layers; layer++){
        for(int x=0; x<G.getElementsInLayer(layer); x++){
            if(forwardReached.count({layer,x})+backwardReached.count({layer,x})==2){
                graph.nodes.insert({layer,x});
            }
        }
    }
    for(auto node : graph.nodes){
        graph.node_values[node] = G.getValue(node);
        graph.backward_neighbor[node] = std::list<VertexIndex>(0);
        graph.forward_neighbor[node] = std::list<VertexIndex>(0);
        graph.neighbors.insert(node);
        if(node.layer>0){
            for(auto vi : backwardArcs[node]){
                if(graph.nodes.count(vi)){
                    graph.backward_neighbor[node].push_back(vi);
                }
                graph.neighbors.insert(vi);
                graph.neighbor_connections[vi].push_back(node);
            }
        }
        if(node.layer<nr_layers-1){
            for(auto vi : forwardArcs[node]){
                if(graph.nodes.count(vi)){
                    graph.forward_neighbor[node].push_back(vi);
                }
                if(two_dir_nbs){
                    graph.neighbors.insert(vi);
                    graph.neighbor_connections[vi].push_back(node);
                }
            }
        }
    }

    return graph;
}

double preprocessingTest(const MultipartiteSetGraph& G, bool print_variables){
    ProcessedGraph graph =  properPreprocessing(G,false);
    std::map<VertexIndex,int> priorityValues;
    Solution sol = gurobi_optimize(graph, priorityValues);
    return sol.value;
}





