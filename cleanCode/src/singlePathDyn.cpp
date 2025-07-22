#include "../include/singlePathDyn.h"

struct InterValue{
    float sum_of_weights;
    int nr_neighbors;
    int next_element;
};


void create_adjacency_lists(const MPGraphInterface& mpGraph, MPArcList& forwardEdges, MPArcList& backwardEdges){
    for(int layer=0; layer<mpGraph.getLayers(); layer++){
        int eltIntLayer = mpGraph.getElementsInLayer(layer);
        forwardEdges[layer] = std::vector<std::vector<int>>(eltIntLayer);
        backwardEdges[layer] = std::vector<std::vector<int>>(eltIntLayer);
    }

    std::unique_ptr<EdgeIterator> it = mpGraph.getEdgeIterator();

    while(it->has_next()){
        Edge e = it->current();
        VertexIndex vi1 = e.first;
        VertexIndex vi2 = e.second;
        forwardEdges[vi1.layer][vi1.element].push_back(vi2.element);
        backwardEdges[vi2.layer][vi2.element].push_back(vi1.element);

        it->next();
    }
    
    for(int layer=0; layer<mpGraph.getLayers(); layer++){
        for(int element=0; element<mpGraph.getElementsInLayer(layer); element++){
            sort(forwardEdges[layer][element].begin(),forwardEdges[layer][element].end());
            sort(backwardEdges[layer][element].begin(),backwardEdges[layer][element].end());
        }
    }
}


