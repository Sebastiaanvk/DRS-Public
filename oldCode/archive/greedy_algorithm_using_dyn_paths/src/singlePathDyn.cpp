#include "../include/singlePathDyn.h"

struct InterValue{
    float sum_of_weights;
    int nr_neighbors;
    int next_element;
};



bool checkSinglePath(const MPGraphInterface& mpgraph, const SinglePathDynSol& sol, bool twoDir){
    std::unique_ptr<EdgeIterator> it = mpgraph.getEdgeIterator();
    int nr_layers = mpgraph.getLayers();
    float sum_of_weights = 0;
    std::vector<bool> layer_picked(nr_layers,false);
    for(auto vi : sol.picked_nodes){
        layer_picked[vi.layer] = true;
        sum_of_weights += mpgraph.getValue(vi);
    }
    for(bool b : layer_picked){
        if(!b){
            std::cout << "Not each layer got picked!" << std::endl;
            return false;
        }
    }

    std::vector<bool> layer_has_forward_edge(nr_layers-1,false);
    std::set<VertexIndex> neighbor_set;
    neighbor_set.insert(sol.picked_nodes.begin(),sol.picked_nodes.end());
    while(it->has_next()){
        Edge e = it->current();
        VertexIndex vi1 = e.first;
        VertexIndex vi2 = e.second;
        if(sol.picked_nodes.count(vi1) && sol.picked_nodes.count(vi2)){
            layer_has_forward_edge[std::min(vi1.layer,vi2.layer)] = true;
        } else if(sol.picked_nodes.count(vi1) || sol.picked_nodes.count(vi2)){
            VertexIndex node = vi1;
            VertexIndex nb = vi2;
            if(sol.picked_nodes.count(vi2)){
                node = vi2;
                nb = vi1;
            }
            if(node.layer>nb.layer || twoDir){
               neighbor_set.insert(nb);
            } 
        } 
        it->next();
    }
    for(bool b : layer_has_forward_edge){
        if(!b){
            std::cout << "Not a path!" << std::endl;
            return false;
        }
    }
    if(neighbor_set!=sol.picked_neighbors){
        std::cout << "The neighbor set is incorrect!" << std::endl;
    }
    std::cout << "The node and neighbor set appear to be correct. The given value is: " << sol.max_value; 
    std::cout << " and recalculated value is: " << sum_of_weights/neighbor_set.size() << std::endl;
    return true;
}

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


