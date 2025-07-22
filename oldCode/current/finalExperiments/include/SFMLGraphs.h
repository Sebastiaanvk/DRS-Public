#ifndef SFML_GRAPHS_H 
#define SFML_GRAPHS_H 

#include <SFML/Graphics.hpp>
#include <thread>
#include <cmath>
#include <memory>
#include <iostream>
#include "mpGraphInterface.h"
#include "modelInterface.h"
#include <set>

enum Colour{
    Deleted,
    Red,
    Yellow,
    Green
};

void displayGraphPNG(const MPGraphInterface& G, int width, int height, std::string filename);

void displayGraphPNGPostOpt(const MPGraphInterface& G, int width, int height, OptimizableInterface *modelWrapper, std::string filename);

//void displayGraphPNGRemainingSet(const MPGraphInterface& G, int width, int height, std::set<VertexIndex> nodes_in_set);

void displayGraphPNGColorMap(const MPGraphInterface& G, int width, int height, const std::map<VertexIndex, Colour>& nodeColorMap, std::string filename );

void displayGraphPNGNodesNbsAsInts(const MPGraphInterface& G, int width, int height, const std::set<int>& current_nodes, const std::set<int>& current_nbs, std::string filename );

void displayGraphPNGNodesNbsPicked(const MPGraphInterface& G, int width, int height, const std::set<VertexIndex>& current_nodes, const std::set<VertexIndex>& current_nbs, std::string filename );


#endif // SFML_GRAPHS_H