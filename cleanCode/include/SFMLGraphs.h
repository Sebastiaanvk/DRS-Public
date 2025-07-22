#ifndef SFML_GRAPHS_H 
#define SFML_GRAPHS_H 

#include <SFML/Graphics.hpp>
#include <thread>
#include <cmath>
#include <memory>
#include <iostream>
#include "mpGraphInterface.h"
#include <set>

enum Colour{
    Deleted,
    Red,
    Yellow,
    Green
};

// Saves the initial graph instance as a png using sfml library
void displayGraphPNG(const MPGraphInterface& G, int width, int height, std::string filename);

// Saves the initial graph instance as a png using sfml library
// Has a map to colout the individual vertices
void displayGraphPNGColorMap(const MPGraphInterface& G, int width, int height, const std::map<VertexIndex, Colour>& nodeColorMap, std::string filename );

// Saves the initial graph instance as a png using sfml library
// Colors the current_nodes green and the current_nbs yellow.
// Uses ints to enumerate the vertices
void displayGraphPNGNodesNbsAsInts(const MPGraphInterface& G, int width, int height, const std::set<int>& current_nodes, const std::set<int>& current_nbs, std::string filename );

// Saves the initial graph instance as a png using sfml library
// Colors the current_nodes green and the current_nbs yellow.
// Uses the VertexIndex struct to enumerate the vertices
void displayGraphPNGNodesNbsPicked(const MPGraphInterface& G, int width, int height, const std::set<VertexIndex>& current_nodes, const std::set<VertexIndex>& current_nbs, std::string filename );


#endif // SFML_GRAPHS_H