SinglePathDynSol singlePathDynHeur(const MPGraphInterface& mpGraph){
    MPArcList forwardEdges(mpGraph.getLayers());
    MPArcList backwardEdges(mpGraph.getLayers());

    create_adjacency_lists(mpGraph,forwardEdges,backwardEdges);

    int nr_layers = mpGraph.getLayers();
    std::vector<int> eltsPerLayer;
    for(int layer=0; layer<nr_layers; layer++)
        eltsPerLayer.push_back(mpGraph.getElementsInLayer(layer));

    std::vector<std::vector<InterValue>> dp(mpGraph.getLayers()-1);

    dp[nr_layers-2] = std::vector<InterValue>(eltsPerLayer[nr_layers-2]*eltsPerLayer[nr_layers-1],{0,0,0});
    for(int elt1=0; elt1<eltsPerLayer[nr_layers-2]; elt1++){
        for(int elt2:forwardEdges[nr_layers-2][elt1]){
            dp[nr_layers-2][elt1*eltsPerLayer[nr_layers-1]+elt2] = {mpGraph.getValue({nr_layers-2,elt1})+mpGraph.getValue({nr_layers-1,elt2}),(int)forwardEdges[nr_layers-2][elt1].size(),-1};
        }
    }


    for(int layer = mpGraph.getLayers()-3; layer>=0; layer--){
        int nr_elts1 = eltsPerLayer[layer];
        int nr_elts2 = eltsPerLayer[layer+1];
        int nr_elts3 = eltsPerLayer[layer+2];

// dp[layer][elt1*eltsPerLayer(layer+1)+elt2] will give the value for the pair where we pick elt1 and elt2 in the next layer
// The value is given by the sum of the weights of the best path going through those points to the right and we look at
// the nr of neighbors that are in the layers starting at layer+1 and going to the right.

        dp[layer] = std::vector<InterValue>(nr_elts1*nr_elts2,{0,0,0});
        for(int elt1=0; elt1<nr_elts1; elt1++){
            for(int elt3=0; elt3<nr_elts3; elt3++){
                int i=0;
                int j=0;
                std::vector<int> shared_middle_elts;
                while(i<forwardEdges[layer][elt1].size()&&j<backwardEdges[layer+2][elt3].size()){
                    if(forwardEdges[layer][elt1][i]==backwardEdges[layer+2][elt3][j]){
                        shared_middle_elts.push_back(forwardEdges[layer][elt1][i]);
                        i++;
                        j++;
                    } else if(forwardEdges[layer][elt1][i]<backwardEdges[layer+2][elt3][j]){
                        i++;
                    } else {
                        j++;
                    }
                }

                for(int elt2:shared_middle_elts){
                    InterValue next_layer_value = dp[layer+1][elt2*nr_elts3+elt3];
                    if(next_layer_value.nr_neighbors>0){
                        InterValue current_value = dp[layer][elt1*nr_elts2+elt2];
                        float sum_of_weights = next_layer_value.sum_of_weights;
                        if(layer>0)
                            sum_of_weights += mpGraph.getValue({layer,elt1});
                        int extra_nbs = forwardEdges[layer][elt1].size()+backwardEdges[layer+2][elt3].size()-shared_middle_elts.size(); 
                        int nr_nbs = next_layer_value.nr_neighbors+extra_nbs;
                        if(current_value.nr_neighbors<=0 || sum_of_weights/nr_nbs>current_value.sum_of_weights/current_value.nr_neighbors) {
                            dp[layer][elt1*nr_elts2+elt2] = {sum_of_weights,nr_nbs,elt3};
                        }
                    }
                }
            }
        }
    }

    float highest_path_value = 0;
    int best_first_elt;
    int best_second_elt;
    for(int first_element = 0; first_element<eltsPerLayer[0]; first_element++){
        for(int second_element: forwardEdges[0][first_element]){
            InterValue iv = dp[0][first_element*eltsPerLayer[0]+second_element];
            int nr_neighbors = iv.nr_neighbors + backwardEdges[1][second_element].size();
            if(iv.nr_neighbors>0 && iv.sum_of_weights/nr_neighbors>highest_path_value){
                best_first_elt = first_element;
                best_second_elt = second_element;
                highest_path_value = iv.sum_of_weights/nr_neighbors;

            }
        }
    }

    SinglePathDynSol solution;
    solution.max_value=highest_path_value;
    std::vector<int> sequence;
    int current_node = best_first_elt;
    int next_node = best_second_elt;
    sequence.push_back(current_node);
    solution.picked_nodes.insert({0,current_node});
    for(int layer=0; layer<dp.size(); layer++){
        sequence.push_back(next_node);
        solution.picked_nodes.insert({layer+1,next_node});
        int next_next_node = dp[layer][current_node*eltsPerLayer[layer+1]+next_node].next_element;
        current_node=next_node;
        next_node = next_next_node;
    }
    for(int layer=0; layer<nr_layers-1; layer++){
        for(int elt:forwardEdges[layer][sequence[layer]]){
            solution.picked_neighbors.insert({layer+1,elt});
        }
    }
    for(int layer=1; layer<nr_layers; layer++){
        for(int elt:backwardEdges[layer][sequence[layer]]){
            solution.picked_neighbors.insert({layer-1,elt});
        }
    }

    return solution;
}



