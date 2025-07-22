#include "../include/greedyWithDynPaths.h"


struct GreedyDynPathsVars{
    std::vector<std::vector<std::vector<int>>> forwardEdges;
    std::vector<std::vector<std::vector<int>>> backwardEdges;
    std::vector<std::vector<std::set<int>>> forwardEdgesNbs;
    std::vector<std::vector<std::set<int>>> backwardEdgesNbs;
    std::vector<std::vector<float>> weights;
    std::vector<bool> last_layer_already_nb;
    int nr_layers;
    std::vector<int> eltsPerLayer;
};

struct FloatInt{
    float sum_of_weights;
    int last_node;
};

GreedyDynPathsVars initializeGreedyDyn(const MPGraphInterface& mpGraph){
    GreedyDynPathsVars vars;

    vars.nr_layers = mpGraph.getLayers();
    vars.backwardEdges.resize(vars.nr_layers);
    vars.forwardEdges.resize(vars.nr_layers);
    vars.backwardEdgesNbs.resize(vars.nr_layers);
    vars.forwardEdgesNbs.resize(vars.nr_layers);

    for(int layer=0; layer<vars.nr_layers; layer++){
        int eltInLayer = mpGraph.getElementsInLayer(layer);
        vars.eltsPerLayer.push_back(eltInLayer);
        vars.forwardEdges[layer].resize(eltInLayer);
        vars.backwardEdges[layer].resize(eltInLayer);
        vars.forwardEdgesNbs[layer].resize(eltInLayer);
        vars.backwardEdgesNbs[layer].resize(eltInLayer);
    }

    std::unique_ptr<EdgeIterator> it = mpGraph.getEdgeIterator();

    while(it->has_next()){
        Edge e = it->current();
        VertexIndex vi1 = e.first;
        VertexIndex vi2 = e.second;
        vars.forwardEdges[vi1.layer][vi1.element].push_back(vi2.element);
        vars.backwardEdges[vi2.layer][vi2.element].push_back(vi1.element);
        it->next();
    }

    for(int layer=0; layer<vars.nr_layers; layer++){
        for(int elt=0; elt<vars.eltsPerLayer[layer]; elt++){
            vars.forwardEdgesNbs[layer][elt].insert(vars.forwardEdges[layer][elt].begin(),vars.forwardEdges[layer][elt].end());
            vars.backwardEdgesNbs[layer][elt].insert(vars.backwardEdges[layer][elt].begin(),vars.backwardEdges[layer][elt].end());
        }
    }
    
    for(int layer=0; layer<mpGraph.getLayers(); layer++){
        for(int element=0; element<mpGraph.getElementsInLayer(layer); element++){
            sort(vars.forwardEdges[layer][element].begin(),vars.forwardEdges[layer][element].end());
            sort(vars.backwardEdges[layer][element].begin(),vars.backwardEdges[layer][element].end());
        }
    }

    for(int layer=0; layer<vars.nr_layers; layer++)
        vars.eltsPerLayer.push_back(mpGraph.getElementsInLayer(layer));


    vars.weights.resize(vars.nr_layers);
    for(int layer=0; layer<vars.nr_layers; layer++){
        vars.weights[layer].resize(vars.eltsPerLayer[layer]);
        for(int x=0; x<vars.eltsPerLayer[layer]; x++){
            vars.weights[layer][x] = mpGraph.getValue({layer,x});
        }
    }
    vars.last_layer_already_nb.resize(vars.eltsPerLayer[vars.nr_layers-1],false);

    return vars;
}


// as input of the map at node at (layer,x), we take the number of the neighbors to the left of the current node (layer, x).
// then for each number of neighbors, we take the best weight for the path from the left to (layer, x) where we have
// exactly this number of nieghbors.

std::vector<std::vector<std::map<int,FloatInt>>> calcDPTable1dir(const GreedyDynPathsVars& vars){
    std::vector<std::vector<std::map<int,FloatInt>>> dp(vars.nr_layers);

    dp[0].resize(vars.eltsPerLayer[0]);
    for(int x=0; x<vars.eltsPerLayer[0];x++){
        dp[0][x][0]={vars.weights[0][x],0};
    }

    for(int layer=1; layer<vars.nr_layers; layer++){
        dp[layer].resize(vars.eltsPerLayer[layer]);
        for(int x=0; x<vars.eltsPerLayer[layer]; x++){
            int xLeftNbs = vars.backwardEdgesNbs[layer][x].size();
            int xWeight = vars.weights[layer][x];
            for(int y : vars.backwardEdges[layer][x]){
                for(auto mapResult:dp[layer-1][y]){
                    int totalNeighbors = mapResult.first+xLeftNbs;
                    float sum_of_weights_candidate = mapResult.second.sum_of_weights+xWeight;
                    if(dp[layer][x].count(totalNeighbors)==0||dp[layer][x][totalNeighbors].sum_of_weights < sum_of_weights_candidate){
                        dp[layer][x][totalNeighbors] = {sum_of_weights_candidate,y};
                    }
                }
            }
        }
    }
    return dp;

} 

