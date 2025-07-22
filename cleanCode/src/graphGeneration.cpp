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

std::vector<Edge> totalEdgeVector(const std::vector<int>& partition){
   std::vector<Edge> edges;
   for(int i=0; i<partition.size()-1; i++){
      for(int vertex1 = 0; vertex1<partition[i]; vertex1++ ){
         for(int vertex2 = 0; vertex2<partition[i+1]; vertex2++){
            edges.push_back({{i,vertex1},{i+1,vertex2}});
         }
      }
   }
   return edges;
}

std::vector<Edge> generateEdges(const std::vector<int>& partition, int nrEdges, std::mt19937& gen){
   std::vector<Edge> edges = totalEdgeVector(partition);
   std::shuffle(edges.begin(),edges.end(),gen);
   edges.resize(std::min(nrEdges,int(edges.size())));
   return edges;
}

std::vector<Edge> generateEdgesRatio(const std::vector<int>& partition, float edgeRatio, std::mt19937& gen){
   std::vector<Edge> edges = totalEdgeVector(partition);
   std::shuffle(edges.begin(),edges.end(),gen);
   int nrEdges = edges.size()*edgeRatio;
   edges.resize(std::min(nrEdges,int(edges.size())));
   return edges;
}

float uniform_distribution_default(std::mt19937& gen){
   std::uniform_int_distribution<> dist(0,1024);
   return dist(gen);
}

std::map<VertexIndex,float> genWeights(const std::vector<int>& partition, std::function<float(std::mt19937&)> weightFunction, std::mt19937& gen){
   std::map<VertexIndex,float> weights;

   for(int i=0; i<partition.size(); i++){
      for(int x=0; x<partition[i]; x++){
         weights[{i,x}] = weightFunction(gen);
      }
   }

   return weights;
}

MultipartiteSetGraph generateGraph(int nrNodes, int nrLayers, int minLayerSize, int nrEdges, std::function<float(std::mt19937&)> weightFunction, std::mt19937& gen){
   std::vector<int> partition = generatePartitionMinSize(nrNodes,nrLayers, minLayerSize ,gen);
   std::vector<Edge> edgeVector = generateEdges(partition, nrEdges,gen);
   MultipartiteSetGraph graph;
   graph.layers = nrLayers;
   for(int i=0; i<partition.size(); i++){
      graph.elementsPerLayer[i] = partition[i];
      for(int x=0; x<partition[i]; x++){
         graph.values[{i,x}] = weightFunction(gen);
      }
   }
   graph.values = genWeights(partition, weightFunction, gen); // Is this superfluous?
   graph.edges.insert(edgeVector.begin(),edgeVector.end());

   return graph;
}

MultipartiteSetGraph generateGraphDoubleFixedEdges(int nrNodes, int nrLayers, float minLayerRatio, int nrEdges, float oddsSet1, std::function<float(std::mt19937&)> weightFunction1, std::function<float(std::mt19937&)> weightFunction2, unsigned int seed){
   std::mt19937 gen(seed);

   int minLayerSize = std::min(1.0f,minLayerRatio)*nrNodes/nrLayers;
   std::vector<int> partition = generatePartitionMinSize(nrNodes,nrLayers, minLayerSize ,gen);

   std::vector<Edge> edgeVector = generateEdges(partition, nrEdges,gen);
   MultipartiteSetGraph graph;
   graph.layers = nrLayers;
   for(int i=0; i<partition.size(); i++){
      graph.elementsPerLayer[i] = partition[i];
      for(int x=0; x<partition[i]; x++){
         if (x <= partition[i]*oddsSet1){
            graph.values[{i,x}] = weightFunction1(gen);
         } else {
            graph.values[{i,x}] = weightFunction2(gen);
         }
      }
   }
   graph.edges.insert(edgeVector.begin(),edgeVector.end());

   return graph;
}

