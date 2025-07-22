#include "../include/greedyHNSNPathDirected.h"


MPGraphAsGreedyGraph mpGraphToGreedyGraph(const MPGraphInterface* mpGraphPointer, bool forwardNeighborConstraints){
    std::map<VertexIndex,int> vertexIndexToInt;
    std::vector<VertexIndex> intToVertexIndex;
    GreedyGraph graph;

    int layer = 0;
    int elt = 0;
    Vertex index = 0;

    while(layer < mpGraphPointer->getLayers()){
        while(elt<mpGraphPointer->getElementsInLayer(layer)){
            if(layer==0){
                graph.node_values.push_back(mpGraphPointer->getValue({layer,elt}));
                graph.first_layer.push_back(index);
            } else {
                graph.node_values.push_back(mpGraphPointer->getValue({layer,elt}));
            }
            if(layer==mpGraphPointer->getLayers()-1){
                graph.last_layer.push_back(index);
            }
            vertexIndexToInt[{layer,elt}] = index;
            intToVertexIndex.push_back({layer,elt});
            elt++;
            index++;
        }
        elt = 0;
        layer += 1;
    }
    graph.nr_nodes = index;
    graph.nr_neighbors = index;
    graph.forward_arcs = std::vector<std::set<Vertex>>(graph.nr_nodes);  
    graph.backward_arcs = std::vector<std::set<Vertex>>(graph.nr_nodes);  
    graph.nb_to_nodes = std::vector<std::set<Vertex>>(graph.nr_neighbors);
    graph.node_to_nbs = std::vector<std::set<Vertex>>(graph.nr_nodes);
    
    for(Vertex i=0; i<graph.nr_nodes; i++){
        graph.nb_to_nodes[i].insert(i);
    }

    std::unique_ptr<EdgeIterator> it = mpGraphPointer->getEdgeIterator();
    while(it->has_next()){
        Edge edge = it->current();
        int index1 = vertexIndexToInt[edge.first];
        int index2 = vertexIndexToInt[edge.second];
        graph.forward_arcs[index1].insert(index2);
        graph.backward_arcs[index2].insert(index1);
        graph.nb_to_nodes[index1].insert(index2);
        if(forwardNeighborConstraints){
            graph.nb_to_nodes[index2].insert(index1);
        }
        it->next();
    }
    for( int nb=0; nb<graph.nr_neighbors; nb++){
        for(int x : graph.nb_to_nodes[nb]){
            graph.node_to_nbs[x].insert(nb);
        }
    }

   return {graph, vertexIndexToInt, intToVertexIndex};
}

void greedyplus(GreedyGraph greedyGraph,  int nr_iterations){
}

/*
GreedyModel::GreedyModel(MPGraphInterface* mpGraphPointer, bool addForwardNeighborConstraints)
    : mpGraphPointer(mpGraphPointer), addForwardNeighborConstraints(addForwardNeighborConstraints){
}*/

void forwardsDfs(int current_node, std::vector<bool>& alreadyReached, const GreedyGraph& graph){
    for(int x : graph.forward_arcs[current_node]){
        if(!alreadyReached[x]){
            alreadyReached[x] = true;
            forwardsDfs(x, alreadyReached, graph);
        }
    }
}


void backwardsDFS(int current_node, std::vector<bool>& alreadyReached, const GreedyGraph& graph){
    for(int x : graph.backward_arcs[current_node]){
        if(!alreadyReached[x]){
            alreadyReached[x] = true;
            backwardsDFS(x, alreadyReached, graph);
        }
    }
}

std::list<Vertex> find_unreachables(const GreedyGraph& graph){
    std::vector<bool>  forwardReached(graph.nr_nodes,false);
    for(int x : graph.first_layer){
        if(!forwardReached[x]){
            forwardReached[x] = true;
            forwardsDfs(x, forwardReached, graph);
        }
    }

    std::vector<bool>  backwardReached(graph.nr_nodes,false);
    for(int x : graph.last_layer){
        if(!backwardReached[x]){
            backwardReached[x] = true;
            backwardsDFS(x, backwardReached, graph);
        }
    }
    std::list<int> unreachables;
    for(int i=0; i<graph.nr_nodes; i++){
        if( !(forwardReached[i] && backwardReached[i]) ){
            unreachables.push_back(i);
        }
    }
    return unreachables;
}

