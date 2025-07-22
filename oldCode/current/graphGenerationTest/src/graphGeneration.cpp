#include "../include/graphGeneration.h"

std::vector<int> generatePartition(int n, int k, std::mt19937& gen){
   std::uniform_int_distribution<int> dist(1,n+k-1);

   std::unordered_set<int> bars;

   while (bars.size()<k-1){
      int nextBar = dist(gen);
      bars.insert(nextBar);
   }

   std::vector<int> barVec(bars.begin(),bars.end());
   barVec.push_back(0);
   barVec.push_back(n+k);
   
   std::sort(barVec.begin(),barVec.end());

   std::vector<int> partition;
   for(int i=0; i<k; i++){
      partition.push_back(barVec[i+1]-barVec[i]-1);
   }   
   return partition;
}


std::vector<int> generatePartitionMinSize(int n, int k, int minSize, std::mt19937& gen){
   std::vector<int> parts = generatePartition(n-k*minSize,k,gen);
   for(int i=0; i<parts.size(); i++){
      parts[i] += minSize;
   }
   return parts;
}



std::vector<Edge> generateEdges(const std::vector<int>& partition, int nrEdges, std::mt19937& gen){
   std::vector<Edge> edges;
   for(int i=0; i<partition.size()-1; i++){
      for(int vertex1 = 0; vertex1<partition[i]; vertex1++ ){
         for(int vertex2 = 0; vertex2<partition[i+1]; vertex2++){
            edges.push_back({{i,vertex1},{i+1,vertex2}});
         }
      }
   }
   std::shuffle(edges.begin(),edges.end(),gen);
   edges.resize(std::min(nrEdges,int(edges.size())));
   return edges;
}

float uniform_distribution_default(std::mt19937& gen){
   std::uniform_int_distribution<> dist(0,1024);
   return dist(gen);
}

MultipartiteSetGraph generateGraph(int nrNodes, int nrLayers, int minLayerSize, int nrEdges, std::function<float(std::mt19937&)> weightFunction, std::mt19937& gen){
   std::vector<int> partition = generatePartitionMinSize(nrNodes,nrLayers, minLayerSize ,gen);
   std::vector<Edge> edgeVector = generateEdges(partition, nrEdges,gen);
   MultipartiteSetGraph graph;
   graph.layers = nrLayers;
   for(int i=0; i<nrLayers; i++){
      graph.elementsPerLayer[i] = partition[i];
      for(int x=0; x<partition[i]; x++){
         graph.values[{i,x}] = weightFunction(gen);
      }
   }
   graph.edges.insert(edgeVector.begin(),edgeVector.end());

   return graph;
}