SinglePathDynSol singlePathDynNbs(const MPGraphInterface& mpGraph){
    MPArcList forwardEdges(mpGraph.getLayers());
    MPArcList backwardEdges(mpGraph.getLayers());

    create_adjacency_lists(mpGraph,forwardEdges,backwardEdges);

    int nr_layers = mpGraph.getLayers();
    std::vector<int> eltsPerLayer;
    for(int layer=0; layer<nr_layers; layer++)
        eltsPerLayer.push_back(mpGraph.getElementsInLayer(layer));


    struct FloatInt{
        float sum_of_weights;
        int last_node;
    };


    std::vector<std::vector<float>> weights(nr_layers);
    for(int layer=0; layer<nr_layers; layer++){
        weights[layer].resize(eltsPerLayer[layer]);
        for(int x=0; x<eltsPerLayer[layer]; x++){
            weights[layer][x] = mpGraph.getValue({layer,x});
        }
    }
// dp[lay][y][z][nbs] Gives the best sum of weight of a path going through y at layer lay and z at layer lay+1
// where the number of neighbors upto and including layer lay through this path is exactly nbs.
// The last_node in the FloatInt is the node of this path in the lay-1'th layer.

    std::vector<std::vector<std::vector<std::map<int,FloatInt>>>> dp;
    dp.resize(nr_layers - 1);
    for(int layer=0; layer<nr_layers-1; layer++){
        dp[layer].resize(eltsPerLayer[layer],std::vector<std::map<int,FloatInt>>(eltsPerLayer[layer+1]));
    }

    for(int z=0; z<eltsPerLayer[1]; z++){
        int nr_nbs = backwardEdges[1][z].size();
        for(int y : backwardEdges[1][z]){
            dp[0][y][z][nr_nbs] = {weights[0][y]+weights[1][z],-1};
        }
    }

    for(int layer = 1; layer<nr_layers-1; layer++){
        for(int x=0; x<eltsPerLayer[layer-1]; x++){
            std::set<int> elts_two_layers_further;
            for(int y : forwardEdges[layer-1][x]){
                elts_two_layers_further.insert(forwardEdges[layer][y].begin(), forwardEdges[layer][y].end());
            }
            for(int z : elts_two_layers_further){
                std::vector<int>& nbs_from_left = forwardEdges[layer-1][x];
                std::vector<int>& nbs_from_right = backwardEdges[layer+1][z];
                std::vector<int> shared_middle_elts;
                int i=0;
                int j=0;
                //This works because we sorted the nbs in the adjacency lists
                while(i<nbs_from_left.size()&&j<nbs_from_right.size()){
                    if(nbs_from_left[i]==nbs_from_right[j]){
                        shared_middle_elts.push_back(nbs_from_left[i]);
                        i++;
                        j++;
                    } else if(nbs_from_left[i]<nbs_from_right[j]){
                        i++;
                    } else {
                        j++;
                    }
                }
                int extra_nbs = nbs_from_left.size()+nbs_from_right.size()-shared_middle_elts.size();
                for(int y : shared_middle_elts){
                    for(auto fi : dp[layer-1][x][y]){
                        int new_total_nbs = fi.first+extra_nbs;
                        float new_weight = fi.second.sum_of_weights+weights[layer+1][z];
                        if(dp[layer][y][z].count(new_total_nbs)==0 || new_weight>dp[layer][y][z][new_total_nbs].sum_of_weights){
                            dp[layer][y][z][new_total_nbs] = {new_weight,x};
                        }
                    }
                }
            }
        }
    }

    float highest_path_value = 0;
    int best_x=-1;
    int best_y=-1;
    int best_z=-1;
    int remaining_nbs;
    for(int y = 0; y<eltsPerLayer[nr_layers-2]; y++){
        for(int z: forwardEdges[nr_layers-2][y]){
            for(auto fi : dp[nr_layers-2][y][z]){
                int nr_nbs = fi.first + forwardEdges[nr_layers-2][y].size();
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
    if(best_z==-1){
        return {0,{},{}};
    }

    SinglePathDynSol solution;
    solution.max_value=highest_path_value;
    std::vector<int> sequence = {best_z,best_y,best_x};
    int current_x = best_x;
    int current_y = best_y;
    int current_z = best_z;
    for(int layer=nr_layers-3; layer>0; layer--){
        std::vector<int>& nbs_from_left = forwardEdges[layer][current_x];
        std::vector<int>& nbs_from_right = backwardEdges[layer+2][current_z];
        remaining_nbs -= nbs_from_left.size()+nbs_from_right.size();
        int i=0;
        int j=0;
        //This works because we sorted the nbs in the adjacency lists
        while(i<nbs_from_left.size()&&j<nbs_from_right.size()){
            if(nbs_from_left[i]==nbs_from_right[j]){
                remaining_nbs++;
                i++;
                j++;
            } else if(nbs_from_left[i]<nbs_from_right[j]){
                i++;
            } else {
                j++;
            }
        }
        int new_elt = dp[layer][current_x][current_y][remaining_nbs].last_node;
        sequence.push_back(new_elt);
        current_z = current_y;
        current_y = current_x;
        current_x = new_elt;
    }
    
    for(int layer=nr_layers-1; layer>=0; layer--){
        int current_node = sequence[nr_layers-1-layer];
        solution.picked_nodes.insert({layer,current_node});
        if(layer<nr_layers-1){
            for(int elt:forwardEdges[layer][current_node]){
                solution.picked_neighbors.insert({layer+1,elt});
            }
        }
        if(layer>0){
            for(int elt:backwardEdges[layer][current_node]){
                solution.picked_neighbors.insert({layer-1,elt});
            }
        }
    }
    return solution;
}



SinglePathDynSol singlePathDyn1DirNbs(const MPGraphInterface& mpGraph){
    MPArcList forwardEdges(mpGraph.getLayers());
    MPArcList backwardEdges(mpGraph.getLayers());

    create_adjacency_lists(mpGraph,forwardEdges,backwardEdges);

    int nr_layers = mpGraph.getLayers();
    std::vector<int> eltsPerLayer;
    for(int layer=0; layer<nr_layers; layer++)
        eltsPerLayer.push_back(mpGraph.getElementsInLayer(layer));

    struct FloatInt{
        float sum_of_weights;
        int last_node;
    };

    std::vector<std::vector<float>> weights(nr_layers);
    for(int layer=0; layer<nr_layers; layer++){
        weights[layer].resize(eltsPerLayer[layer]);
        for(int x=0; x<eltsPerLayer[layer]; x++){
            weights[layer][x] = mpGraph.getValue({layer,x});
        }
    }


// The dp actually only needs nr_layers-1, but then we would have to add -1 each time
// to get the correct layer.
    std::vector<std::vector<std::map<int,FloatInt>>> dp(nr_layers);

    dp[0].resize(eltsPerLayer[0]);
    for(int x=0; x<eltsPerLayer[0];x++){ 
        dp[0][x][1]={weights[0][x],0};
    }

    for(int layer=1; layer<nr_layers; layer++){
        dp[layer].resize(eltsPerLayer[layer]);
        for(int x=0; x<eltsPerLayer[layer]; x++){
            int xLeftNbs = backwardEdges[layer][x].size();
            int xWeight = weights[layer][x];
            for(int y : backwardEdges[layer][x]){
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

    int best_last_element = -1;
    float best_weight = 0;
    int best_nr_nbs = 1;

    for(int x=0; x<eltsPerLayer[nr_layers-1]; x++){
        for(auto mapResult:dp[nr_layers-1][x]){
            float sum_of_weights_cand = mapResult.second.sum_of_weights;
            int nr_nbs_can = mapResult.first;
            if(best_weight/best_nr_nbs < sum_of_weights_cand/nr_nbs_can){
                best_last_element = x;
                best_nr_nbs = nr_nbs_can;
                best_weight = sum_of_weights_cand;
            }
        }
    }
    SinglePathDynSol sol;
    if(best_last_element==-1){
        return {0,{},{}};
    }



    sol.max_value = best_weight/best_nr_nbs;
    sol.picked_nodes.insert({nr_layers-1,best_last_element});

    int current_element = best_last_element;
    int current_nr_nbs = best_nr_nbs;
    for(int layer = nr_layers-1; layer>0; layer--){
        for(int y:backwardEdges[layer][current_element]){
            sol.picked_neighbors.insert({layer-1,y});
        }
        int previous_node = dp[layer][current_element][current_nr_nbs].last_node;
        current_nr_nbs -= backwardEdges[layer][current_element].size();
        current_element = previous_node;
        sol.picked_nodes.insert({layer-1,current_element});
    }
    return sol;
}
