#ifndef SFML_GRAPHS_H 
#define SFML_GRAPHS_H 

#include <SFML/Graphics.hpp>
#include <thread>
#include <cmath>
#include <memory>
#include <iostream>
#include "mpGraphInterface.h"
#include "modelInterface.h"

void displayGraphPNG(const MPGraphInterface& G, int width, int height, std::string filename);

void displayGraphPNGPostOpt(const MPGraphInterface& G, int width, int height, OptimizableInterface *modelWrapper, std::string filename);






#endif // SFML_GRAPHS_H