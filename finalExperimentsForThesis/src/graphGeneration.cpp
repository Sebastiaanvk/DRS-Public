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
   // graph.values = genWeights(partition, weightFunction, gen);
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
      // bool inSet1 = coinSet1(gen);
      // if(inSet1){
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

void saveGraphsUniformCoinflip1(unsigned int seed, 
    std::vector<int> nrLayers, std::vector<int> nrNodes,
    std::vector<float> edgesRatios, std::vector<float> weightCoinFlipOdds,
    int minEltsPerLayer, int maxWeight, std::string folderPath){

   size_t size1 = nrLayers.size();
   size_t size2 = nrNodes.size();
   size_t size3 = edgesRatios.size();
   size_t size4 = weightCoinFlipOdds.size();


   

   // Parallelize the loops using OpenMP
//   #pragma omp parallel for collapse(4) // Beter niet gebruiken omdat std::mt19937 niet thread safe is :(

   for(size_t i=0; i<size1; i++){
      for(size_t j=0; j<size2; j++){
         for(size_t k=0; k<size3; k++){
            for(size_t l=0; l<size4; l++){
               int index = ((i*size2+j)*size3+k)*size4+l;
               float coinOdds = weightCoinFlipOdds[l];

               std::function<float(std::mt19937&)> weightFunction = [maxWeight, coinOdds](std::mt19937& gen) -> float {
                  std::uniform_int_distribution<> dist(0,maxWeight);
                  std::bernoulli_distribution distb(coinOdds);
                  if(distb(gen))
                        return dist(gen);
                  else
                        return 0;
               };
            
               
               MultipartiteSetGraph graph = generateGraphEdgeRatio(nrNodes[j],nrLayers[i],minEltsPerLayer,edgesRatios[k], weightFunction, index+seed);
               std::string str = std::to_string(index);
               while (str.length() < 4) {
                  str = '0' + str;  
               }
               storeGraph(graph,index+seed,folderPath + "/graph"+ str + ".txt");
            }
         }
      }
   }
}

void saveGraphsUniformCoinflip1DefaultValues(unsigned int seed){
//   std::vector<int> nrLayers = {3,4,5,6,7,8,9,10};
   std::vector<int> nrLayers = {3,4,5,6,7,8};

//   std::vector<int> nrNodes = {80,100,120,140,160,180,200};
//   std::vector<int> nrNodes = {80,100,120,140,160};
//   std::vector<int> nrNodes = {100,120,140,160,180,200,220,240,260,280};
   std::vector<int> nrNodes = {45,60,75,90,105,120,135,150,165,180,195};
//   std::vector<float> edgesRatios = {0.20,0.30,0.40,0.50,0.60,0.70,0.80};
   std::vector<float> edgesRatios = {0.10,0.20,0.30,0.40,0.50,0.60};
   std::vector<float> weightCoinFlipOdds = {0.20,0.40,0.60,0.80};

   int minEltsPerLayer = 5;
   int maxWeight = 1024;

   std::string folderPath = "data/graphs/forAccuracyTwentyPerc";

   saveGraphsUniformCoinflip1(seed, nrLayers, nrNodes, edgesRatios, weightCoinFlipOdds, minEltsPerLayer, maxWeight, folderPath);
}