/*
void dependency_dfs(Vertex current_node, std::set<Vertex>& alreadyReached, const GreedyGraph& graph, GreedyAlgorithmVariables& vars){
    for(Vertex x : vars.node_implications[current_node]){
        if(alreadyReached.count(x)==0){
            alreadyReached.insert(x);
            dependency_dfs(x, alreadyReached, graph, vars);
        }
    }
}
*/

/*
void create_dependency_graph(GreedyGraph& graph, GreedyAlgorithmVariables& vars){
    vars.node_implications.resize(graph.nr_nodes);

    for(Vertex x : vars.current_node_set){
        if(graph.backward_arcs[x].size()==1){
            // the only path from the source to x goes through the destination of this edge,
            // therefore, if the dest gets removed, x must get removed as well.
            Vertex origin = *graph.backward_arcs[x].begin();
            vars.node_implications[origin].insert(x);
//            vars.reverse_node_implications[x].insert(origin)
        }
        if(graph.forward_arcs[x].size()==1){
            Vertex dest = *graph.forward_arcs[x].begin();
            vars.node_implications[dest].insert(x);
//           vars.reverse_node_implications[x].insert(dest);
        }
    }

// Most naive way: doing dfs for each node
// Not many edges anyways, so should be fine

    for( Vertex x : vars.current_node_set){
        std::set<Vertex> alreadyReached;
        alreadyReached.insert(x);
        dependency_dfs(x, alreadyReached, graph, vars);

        vars.node_implications[x].insert(alreadyReached.begin(),alreadyReached.end());
    }
    for(Vertex nb : vars.current_nb_set){
        std::set<Vertex> extra_nodes;
        for(Vertex x : graph.nb_to_nodes[nb]){
            extra_nodes.insert(vars.node_implications[x].begin(), vars.node_implications[x].end());
        }
        graph.nb_to_nodes[nb].insert(extra_nodes.begin(), extra_nodes.end());
        for(Vertex x : graph.nb_to_nodes[nb]){
            graph.node_to_nbs[x].insert(nb);
        }
    } 
}
*/

void implicationDFSIteration(Vertex current_node, std::vector<int>& forward_vis_count,
 std::vector<int>& backward_vis_count, const GreedyGraph& graph, const GreedyAlgorithmVariables& vars,
 const std::set<Vertex>& original_nodes, std::set<Vertex>& discovered_nodes){
    for(Vertex x : graph.forward_arcs[current_node]){
        if(original_nodes.count(x)+discovered_nodes.count(x)==0){
            forward_vis_count[x] += 1;
            if(forward_vis_count[x]==graph.backward_arcs[x].size()){
                discovered_nodes.insert(x); 
                implicationDFSIteration(x, forward_vis_count, backward_vis_count, graph, vars, original_nodes, discovered_nodes);
            }
        }
    }

    for(Vertex x : graph.backward_arcs[current_node]){
        if(original_nodes.count(x)+discovered_nodes.count(x)==0){
            backward_vis_count[x] += 1;
            if(backward_vis_count[x]==graph.forward_arcs[x].size()){
                discovered_nodes.insert(x); 
                implicationDFSIteration(x, forward_vis_count, backward_vis_count, graph, vars, original_nodes, discovered_nodes);
            }
        }
    }
}


std::set<Vertex> implicationDFS(Vertex current_nb, GreedyGraph& graph, GreedyAlgorithmVariables& vars){
    std::vector<int> forward_vis_count(graph.nr_nodes, 0);
    std::vector<int> backward_vis_count(graph.nr_nodes, 0);
    std::set<Vertex> discovered_nodes;
// We start searching from all the nodes that are already in the nb_to_nodes
    for(Vertex x : graph.nb_to_nodes[current_nb]){
        implicationDFSIteration(x, forward_vis_count, backward_vis_count, graph,vars, graph.nb_to_nodes[current_nb], discovered_nodes);
    }

//    implicationDFSIteration(current_nb, forward_vis_count, backward_vis_count, graph,vars, graph.nb_to_nodes[current_nb], discovered_nodes);


    return discovered_nodes;
}

GreedyAlgorithmVariables greedyInitialize(GreedyGraph& graph){
    std::vector<float> acc_neighbor_values;
    return greedyInitialize(graph, acc_neighbor_values);
}