MultipartiteSetGraph generateGraphEdgeRatio(int nrNodes, int nrLayers, int minLayerSize, float edgeRatio, std::function<float(std::mt19937&)> weightFunction, unsigned int seed){
   std::mt19937 gen(seed);
   std::vector<int> partition = generatePartitionMinSize(nrNodes,nrLayers, minLayerSize ,gen);
   std::vector<Edge> edgeVector= generateEdgesRatio(partition, edgeRatio, gen);
   
   MultipartiteSetGraph graph;
   graph.layers = nrLayers;
   for(int i=0; i<partition.size(); i++){
      graph.elementsPerLayer[i] = partition[i];
      for(int x=0; x<partition[i]; x++){
         graph.values[{i,x}] = weightFunction(gen);
      }
   }
   graph.values = genWeights(partition, weightFunction, gen);
   graph.edges.insert(edgeVector.begin(),edgeVector.end());

   return graph;
}


MultipartiteSetGraph generateDoubleGraph(int nrNodes, int nrLayers, float minLayerRatio, float oddsSet1,  float edgeRatio1, float edgeRatio2, float edgeRatioCross, std::function<float(std::mt19937&)> weightFunction1, std::function<float(std::mt19937&)> weightFunction2, unsigned int seed){
   std::mt19937 gen(seed);
   int minLayerSize = std::max(1.0f,minLayerRatio)*nrNodes/nrLayers;
   std::vector<int> partition = generatePartitionMinSize(nrNodes,nrLayers, minLayerSize ,gen);

   MultipartiteSetGraph graph;
   graph.layers = nrLayers;
   std::set<VertexIndex> vertexSet1;
   
   std::bernoulli_distribution coinSet1(oddsSet1);

   std::bernoulli_distribution coinEdgeBoth1(edgeRatio1);
   std::bernoulli_distribution coinEdgeBoth2(edgeRatio2);
   std::bernoulli_distribution coinEdgeDiff(edgeRatioCross);

   for(int i=0; i<partition.size(); i++){
      graph.elementsPerLayer[i] = partition[i];
      for(int x=0; x<partition[i]; x++){
         // bool inSet1 = coinSet1(gen);
         // if(inSet1){
         if(((float)x/partition[i])<=oddsSet1){
            vertexSet1.insert({i,x});
            graph.values[{i,x}] = weightFunction1(gen);
         } else {
            graph.values[{i,x}] = weightFunction2(gen);
         }
         if(i>0){
            for(int y=0; y<partition[i-1]; y++){
               bool hasEdge;
               if(vertexSet1.count({i,x}) && vertexSet1.count({i-1,y})){
                  hasEdge = coinEdgeBoth1(gen);
               } else if( (!vertexSet1.count({i,x})) && !(vertexSet1.count({i-1,y})) ){
                  hasEdge = coinEdgeBoth2(gen);
               } else {
                  hasEdge = coinEdgeDiff(gen);
               }
               if( hasEdge ){
                  graph.edges.insert({{i-1,y},{i,x}});
               }
            }
         }
      }
   }

   return graph;
}