void saveGraphsConstVarsUniformEdges(unsigned int seed, int nrLayers, int nrNodes,
    float minEdgesRatios,float maxEdgesRatios, float minWeightCoinFlipOdds,float maxWeightCoinFlipOdds,
    int minEltsPerLayer, int maxWeight, std::string folderPath, int nrGraphs){
   
   std::random_device rd;
   std::default_random_engine gen(rd());

   for(int index=0; index<nrGraphs; index++){
      std::uniform_real_distribution<float> coinDist(minWeightCoinFlipOdds, maxWeightCoinFlipOdds);
      float coinOdds = coinDist(gen);

      std::uniform_real_distribution<float> edgeDist(minEdgesRatios, maxEdgesRatios);
      float edgeRatio =  edgeDist(gen);

      std::function<float(std::mt19937&)> weightFunction = [maxWeight, coinOdds](std::mt19937& gen) -> float {
         std::uniform_int_distribution<> dist(0,maxWeight);
         std::bernoulli_distribution distb(coinOdds);
         if(distb(gen))
               return dist(gen);
         else
               return 0;
      };

      
      MultipartiteSetGraph graph = generateGraphEdgeRatio(nrNodes,nrLayers,minEltsPerLayer,edgeRatio, weightFunction, index+seed);
      std::string str = std::to_string(index);
      while (str.length() < 4) {
         str = '0' + str;  
      }
      storeGraph(graph,index+seed,folderPath + "/graph"+ str + ".txt");
   }
}

void saveGraphsConstVarsUniformEdgesDefaultValues(unsigned int seed){

   int nrLayers = 6;
   int nrNodes = 160;
   float minEdgesRatios = 0.10;
   float maxEdgesRatios = 0.70;
   float minWeightCoinFlipOdds = 0.40;
   float maxWeightCoinFlipOdds = 0.60;
   int minEltsPerLayer = 10;
   int maxWeight = 1000;
   std::string folderPath = "data/graphs/edgesUniformMini";
   int nrGraphs = 600;
   saveGraphsConstVarsUniformEdges(seed, nrLayers, nrNodes, minEdgesRatios, maxEdgesRatios, minWeightCoinFlipOdds, maxWeightCoinFlipOdds, minEltsPerLayer, maxWeight, folderPath, nrGraphs);
}


void saveDoubleGraphs(unsigned int seed, std::vector<int> nrLayers, std::vector<int> nrNodes, float minLayerRatio, 
      std::vector<float> oddsSet1, std::vector<float> edgeRatio1, std::vector<float> edgeRatio2, std::vector<float> edgeRatioCross,
      std::vector<float> weightRatio, int graphsPerPars, std::string folderPath){
   
   const int NRVECS = 7;
   int sizes[NRVECS];
      
   sizes[0] = nrLayers.size();
   sizes[1] = nrNodes.size();
   sizes[2] = oddsSet1.size();
   sizes[3] = edgeRatio1.size();
   sizes[4] = edgeRatio2.size();
   sizes[5] = edgeRatioCross.size();
   sizes[6] = weightRatio.size();

   int total_graphs = 1;

   for(int i=0; i<NRVECS; i++){
      total_graphs *= sizes[i];
   }
   
   for(int i=0; i<total_graphs; i++){
      int indices[NRVECS];
      int remInd = i;
      for(int j=NRVECS-1; j>=0; j--){
         indices[j] = remInd%sizes[j];
         remInd /= sizes[j];
      }

      int maxWeight2 = 100;
      int maxWeight1 = weightRatio[indices[6]]*maxWeight2;
      

      std::function<float(std::mt19937&)> weightFunction1 = [maxWeight1](std::mt19937& gen) -> float {
         std::uniform_int_distribution<> dist(0,maxWeight1);
         return dist(gen);
      };

      std::function<float(std::mt19937&)> weightFunction2 = [maxWeight2](std::mt19937& gen) -> float {
         std::uniform_int_distribution<> dist(0,maxWeight2);
         return dist(gen);
      };

      for(int j=0; j<graphsPerPars; j++){
         int index = i*graphsPerPars + j;
         MultipartiteSetGraph graph = generateDoubleGraph(nrNodes[indices[1]],nrLayers[indices[0]], minLayerRatio,
               oddsSet1[indices[2]], edgeRatio1[indices[3]], edgeRatio2[indices[4]], edgeRatioCross[indices[5]],
            weightFunction1, weightFunction2, seed+index);
         std::string str = std::to_string(index);
         while (str.length() < 4) {
            str = '0' + str;  
         }
         storeGraph(graph, seed+index, folderPath + "/graph"+ str + ".txt");
      }

   }
}