GreedyAlgorithmVariables greedyInitialize(GreedyGraph& graph, const std::vector<float>& acc_neighbor_values){
    GreedyAlgorithmVariables vars;

    if(acc_neighbor_values.size()==0){
        vars.acc_neighbor_values = std::vector<float>(graph.nr_nodes, 0);
    } else {
        vars.acc_neighbor_values = acc_neighbor_values;
    }

    for( Vertex nb=0; nb<graph.nr_neighbors; nb++){
        vars.current_nb_set.insert(nb);
    }
    vars.sum_of_nodes = 0;
    for(Vertex x=0; x<graph.nr_nodes; x++){
        vars.current_node_set.insert(x);
        vars.sum_of_nodes += graph.node_values[x];
    }

//Remove the unreachables:
    std::list<Vertex> unreachables = find_unreachables(graph);

    for( Vertex x : unreachables){
        vars.current_node_set.erase(x);
        vars.sum_of_nodes -= graph.node_values[x];

        for(Vertex nb : graph.node_to_nbs[x]){
            graph.nb_to_nodes[nb].erase(x);
        }
        for(Vertex y : graph.forward_arcs[x]){
            graph.backward_arcs[y].erase(x);
        }
        for(Vertex y : graph.backward_arcs[x]){
            graph.forward_arcs[y].erase(x);
        }
    }

// At this point, we should be left with a graph where each node is reachable
// and each neighbor is connected to all the right direct nodes


//Updating the neighbor set:
    for(Vertex nb : vars.current_nb_set){
        std::set<Vertex> new_nodes = implicationDFS(nb,graph,vars);
        graph.nb_to_nodes[nb].insert(new_nodes.begin(), new_nodes.end());
        for(Vertex x : new_nodes){
            graph.node_to_nbs[x].insert(nb);
        }
    }

// setting the values
    for(Vertex nb : vars.current_nb_set){
        float sum = 0;
        for(Vertex x : graph.nb_to_nodes[nb]){
            sum += graph.node_values[x];
        }
        vars.nb_values_map[nb] = sum;
    }
    
    for(Vertex nb : vars.current_nb_set){
        vars.nb_values_set.insert({vars.nb_values_map[nb] + vars.acc_neighbor_values[nb],nb});
    }
    return vars;
}

void update_nb_value(Vertex nb, float diff, GreedyGraph& graph, GreedyAlgorithmVariables& vars){
    if(vars.nb_values_set.count({vars.nb_values_map[nb]+ vars.acc_neighbor_values[nb] ,nb})==0){
        std::cout << "Oops something went terribly wrong!" << std::endl;
    }
    vars.nb_values_set.erase({vars.nb_values_map[nb]+ vars.acc_neighbor_values[nb] ,nb});
    vars.nb_values_map[nb] += diff;
    vars.nb_values_set.insert({vars.nb_values_map[nb]+ vars.acc_neighbor_values[nb], nb});
}

void removeNode(Vertex chosen_node, GreedyGraph& graph, GreedyAlgorithmVariables& vars){
    vars.current_node_set.erase(chosen_node);
    vars.sum_of_nodes -= graph.node_values[chosen_node];
    for(Vertex nb : graph.node_to_nbs[chosen_node]){
        graph.nb_to_nodes[nb].erase(chosen_node);
        update_nb_value(nb, -graph.node_values[chosen_node], graph, vars);
    }

    for(Vertex y : graph.forward_arcs[chosen_node]){
        graph.backward_arcs[y].erase(chosen_node);
    }
    for(Vertex y : graph.backward_arcs[chosen_node]){
        graph.forward_arcs[y].erase(chosen_node);
    }
}

void removeNeighbor(Vertex chosen_nb, GreedyGraph& graph, GreedyAlgorithmVariables& vars ){
    std::set<Vertex> removed_nodes = graph.nb_to_nodes[chosen_nb];
    for(Vertex x : removed_nodes){
        graph.node_to_nbs[x].erase(chosen_nb);
        removeNode(x, graph, vars);

    }
}

void update_nb_sets(GreedyGraph& graph, GreedyAlgorithmVariables& vars){
    for(Vertex nb : vars.current_nb_set){
        std::set<Vertex> new_nodes = implicationDFS(nb,graph,vars);
        graph.nb_to_nodes[nb].insert(new_nodes.begin(), new_nodes.end());
        float diff = 0;
        for(Vertex x : new_nodes){
            graph.node_to_nbs[x].insert(nb);
            diff += graph.node_values[x];
        }
        update_nb_value(nb, diff, graph, vars);
    }
}