PickedSet next_path(const std::vector<std::vector<std::map<int,FloatInt>>>& dp, GreedyDynPathsVars& vars, float original_sum_of_weights, int original_nr_nbs){
    PickedSet picked;

    int best_last_element = -1;
    float best_new_value = 0;
    int best_nr_new_nbs = 1;
    
    for(int x=0; x<vars.eltsPerLayer[vars.nr_layers-1]; x++){
        for(auto mapResult:dp[vars.nr_layers-1][x]){
            float sum_of_weights_cand = mapResult.second.sum_of_weights;
            int nr_nbs_can = mapResult.first;
            if(!vars.last_layer_already_nb[x]){
                nr_nbs_can += 1;
            }

            float new_value = (original_sum_of_weights+sum_of_weights_cand)/(original_nr_nbs+nr_nbs_can);

            if(sum_of_weights_cand!=0 && new_value>best_new_value){ 
                best_last_element = x;
                best_nr_new_nbs = nr_nbs_can;
                best_new_value = new_value;
            }
        }
    }

    if(best_last_element==-1){
        return picked;
    }


    picked.nodes.insert({vars.nr_layers-1,best_last_element});

    int current_element = best_last_element;
    int current_nr_nbs = best_nr_new_nbs;
    if(!vars.last_layer_already_nb[best_last_element]){
        current_nr_nbs -= 1;
        picked.nbs.insert({vars.nr_layers-1,best_last_element});
    }

    vars.last_layer_already_nb[best_last_element] = true;


    for(int layer = vars.nr_layers-1; layer>0; layer--){
        for(int y:vars.backwardEdgesNbs[layer][current_element]){
            picked.nbs.insert({layer-1,y});
        }
        int previous_node = dp[layer][current_element].at(current_nr_nbs).last_node;
        current_nr_nbs -= vars.backwardEdgesNbs[layer][current_element].size();
        current_element = previous_node;
        picked.nodes.insert({layer-1,current_element});
    }

    return picked;
}

void update_graph(GreedyDynPathsVars& vars, const PickedSet& picked){
    for(auto vi:picked.nodes){
        vars.weights[vi.layer][vi.element] = 0;
    }

    for(auto vi:picked.nbs){
        int layer = vi.layer;
        int y = vi.element;
        if(layer!=vars.nr_layers-1){
            for(auto x:vars.forwardEdges[layer][y]){
                vars.backwardEdgesNbs[layer+1][x].erase(y);
            }
        }
    }
}

void displayGraph(const std::set<VertexIndex>& picked_nodes_current, const std::set<VertexIndex>& picked_nbs_current, const MPGraphInterface& mpGraph, std::string fileName){
    displayGraphPNGNodesNbsPicked(mpGraph, 1000, 600, picked_nodes_current, picked_nbs_current, fileName);
}


GreedyWithDynPathsSol greedyDynPaths1dir(const MPGraphInterface& mpGraph, bool display_iterations){

    GreedyDynPathsVars  vars = initializeGreedyDyn(mpGraph);

    bool cont = true;

    float sum_of_weights_total = 0;
    int nr_nbs_total = 0;
    std::set<VertexIndex> picked_nodes_current;
    std::set<VertexIndex> picked_nbs_current;

    GreedyWithDynPathsSol best_solution;
    best_solution.value = 0;

    int iteration = 0;

    if(display_iterations){
        displayGraph(picked_nodes_current, picked_nbs_current, mpGraph, "initialGraph");
    }


    while(cont){
        std::vector<std::vector<std::map<int,FloatInt>>> dp = calcDPTable1dir(vars);

        PickedSet picked = next_path(dp, vars, sum_of_weights_total, nr_nbs_total);
        if(picked.nodes.size()==0){
            cont = false;
            break;
        }

        for(auto vi : picked.nodes){
            sum_of_weights_total += vars.weights[vi.layer][vi.element];
            picked_nodes_current.insert(vi);
        }

        update_graph(vars, picked);

        nr_nbs_total += picked.nbs.size();

        picked_nbs_current.insert(picked.nbs.begin(), picked.nbs.end());


        if(nr_nbs_total !=0 && sum_of_weights_total/nr_nbs_total>best_solution.value){
            best_solution.value = sum_of_weights_total/nr_nbs_total; 
            best_solution.picked_nodes = picked_nodes_current;
            best_solution.picked_nbs = picked_nbs_current;
        }

        if(display_iterations){
            displayGraph(picked_nodes_current, picked_nbs_current, mpGraph, "graphAtIteration" + std::to_string(iteration));
        }

        iteration++;
         
    }
    return best_solution; 
}