SinglePathDynSol singlePathDynNbs(const MPGraphInterface& mpGraph){
    MPArcList forwardEdges(mpGraph.getLayers());
    MPArcList backwardEdges(mpGraph.getLayers());

    create_adjacency_lists(mpGraph,forwardEdges,backwardEdges);

    int nr_layers = mpGraph.getLayers();
    std::vector<int> eltsPerLayer;
    for(int layer=0; layer<nr_layers; layer++)
        eltsPerLayer.push_back(mpGraph.getElementsInLayer(layer));

//    std::vector<std::vector<InterValue>> dp(mpGraph.getLayers()-1);

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
    int best_x;
    int best_y;
    int best_z;
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

    int best_last_element;
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



struct BfvarsSingle{
    int nr_layers;
    MPArcList forwardEdges;
    MPArcList backwardEdges;
    std::vector<std::vector<float>> weights;
    std::vector<int> current_path;
    int current_nr_nbs;
    float current_path_weight;
    float best_value = 0;
    std::vector<int> best_path;
};

void singlePath1DirNbsBruteForceDFSIteration(int layer, int node, BfvarsSingle& bfvars){
    if( layer==bfvars.nr_layers-1){
        if(bfvars.current_path_weight/bfvars.current_nr_nbs > bfvars.best_value){
            bfvars.best_value = bfvars.current_path_weight/bfvars.current_nr_nbs;
            bfvars.best_path = bfvars.current_path;
        }
        return;
    }
    for(int y : bfvars.forwardEdges[layer][node] ){
        bfvars.current_path.push_back(y);
        bfvars.current_path_weight += bfvars.weights[layer+1][y];
        bfvars.current_nr_nbs += bfvars.backwardEdges[layer+1][y].size();

        singlePath1DirNbsBruteForceDFSIteration(layer+1, y, bfvars);

        bfvars.current_path.pop_back();
        bfvars.current_path_weight -= bfvars.weights[layer+1][y];
        bfvars.current_nr_nbs -= bfvars.backwardEdges[layer+1][y].size();
    }
}

SinglePathDynSol singlePath1DirNbsBruteForce(const MPGraphInterface& mpGraph){
    BfvarsSingle bfvars;
    bfvars.forwardEdges.resize(mpGraph.getLayers());
    bfvars.backwardEdges.resize(mpGraph.getLayers());

    create_adjacency_lists(mpGraph,bfvars.forwardEdges,bfvars.backwardEdges);

    bfvars.nr_layers = mpGraph.getLayers();

    bfvars.weights.resize(bfvars.nr_layers);
    for(int layer=0; layer<bfvars.nr_layers; layer++){
        bfvars.weights[layer].resize(mpGraph.getElementsInLayer(layer));
        for(int x=0; x<mpGraph.getElementsInLayer(layer); x++){
            bfvars.weights[layer][x] = mpGraph.getValue({layer,x});
        }
    }

    bfvars.best_value=0;
    
    for(int x=0; x<mpGraph.getElementsInLayer(1); x++){
        int best_left_neighbor = -1;
        float best_left_neighbor_weight = -1;
        for(int y : bfvars.backwardEdges[1][x] ){
            if(bfvars.weights[0][y]>best_left_neighbor_weight){
                best_left_neighbor = y;
                best_left_neighbor_weight = bfvars.weights[0][y];
            }
        }
        if(best_left_neighbor>=0){
            bfvars.current_nr_nbs = bfvars.backwardEdges[1][x].size()+1;
            bfvars.current_path_weight = best_left_neighbor_weight+bfvars.weights[1][x];
            bfvars.current_path = {best_left_neighbor,x};
            singlePath1DirNbsBruteForceDFSIteration(1,x,bfvars);
        }
    }

    SinglePathDynSol sol;
    sol.max_value = bfvars.best_value;
    sol.picked_nodes.insert({0,bfvars.best_path[0]});
    for(int layer=1; layer<bfvars.nr_layers; layer++){
        int node = bfvars.best_path[layer];
        sol.picked_nodes.insert({layer,node});
        for(int y : bfvars.backwardEdges[layer][node]){
            sol.picked_neighbors.insert({layer-1,y});
        }
    }
    return sol;
}



void singlePathBruteForceDFSIteration(int layer, int node, BfvarsSingle& bfvars){
    if( layer==bfvars.nr_layers-1){
        int extra_nbs = bfvars.forwardEdges[layer-1][bfvars.current_path[layer-1]].size();
        if(bfvars.current_path_weight/(bfvars.current_nr_nbs + extra_nbs) > bfvars.best_value){
            bfvars.best_value = bfvars.current_path_weight/(bfvars.current_nr_nbs + extra_nbs);
            bfvars.best_path = bfvars.current_path;
        }
        return;
    }
    for(int y : bfvars.forwardEdges[layer][node] ){
        bfvars.current_path.push_back(y);
        bfvars.current_path_weight += bfvars.weights[layer+1][y];

        int extra_nbs;
        if(layer == 0){
            extra_nbs = bfvars.backwardEdges[1][y].size();
        } else {
            std::vector<int>& nbs_from_left = bfvars.forwardEdges[layer-1][bfvars.current_path[layer-1]];
            std::vector<int>& nbs_from_right = bfvars.backwardEdges[layer+1][y];
            extra_nbs = nbs_from_left.size()+nbs_from_right.size();
            int i=0;
            int j=0;
            //This works because we sorted the nbs in the adjacency lists
            while(i<nbs_from_left.size()&&j<nbs_from_right.size()){
                if(nbs_from_left[i]==nbs_from_right[j]){
                    extra_nbs--;
                    i++;
                    j++;
                } else if(nbs_from_left[i]<nbs_from_right[j]){
                    i++;
                } else {
                    j++;
                }
            }
        }

        bfvars.current_nr_nbs += extra_nbs;

        singlePathBruteForceDFSIteration(layer+1, y, bfvars);

        bfvars.current_path.pop_back();
        bfvars.current_path_weight -= bfvars.weights[layer+1][y];
        bfvars.current_nr_nbs -= extra_nbs;
    }
}


SinglePathDynSol singlePathBruteForce(const MPGraphInterface& mpGraph){
    BfvarsSingle bfvars;
    bfvars.forwardEdges.resize(mpGraph.getLayers());
    bfvars.backwardEdges.resize(mpGraph.getLayers());

    create_adjacency_lists(mpGraph,bfvars.forwardEdges,bfvars.backwardEdges);

    bfvars.nr_layers = mpGraph.getLayers();

    bfvars.weights.resize(bfvars.nr_layers);
    for(int layer=0; layer<bfvars.nr_layers; layer++){
        bfvars.weights[layer].resize(mpGraph.getElementsInLayer(layer));
        for(int x=0; x<mpGraph.getElementsInLayer(layer); x++){
            bfvars.weights[layer][x] = mpGraph.getValue({layer,x});
        }
    }

    bfvars.best_value=0;
    
    for(int x=0; x<mpGraph.getElementsInLayer(0); x++){
        bfvars.current_nr_nbs = 0;
        bfvars.current_path_weight = bfvars.weights[0][x];
        bfvars.current_path = {x};
        singlePathBruteForceDFSIteration(0,x,bfvars);
    }

    SinglePathDynSol sol;
    sol.max_value = bfvars.best_value;
    sol.picked_nodes.insert({0,bfvars.best_path[0]});

    for(int layer=0; layer<bfvars.nr_layers; layer++){
        int node = bfvars.best_path[layer];
        sol.picked_nodes.insert({layer,node});
        if(layer>0){
            for(int y : bfvars.backwardEdges[layer][node]){
                sol.picked_neighbors.insert({layer-1,y});
            }
        }
        if(layer<bfvars.nr_layers-1){
            for(int y : bfvars.forwardEdges[layer][node]){
                sol.picked_neighbors.insert({layer+1,y});
            }
        }
    }
    return sol;
}