GreedySolution greedyAlgorithm(GreedyGraph graph, std::vector<float>& accNeighborValues){
    
    GreedyAlgorithmVariables vars = greedyInitialize(graph, accNeighborValues);
    
    

    float max_value = vars.sum_of_nodes/vars.current_nb_set.size();
    std::set<Vertex> best_neighbor_set = vars.current_nb_set;
    std::set<Vertex> best_node_set = vars.current_node_set;

    std::cout << "Initial max value = " << max_value << std::endl;

    while(vars.current_nb_set.size()>1){
//        std::cout << vars.current_nb_set.size() << std::endl;
//        std::cout << vars.nb_values_set.size() << std::endl;
        std::pair<float,Vertex> chosen_pair = *vars.nb_values_set.begin();
        Vertex chosen_nb = chosen_pair.second;
        vars.acc_neighbor_values[chosen_nb] += vars.nb_values_map[chosen_nb];
        vars.nb_values_set.erase(chosen_pair);
        vars.current_nb_set.erase(chosen_nb);
        
        removeNeighbor(chosen_nb, graph, vars);

        update_nb_sets(graph,vars);
        
//        std::cout << "Sum of nodes: " << vars.sum_of_nodes << std::endl;
//        std::cout << "Nb set size: " << vars.current_nb_set.size() << std::endl;

        if(vars.sum_of_nodes/vars.current_nb_set.size()>max_value){
            best_neighbor_set = vars.current_nb_set;
            best_node_set = vars.current_node_set;
            max_value = vars.sum_of_nodes/vars.current_nb_set.size();
//            std::cout << "new max value = " << max_value << std::endl;
        }
    }

    accNeighborValues = vars.acc_neighbor_values;

    return {max_value, best_node_set, best_neighbor_set};
}


void displayGraph(const GreedyGraph& graph, const GreedyAlgorithmVariables& vars, const MPGraphInterface& mpGraph, std::string fileName){
    displayGraphPNGNodesNbsAsInts(mpGraph, 1000, 600, vars.current_node_set, vars.current_nb_set, fileName);
}

GreedySolution greedyAlgorithmPrintIterations(GreedyGraph graph, std::vector<float>& accNeighborValues, const MPGraphInterface& mpGraph, bool printEachIteration){
    
    GreedyAlgorithmVariables vars = greedyInitialize(graph, accNeighborValues);
    
    

    float max_value = vars.sum_of_nodes/vars.current_nb_set.size();
    std::set<Vertex> best_neighbor_set = vars.current_nb_set;
    std::set<Vertex> best_node_set = vars.current_node_set;

    std::cout << "Initial max value = " << max_value << std::endl;

    int iteration = 0;

    displayGraph(graph, vars, mpGraph, "BeforeIterations" );

    while(vars.current_nb_set.size()>1){
        std::pair<float,Vertex> chosen_pair = *vars.nb_values_set.begin();
        Vertex chosen_nb = chosen_pair.second;
        bool nodes_changed = (graph.nb_to_nodes[chosen_nb].size()!=0);

        if(!printEachIteration && nodes_changed){
            displayGraph(graph, vars, mpGraph, "presentation/greedyPeeling/graphAtIteration" + std::to_string(iteration-1));
        }

        vars.acc_neighbor_values[chosen_nb] += vars.nb_values_map[chosen_nb];
        vars.nb_values_set.erase(chosen_pair);
        vars.current_nb_set.erase(chosen_nb);
        
        removeNeighbor(chosen_nb, graph, vars);

        update_nb_sets(graph,vars);
        
//        std::cout << "Sum of nodes: " << vars.sum_of_nodes << std::endl;
//        std::cout << "Nb set size: " << vars.current_nb_set.size() << std::endl;

        if(vars.sum_of_nodes/vars.current_nb_set.size()>max_value){
            best_neighbor_set = vars.current_nb_set;
            best_node_set = vars.current_node_set;
            max_value = vars.sum_of_nodes/vars.current_nb_set.size();
//            std::cout << "new max value = " << max_value << std::endl;
        }
        if(printEachIteration||nodes_changed){
            displayGraph(graph, vars, mpGraph, "presentation/greedyPeeling/graphAtIteration" + std::to_string(iteration));
        }
        iteration++;
    }

    accNeighborValues = vars.acc_neighbor_values;

    return {max_value, best_node_set, best_neighbor_set};
}