std::vector<std::vector<std::vector<std::map<int,FloatInt>>>>  calcDPTable2dir(const GreedyDynPathsVars& vars){
// dp[lay][y][z][nbs] Gives the best sum of weight of a path going through y at layer lay and z at layer lay+1
// where the number of neighbors upto and including layer lay through this path is exactly nbs.
// The last_node in the FloatInt is the node of this path in the lay-1'th layer.

    std::vector<std::vector<std::vector<std::map<int,FloatInt>>>> dp(vars.nr_layers-1);


    dp[0].resize(vars.eltsPerLayer[0],std::vector<std::map<int,FloatInt>>(vars.eltsPerLayer[1]));
    for(int z=0; z<vars.eltsPerLayer[1]; z++){
        int nr_nbs = vars.backwardEdgesNbs[1][z].size(); 
        for(int y : vars.backwardEdges[1][z]){
            dp[0][y][z][nr_nbs] = {vars.weights[0][y]+vars.weights[1][z],-1};
        }
    }

    for(int layer = 1; layer<vars.nr_layers-1; layer++){
        dp[layer].resize(vars.eltsPerLayer[layer],std::vector<std::map<int,FloatInt>>(vars.eltsPerLayer[layer+1]));
        for(int x=0; x<vars.eltsPerLayer[layer-1]; x++){
            std::set<int> elts_two_layers_further;
            for(int y : vars.forwardEdges[layer-1][x]){
                elts_two_layers_further.insert(vars.forwardEdges[layer][y].begin(), vars.forwardEdges[layer][y].end());
            }
            for(int z : elts_two_layers_further){
                const std::vector<int>& nodes_from_left = vars.forwardEdges[layer-1][x];
                const std::vector<int>& nodes_from_right = vars.backwardEdges[layer+1][z];
                std::vector<int> shared_middle_elts;
                int i=0;
                int j=0;
                //This works because we sorted the nbs in the adjacency lists
                while(i<nodes_from_left.size()&&j<nodes_from_right.size()){
                    if(nodes_from_left[i]==nodes_from_right[j]){
                        shared_middle_elts.push_back(nodes_from_left[i]);
                        i++;
                        j++;
                    } else if(nodes_from_left[i]<nodes_from_right[j]){
                        i++;
                    } else {
                        j++;
                    }
                }
                int nr_shared_nbs = 0;

                std::set<int>::iterator nbs_from_left = vars.forwardEdgesNbs[layer-1][x].begin();
                std::set<int>::iterator nbs_from_right = vars.backwardEdgesNbs[layer+1][z].begin();
                while(nbs_from_left != vars.forwardEdgesNbs[layer-1][x].end() && nbs_from_right != vars.backwardEdgesNbs[layer+1][z].end()){
                    if( *nbs_from_left == *nbs_from_right ){
                        nr_shared_nbs += 1;
                        nbs_from_left++;
                        nbs_from_right++;
                    } else if(*nbs_from_left<*nbs_from_right){
                        nbs_from_left++;
                    } else {
                        nbs_from_right++;
                    }
                }
                int extra_nbs = vars.forwardEdgesNbs[layer-1][x].size() + vars.backwardEdgesNbs[layer+1][z].size()-nr_shared_nbs;
                for(int y : shared_middle_elts){
                    for(auto fi : dp[layer-1][x][y]){
                        int new_total_nbs = fi.first+extra_nbs;
                        float new_weight = fi.second.sum_of_weights+vars.weights[layer+1][z];
                        if(dp[layer][y][z].count(new_total_nbs)==0 || new_weight>dp[layer][y][z][new_total_nbs].sum_of_weights){
                            dp[layer][y][z][new_total_nbs] = {new_weight,x};
                        }
                    }
                }
            }
        }
    }

    return dp;

} 