// The "degree" is the expected average degree between two layers
MultipartiteSetGraph generateDoubleGraphDegree(int nrNodes, int nrLayers, float minLayerRatio, float oddsSet1,  float degree1, float degree2, float degreeCross, std::function<float(std::mt19937&)> weightFunction1, std::function<float(std::mt19937&)> weightFunction2, unsigned int seed){
   std::mt19937 gen(seed);
   int minLayerSize = std::min(1.0f,minLayerRatio)*nrNodes/nrLayers;
   std::vector<int> partition = generatePartitionMinSize(nrNodes,nrLayers, minLayerSize ,gen);

   MultipartiteSetGraph graph;
   graph.layers = nrLayers;
   std::set<VertexIndex> vertexSet1;
   
   std::bernoulli_distribution coinSet1(oddsSet1);


   graph.elementsPerLayer[0] = partition[0];
   for(int x=0; x<partition[0]; x++){
      if(((float)x/partition[0])<=oddsSet1){
         vertexSet1.insert({0,x});
         graph.values[{0,x}] = weightFunction1(gen);
      } else {
         graph.values[{0,x}] = weightFunction2(gen);
      }
   }

   for(int i=1; i<partition.size(); i++){
      graph.elementsPerLayer[i] = partition[i];
      int elts1 = partition[i]*oddsSet1;
      int elts2 = partition[i]-elts1;
      int elts1prev = partition[i-1]*oddsSet1;
      int elts2prev = partition[i-1]-elts1prev;
      // edgeRatio = (x+y)*degree/(2*x*y)
      auto getEdgeRatio = [](float x, float y, float degree){ return std::min(1.0f,((x+y)*degree)/(2*x*y));};
      std::bernoulli_distribution coinEdgeBoth1(getEdgeRatio(elts1prev,elts1,degree1));
      std::bernoulli_distribution coinEdgeBoth2(getEdgeRatio(elts2prev,elts2,degree2));
      std::bernoulli_distribution coinEdgeCross1(getEdgeRatio(elts1prev,elts2,degreeCross));
      std::bernoulli_distribution coinEdgeCross2(getEdgeRatio(elts2prev,elts1,degreeCross));

      for(int x=0; x<partition[i]; x++){
         if(((float)x/partition[i])<=oddsSet1){
            vertexSet1.insert({i,x});
            graph.values[{i,x}] = weightFunction1(gen);
         } else {
            graph.values[{i,x}] = weightFunction2(gen);
         }
         for(int y=0; y<partition[i-1]; y++){
            bool hasEdge;
            if(vertexSet1.count({i,x}) && vertexSet1.count({i-1,y})){
               hasEdge = coinEdgeBoth1(gen);
            } else if( (!vertexSet1.count({i,x})) && !(vertexSet1.count({i-1,y})) ){
               hasEdge = coinEdgeBoth2(gen);
            } else if(vertexSet1.count({i-1,y})){
               hasEdge = coinEdgeCross1(gen);
            } else {
               hasEdge = coinEdgeCross2(gen);
            }
            if( hasEdge ){
               graph.edges.insert({{i-1,y},{i,x}});
            }
         }
      }
   }

   return graph;
}


void storeGraph(const MultipartiteSetGraph& graph, unsigned int seed_to_generate, std::string fileName){
   std::ofstream outFile(fileName);
   if(!outFile){
      std::cout << "Failed to open file: " << fileName << std::endl;
      return;
   }
   outFile.clear();
   outFile << seed_to_generate << '\n';
   outFile << graph.getLayers() << '\n';
   for(int i=0; i<graph.getLayers(); i++){
      if(i!=0){
         outFile << ' ';
      }
      outFile << graph.getElementsInLayer(i);
   }
   outFile << '\n' << graph.edges.size() << "\n\n";


   for(int i=0; i<graph.getLayers(); i++){
      for(int j=0; j<graph.getElementsInLayer(i); j++){
         if(j!=0){
            outFile << ' ';
         }
         outFile << graph.getValue({i,j});
      }
      outFile << '\n';
   }
   
   for(auto e : graph.edges){
      VertexIndex v = e.first;
      VertexIndex w = e.second;
      outFile << '\n' << v.layer << ' ' << v.element << ' ' << w.element ;
   }
   outFile.close();
}


MultipartiteSetGraph readGraph(std::string fileName){
   std::ifstream inFile(fileName);
   MultipartiteSetGraph graph;
   if(!inFile){
      std::cout << "Failed to open file: " << fileName << std::endl;
      return graph;
   }
   int seed;
   inFile >> seed >> graph.layers;
   for(int i=0; i<graph.layers; i++){
      int eltsLayer;
      inFile >> eltsLayer;
      graph.elementsPerLayer[i] = eltsLayer;
   }
   int nrEdges; 
   inFile >> nrEdges;

   for(int i=0; i<graph.layers; i++){
      int eltsLayer = graph.elementsPerLayer[i];
      for(int j=0; j<eltsLayer; j++){
         float weight;
         inFile >> weight;
         graph.values[{i,j}] = weight;
      }
   }

   for(int i=0; i<nrEdges; i++){
      int layer, elt1, elt2;
      inFile >> layer >>  elt1 >>  elt2;
      graph.edges.insert({{layer,elt1},{layer+1,elt2}});
   }
   inFile.close();

   return graph;
}

MultipartiteSetGraph readGraph(std::string folderName, int index){
    std::string str = std::to_string(index);
    while(str.size()<4){
        str = "0"+str;
    }
    str =  "graph"+str+ ".txt";
   return readGraph(folderName + "/" + str);
}
