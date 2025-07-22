#include <iostream>
#include "/home/sebas/gurobi1102/linux64/include/gurobi_c++.h"
#include <set>
#include <map>
#include <utility>
#include <list>
#include "graphs.h"




double find_optimum_linearized(const MultipartiteGraph& G, bool relaxed, bool path_constraints, bool print_variable_values, bool neighbors_both_sides);