PickedSet next_path2dir(const std::vector<std::vector<std::vector<std::map<int,FloatInt>>>>& dp, GreedyDynPathsVars& vars, float original_sum_of_weights, int original_nr_nbs){
    PickedSet picked;

    float highest_path_value = 0;
    int best_x;
    int best_y;
    int best_z = -1;
    int remaining_nbs;

    for(int y = 0; y<vars.eltsPerLayer[vars.nr_layers-2]; y++){
        for(int z: vars.forwardEdges[vars.nr_layers-2][y]){
            for(auto fi : dp[vars.nr_layers-2][y][z]){
                int nr_nbs = fi.first + vars.forwardEdgesNbs[vars.nr_layers-2][y].size();
                float sum_of_weights = fi.second.sum_of_weights;
                if(sum_of_weights/nr_nbs > highest_path_value){
                    best_x = fi.second.last_node;
                    best_y = y;
                    best_z = z;
                    highest_path_value = sum_of_weights/nr_nbs;
                    remaining_nbs = fi.first;
                }
            }
        }
    }

    if(best_z == -1){
        return picked;
    }

    std::vector<int> sequence = {best_z,best_y,best_x};
    int current_x = best_x;
    int current_y = best_y;
    int current_z = best_z;
    for(int layer=vars.nr_layers-3; layer>0; layer--){

        std::set<int>::iterator nbs_from_left = vars.forwardEdgesNbs[layer][current_x].begin();
        std::set<int>::iterator nbs_from_right = vars.backwardEdgesNbs[layer+2][current_z].begin();

        remaining_nbs -= vars.forwardEdgesNbs[layer][current_x].size()+vars.backwardEdgesNbs[layer+2][current_z].size();

        //This works because we sorted the nbs in the adjacency lists
        while(nbs_from_left != vars.forwardEdgesNbs[layer][current_x].end() && nbs_from_right != vars.backwardEdgesNbs[layer+2][current_z].end()){
            if(*nbs_from_left == *nbs_from_right){
                remaining_nbs++;
                nbs_from_left++;
                nbs_from_right++;
            } else if(*nbs_from_left<*nbs_from_right){
                nbs_from_left++;
            } else {
                nbs_from_right++;
            }
        }

        int new_elt = dp[layer][current_x][current_y].at(remaining_nbs).last_node;
        sequence.push_back(new_elt);
        current_z = current_y;
        current_y = current_x;
        current_x = new_elt;
    }
    
    for(int layer=vars.nr_layers-1; layer>=0; layer--){
        int current_node = sequence[vars.nr_layers-1-layer];
        picked.nodes.insert({layer,current_node});
        if(layer<vars.nr_layers-1){
            for(int elt:vars.forwardEdgesNbs[layer][current_node]){
                picked.nbs.insert({layer+1,elt});
            }
        }
        if(layer>0){
            for(int elt:vars.backwardEdgesNbs[layer][current_node]){
                picked.nbs.insert({layer-1,elt});
            }
        }
    }

    return picked;
}

void update_graph2dir(GreedyDynPathsVars& vars, const PickedSet& picked){
    for(auto vi:picked.nodes){
        vars.weights[vi.layer][vi.element] = 0;
    }

    for(auto vi:picked.nbs){
        int layer = vi.layer;
        int y = vi.element;
        if(layer!=vars.nr_layers-1){
            for(auto x:vars.forwardEdges[layer][y]){
                vars.backwardEdgesNbs[layer+1][x].erase(y);
            }
        }
        if(layer!=0){
            for(auto x:vars.backwardEdges[layer][y]){
                vars.forwardEdgesNbs[layer-1][x].erase(y);
            }
        }
    }
}

GreedyWithDynPathsSol greedyDynPaths2dir(const MPGraphInterface& mpGraph, bool display_iterations){

    GreedyDynPathsVars  vars = initializeGreedyDyn(mpGraph);


    float sum_of_weights_total = 0;
    int nr_nbs_total = 0;
    std::set<VertexIndex> picked_nodes_current;
    std::set<VertexIndex> picked_nbs_current;

    GreedyWithDynPathsSol best_solution;
    best_solution.value = 0;

    int iteration = 0;

    if(display_iterations){
        displayGraph(picked_nodes_current, picked_nbs_current, mpGraph, "graphAtIteration" + std::to_string(iteration));
    }


    while(true){
        std::vector<std::vector<std::vector<std::map<int,FloatInt>>>> dp = calcDPTable2dir(vars);

        PickedSet picked = next_path2dir(dp, vars, sum_of_weights_total, nr_nbs_total);
        if(picked.nodes.size()==0){
            break;
        }

        for(auto vi : picked.nodes){
            sum_of_weights_total += vars.weights[vi.layer][vi.element];
            picked_nodes_current.insert(vi);
        }

        update_graph2dir(vars, picked);

        nr_nbs_total += picked.nbs.size();
        for( auto nb : picked.nbs){
            if(picked_nbs_current.count(nb)){
                std::cout << "The nb at " << nb << " is double added!\n";
            }
        }
        picked_nbs_current.insert(picked.nbs.begin(), picked.nbs.end());


        if(nr_nbs_total !=0 && sum_of_weights_total/nr_nbs_total>best_solution.value){
            best_solution.value = sum_of_weights_total/nr_nbs_total; 
            best_solution.picked_nodes = picked_nodes_current;
            best_solution.picked_nbs = picked_nbs_current;
        }

        if(display_iterations){
            displayGraph(picked_nodes_current, picked_nbs_current, mpGraph, "graphAtIteration" + std::to_string(iteration));
        }

        iteration++;
         
    }
    return best_solution; 
}

