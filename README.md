# Instructions

This repository contains the main part of the code base for my master thesis project.

In its current state it is not cleaned up yet.

For compiling, one needs to link the relevant dependencies. See the Makefile.

The main file is still filled with commented out lines that show the experiments and tests that were run last.

The file algorithmWrappers shows the algorithms that were used for generating the plots in the thesis.

The graphGeneration files show the methods for graph generation and certain data sets.

The experiments file shows how the experiments were done.

The codes for analyzing the data and generating plots can be found in the python folder.

The data folder contains the data sets and the results of the algorithm implementations on the data sets

the names of the data set folders are different than the names in the thesis, see the following python dictionary:
folderNameMap = {
    "doubleDegreeAllNightHeuristicTotal" : "varAll_wDUni_dDeg_2",
    "allNightVariableNodes" : "varNodes_wDUni_edges",
    "allNightVariableEdgesNew" : "varEdges_wDUni_edges",
    "allNightVariableLayers" : "varLayers_wDUni_edges",
    "allNightAllAlgorithmsFast" : "varAll_wDUni_edges"
}










