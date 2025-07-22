#ifndef SFML_GRAPHS_H 
#define SFML_GRAPHS_H 

#include <SFML/Graphics.hpp>
#include "graphs.h"
#include "modelWrapper.h"
#include <thread>
#include <cmath>





void displayGraphPNG(const MultipartiteGraph& G, int width, int height, std::string filename);


void displayGraphPNGPostOpt(const MultipartiteGraph& G, int width, int height, const ModelWrapper& modelWrapper, std::string filename);










#endif // SFML_GRAPHS_H