void saveDoubleGraphsDefaultValues(unsigned int seed){

   std::vector<int> nrLayers = {3,4,5,6,7,8};
   std::vector<int> nrNodes = {90,120,150,180};
   float minLayerRatio = 0.80;
   std::vector<float> oddsSet1 = { 0.20, 0.30,0.40, 0.50};
   std::vector<float> edgeRatio1 = {0.40,0.50,0.60,0.70}; 
   std::vector<float> edgeRatio2 = {0.25,0.35,0.45,0.55};
   std::vector<float> edgeRatioCross = {0.20}; 
   std::vector<float> weightRatio  = {10};
   int graphsPerPars = 5;
   std::string folderPath = "data/graphs/doubleMini";

   saveDoubleGraphs(seed,nrLayers, nrNodes, minLayerRatio, oddsSet1, edgeRatio1, edgeRatio2,
            edgeRatioCross, weightRatio, graphsPerPars,folderPath);

}


void saveDoubleGraphsCustomWeight(unsigned int seed, std::vector<int> nrLayers, std::vector<int> nrNodes, float minLayerRatio, 
      std::vector<float> oddsSet1, std::vector<float> edgeRatio1, std::vector<float> edgeRatio2, std::vector<float> edgeRatioCross,
      std::function<float(std::mt19937&)> weightFunction1,std::function<float(std::mt19937&)> weightFunction2, int graphsPerPars, std::string folderPath){
  
   const int NRVECS = 6;
   int sizes[NRVECS];
      
   sizes[0] = nrLayers.size();
   sizes[1] = nrNodes.size();
   sizes[2] = oddsSet1.size();
   sizes[3] = edgeRatio1.size();
   sizes[4] = edgeRatio2.size();
   sizes[5] = edgeRatioCross.size();

   int total_graphs = 1;

   for(int i=0; i<NRVECS; i++){
      total_graphs *= sizes[i];
   }
   
   for(int i=0; i<total_graphs; i++){
      int indices[NRVECS];
      int remInd = i;
      for(int j=NRVECS-1; j>=0; j--){
         indices[j] = remInd%sizes[j];
         remInd /= sizes[j];
      }

     for(int j=0; j<graphsPerPars; j++){
         int index = i*graphsPerPars + j;
         MultipartiteSetGraph graph = generateDoubleGraph(nrNodes[indices[1]],nrLayers[indices[0]], minLayerRatio,
               oddsSet1[indices[2]], edgeRatio1[indices[3]], edgeRatio2[indices[4]], edgeRatioCross[indices[5]],
            weightFunction1, weightFunction2, seed+index);
         std::string str = std::to_string(index);
         while (str.length() < 4) {
            str = '0' + str;  
         }
         storeGraph(graph, seed+index, folderPath + "/graph"+ str + ".txt");
      }

   }
}

void saveDoubleGraphsCoinFlipDefaultValues(unsigned int seed){

   std::vector<int> nrLayers = {3,4,5,6,7,8};
   std::vector<int> nrNodes = {90,120,150,180,200};
   float minLayerRatio = 0.80;
   std::vector<float> oddsSet1 = { 0.20, 0.30,0.40, 0.50};
   std::vector<float> edgeRatio1 = {0.30,0.40,0.50}; 
   std::vector<float> edgeRatio2 = {0.15,0.25,0.35};
   std::vector<float> edgeRatioCross = {0.10}; 
   int graphsPerPars = 5;
   std::string folderPath = "data/graphs/newFolder";

   int maxWeight1 = 1000;
   float coinflip1 = 0.5f;
   int maxWeight2 = 100;
   float coinflip2 = 0.5f;

   std::function<float(std::mt19937&)> weightFunction1 = [maxWeight1,coinflip1](std::mt19937& gen) -> float {
      std::bernoulli_distribution distb(coinflip1);
      std::uniform_int_distribution<> dist(0,maxWeight1);
      if(distb(gen)){
         return dist(gen);
      }
      return 0;
   };

   std::function<float(std::mt19937&)> weightFunction2 = [maxWeight2,coinflip2](std::mt19937& gen) -> float {
      std::bernoulli_distribution distb(coinflip2);
      std::uniform_int_distribution<> dist(0,maxWeight2);
      if(distb(gen)){
         return dist(gen);
      }
      return 0;
   };


   saveDoubleGraphsCustomWeight(seed,nrLayers, nrNodes, minLayerRatio, oddsSet1, edgeRatio1, edgeRatio2,
            edgeRatioCross, weightFunction1, weightFunction2, graphsPerPars,folderPath);
}




void doubleGraphsDefaultUniformRanges(unsigned int seed){

   int nrLayersMin = 3;
   int nrLayersMax = 8;
   int nrNodesMin = 50;
   int nrNodesMax = 220;

   float minLayerRatio = 0.80;

   float oddsSet1Min = 0.20;
   float oddsSet1Max = 0.50;

   float edgeRatio1Min = 0.20;
   float edgeRatio1Max = 0.50;

   float edgeRatio2Min = 0.15;
   float edgeRatio2Max = 0.35;

   float edgeRatioCrossMin = 0.05;
   float edgeRatioCrossMax = 0.15;

   int graphsPerPars = 5;

   std::string folderPath = "data/graphs/doubleGraphsCoinFlipUniform2";

   int maxWeight1 = 1000;
   float coinflip1 = 0.5f;
   int maxWeight2 = 100;
   float coinflip2 = 0.5f;

   std::function<float(std::mt19937&)> weightFunction1 = [maxWeight1,coinflip1](std::mt19937& gen) -> float {
      std::bernoulli_distribution distb(coinflip1);
      std::uniform_int_distribution<> dist(0,maxWeight1);
      if(distb(gen)){
         return dist(gen);
      }
      return 0;
   };

   std::function<float(std::mt19937&)> weightFunction2 = [maxWeight2,coinflip2](std::mt19937& gen) -> float {
      std::bernoulli_distribution distb(coinflip2);
      std::uniform_int_distribution<> dist(0,maxWeight2);
      if(distb(gen)){
         return dist(gen);
      }
      return 0;
   };

   int nrGraphs = 5000;

   std::random_device rd;
   std::default_random_engine gen(rd());

   int index = 0;

   std::uniform_int_distribution<int> nrLayersRand(nrLayersMin, nrLayersMax);
   std::uniform_int_distribution<int> nrNodesRand(nrNodesMin, nrNodesMax);

   std::uniform_real_distribution<float> oddsSet1Rand(oddsSet1Min,oddsSet1Max);
   std::uniform_real_distribution<float> edgeRatio1Rand(edgeRatio1Min,edgeRatio1Max);
   std::uniform_real_distribution<float> edgeRatio2Rand(edgeRatio2Min,edgeRatio2Max);
   std::uniform_real_distribution<float> edgeRatioCrossRand(edgeRatioCrossMin,edgeRatioCrossMax);


   for(int k=0; k<(nrGraphs-1)/graphsPerPars+1; k++){
      int nrLayers = nrLayersRand(gen);
      int nrNodes = nrNodesRand(gen);
      float oddsSet1 = oddsSet1Rand(gen);
      float edgeRatio1 = edgeRatio1Rand(gen);
      float edgeRatio2 = edgeRatio2Rand(gen);
      float edgeRatioCross = edgeRatioCrossRand(gen);

      for(int j=0; j<graphsPerPars; j++){
         MultipartiteSetGraph graph = generateDoubleGraph(nrNodes,nrLayers, minLayerRatio,
               oddsSet1, edgeRatio1, edgeRatio2, edgeRatioCross,
            weightFunction1, weightFunction2, seed+index);
         std::string str = std::to_string(index);
         while (str.length() < 4) {
            str = '0' + str;  
         }
         storeGraph(graph, seed+index, folderPath + "/graph"+ str + ".txt");
         index += 1;
      }
   }

}