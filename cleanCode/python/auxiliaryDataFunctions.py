import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os



def lineplotTimeEdges(df,plotTitle=None,withHue=True):
    if withHue:
        sns.lineplot(data=df,x='nrEdges',y='timeElapsed',hue='nrLayers',errorbar=None,palette="bright")
    else:
        sns.lineplot(data=df,x='nrEdges',y='timeElapsed',errorbar=None,palette="bright")
    if plotTitle != None:
        plt.title(plotTitle)
    plt.xlabel("Number of Edges")
    plt.ylabel("Time elapsed (seconds)")
    plt.show()

def lineplotTimeEdgesSmoother(df,plotTitle=None,withHue=True, nrBins = 20):
    df["binsMiddle"] = pd.cut(df["nrEdges"],nrBins).apply(lambda x: (x.left+x.right)/2)

    if withHue:
        sns.lineplot(data=df,x='binsMiddle',y='timeElapsed',hue='nrLayers',palette="bright",lowess=True)
    else:
        sns.lineplot(data=df,x='binsMiddle',y='timeElapsed')
    if plotTitle != None:
        plt.title(plotTitle)
    plt.xlabel("Number of Edges")
    plt.ylabel("Time elapsed (seconds) (split into " + str(nrBins) +  " bins)")
    plt.show()

def lineplotTimeNodes(df,plotTitle=None,useErrorBar=False):
    if(useErrorBar):
        sns.lineplot(data=df,x='nrNodes',y='timeElapsed',hue='nrLayers',palette="bright")
    else:
        sns.lineplot(data=df,x='nrNodes',y='timeElapsed',hue='nrLayers',errorbar=None,palette="bright")
    if plotTitle != None:
        plt.title(plotTitle)
    plt.xlabel("Number of Nodes")
    plt.ylabel("Time elapsed (seconds)")
    plt.show()

def lineplotTimeEdgesNodesRatio(df,plotTitle=None,nrLayers=None):
    edgesNodesRatio = df['nrNodes']/df['nrEdges']
    if nrLayers == None:
        sns.lineplot(x=edgesNodesRatio,y=df['timeElapsed'],hue=df['nrLayers'],errorbar=None,palette="bright")
    else:
        ind = df['nrLayers'] == nrLayers
        sns.lineplot(x=edgesNodesRatio[ind],y=df['timeElapsed'][ind],hue=df['nrLayers'][ind],errorbar=None,palette="bright")
    if plotTitle != None:
        plt.title(plotTitle)
    plt.xlabel("Number of Edges per Node")
    plt.ylabel("Time elapsed (seconds)")
    plt.show()


def lineplotAccuracyBins(df,exactdf,plotTitle=None,nrBins=20, useErrorBar = False):
    # mask =  (exactdf["solvedToCompletion"]==True) & (df["solvedToCompletion"]==True)
    mask =  (exactdf["solvedToCompletion"]==True)


    x = df.loc[mask,"nrNodes"]

    x = pd.cut(x,nrBins).apply(lambda x: (x.left+x.right)/2)

    y = df.loc[mask,"objectiveValue"]/exactdf.loc[mask,"objectiveValue"]
    hue = df.loc[mask,"nrLayers"]

#This way also works fyi
    #x = df["nrNodes"][mask]
    #y = df["objectiveValue"][mask]/exactdf["objectiveValue"][mask]
    #hue = df["nrLayers"][mask]

    if useErrorBar:
        sns.lineplot(x=x,y=y,hue=hue)
    else:
        sns.lineplot(x=x,y=y,hue=hue,errorbar=None)

    if plotTitle != None:
        plt.title(plotTitle)
    
    plt.xlabel("Number of Nodes")
    plt.ylabel("Approximation Ratio")


    plt.show()

def lineplotAccuracy(df,exactdf,plotTitle=None):
    mask =  (exactdf["solvedToCompletion"]==True) & (df["solvedToCompletion"]==True)

    x = df.loc[mask,"nrNodes"]
    y = df.loc[mask,"objectiveValue"]/exactdf.loc[mask,"objectiveValue"]
    hue = df.loc[mask,"nrLayers"]

#This way also works fyi
    #x = df["nrNodes"][mask]
    #y = df["objectiveValue"][mask]/exactdf["objectiveValue"][mask]
    #hue = df["nrLayers"][mask]

    sns.lineplot(x=x,y=y,hue=hue,errorbar=None)

    if plotTitle != None:
        plt.title(plotTitle)
    
    plt.xlabel("Number of Nodes")
    plt.ylabel("Approximation Ratio")


    plt.show()

def scatterplotAccuracy(df,exactdf,plotTitle=None):
    mask =  (exactdf["solvedToCompletion"]==True) & (df["solvedToCompletion"]==True)

    sns.scatterplot(x=df["nrNodes"][mask],y=df['objectiveValue'][mask]/exactdf['objectiveValue'][mask],hue=df['nrLayers'][mask],palette="bright")

#    sns.scatterplot(x=df["nrNodes"][mask],y=df['objectiveValue'][mask]/exactdf['objectiveValue'][mask],hue=df['nrLayers'][mask],palette="bright")
   

    if plotTitle != None:
        plt.title(plotTitle)
    
    plt.xlabel("Number of Nodes")
    plt.ylabel("Approximation Ratio")

    plt.show()

def indexToGraphName(index):
    indStr = str(index)
    while len(indStr)<4:
        indStr = "0" + indStr
    return "graph" + indStr + ".txt"

def readInputGraph(inp):
    vars = inp[0].split("\n")
    nrLayers = int(vars[1])
    nrEdges = int(vars[3])
    eltsPerLayers = [int(x) for x in vars[2].split(" ")]
    nrVerts = sum(eltsPerLayers)
    weights = [int(x) for x in inp[1].split()]

    nonZeroWeights = 0
    totalWeight = 0
    for x in weights:
        totalWeight += x
        if x != 0:
            nonZeroWeights += 1
    totalPossibleEdges = 0
    for i in range(nrLayers-1):
        totalPossibleEdges += eltsPerLayers[i]*eltsPerLayers[i+1]

    ratioNonZeroWeights = nonZeroWeights/nrVerts
    ratioEdges = nrEdges/totalPossibleEdges

    return {"nrVerts": nrVerts, "nrLayers": nrLayers, "nrEdges": nrEdges, "ratioEdges": ratioEdges, "totalWeight": totalWeight, "ratioNonZeroWeights": ratioNonZeroWeights}

def loadGraphsAsDF(folderPath):
    
    index = 0
    graphName = indexToGraphName(index)
    graphs = {} 
    while os.path.exists(folderPath + graphName) :
        inp = open(folderPath+graphName).read().split("\n\n")
        graphs[index] = readInputGraph(inp)
        index += 1
        graphName = indexToGraphName(index)
    return pd.DataFrame.from_dict(graphs,  orient='index') #Otherwise the columns and rows are switched



algorithmsUni = ["altGurobiUni","greedySinglePathsUni","greedyPeelingUni","baselineEverythingUni","dynSinglePathsUni"]
algorithmsBi = ["altGurobiBi","greedySinglePathsBi","greedyPeelingBi","baselineEverythingBi","dynSinglePathsBi"]

def accuracyFullFolder(folderPath):
    pdsUni  = [ pd.read_csv(folderPath + algName + ".txt") for algName in algorithmsUni] 
    pdsBi  = [ pd.read_csv(folderPath + algName + ".txt") for algName in algorithmsBi] 
     
    for i in range(1,5):
        lineplotAccuracy(pdsUni[i],pdsUni[0],algorithmsUni[i])
    for i in range(1,5):
        lineplotAccuracy(pdsBi[i],pdsBi[0],algorithmsBi[i])


def accuracyFullFolderBins(folderPath,nrBins,useErrorBar=False):
    pdsUni  = [ pd.read_csv(folderPath + algName + ".txt") for algName in algorithmsUni] 
    pdsBi  = [ pd.read_csv(folderPath + algName + ".txt") for algName in algorithmsBi] 
     
    for i in range(1,5):
        lineplotAccuracyBins(pdsUni[i],pdsUni[0],algorithmsUni[i],nrBins,useErrorBar=useErrorBar)
    for i in range(1,5):
        lineplotAccuracyBins(pdsBi[i],pdsBi[0],algorithmsBi[i],nrBins,useErrorBar=useErrorBar)



def accuracyFullFolderBins(folderPath,nrBins,useErrorBar=False):
    pdsUni  = [ pd.read_csv(folderPath + algName + ".txt") for algName in algorithmsUni] 
    pdsBi  = [ pd.read_csv(folderPath + algName + ".txt") for algName in algorithmsBi] 
     
    for i in range(1,5):
        lineplotAccuracyBins(pdsUni[i],pdsUni[0],algorithmsUni[i],nrBins,useErrorBar=useErrorBar)
    for i in range(1,5):
        lineplotAccuracyBins(pdsBi[i],pdsBi[0],algorithmsBi[i],nrBins,useErrorBar=useErrorBar)


def accuracyFullFolderBins(folderPath,nrBins,useErrorBar=False):
    pdsUni  = [ pd.read_csv(folderPath + algName + ".txt") for algName in algorithmsUni] 
    pdsBi  = [ pd.read_csv(folderPath + algName + ".txt") for algName in algorithmsBi] 
     
    for i in range(1,5):
        lineplotAccuracyBins(pdsUni[i],pdsUni[0],algorithmsUni[i],nrBins,useErrorBar=useErrorBar)
    for i in range(1,5):
        lineplotAccuracyBins(pdsBi[i],pdsBi[0],algorithmsBi[i],nrBins,useErrorBar=useErrorBar)


def lineplotAccuracyBinsCustom(df,exactdf,xColumn, xLabel, plotTitle=None,nrBins=20, useErrorBar = False):
    # mask =  (exactdf["solvedToCompletion"]==True) & (df["solvedToCompletion"]==True)
    mask =  (exactdf["solvedToCompletion"]==True)

    x = df.loc[mask,xColumn]

    x = pd.cut(x,nrBins).apply(lambda x: (x.left+x.right)/2)

    y = df.loc[mask,"objectiveValue"]/exactdf.loc[mask,"objectiveValue"]
    hue = df.loc[mask,"nrLayers"]

    if useErrorBar:
        sns.lineplot(x=x,y=y,hue=hue)
    else:
        sns.lineplot(x=x,y=y,hue=hue,errorbar=None)

    if plotTitle != None:
        plt.title(plotTitle)
    
    plt.xlabel(xLabel)
    plt.ylabel("Approximation Ratio")


    plt.show()

def lineplotAccuracyBinsCustomExtra(df,exactdf, extraStats, xColumn, xLabel, plotTitle=None, nrBins=20, useErrorBar = False):
    # mask =  (exactdf["solvedToCompletion"]==True) & (df["solvedToCompletion"]==True)
    mask =  (exactdf["solvedToCompletion"]==True)

    x = extraStats.loc[mask,xColumn]

    x = pd.cut(x,nrBins).apply(lambda x: (x.left+x.right)/2)

    y = df.loc[mask,"objectiveValue"]/exactdf.loc[mask,"objectiveValue"]
    hue = df.loc[mask,"nrLayers"]

    if useErrorBar:
        sns.lineplot(x=x,y=y,hue=hue)
    else:
        sns.lineplot(x=x,y=y,hue=hue,errorbar=None)

    if plotTitle != None:
        plt.title(plotTitle)
    
    plt.xlabel(xLabel)
    plt.ylabel("Approximation Ratio")


    plt.show()

def accuracyFullFolderBinsCustom(folderPath,xColumn, xLabel, nrBins=20,useErrorBar=False, useExtraStats=False):
    pdsUni  = [ pd.read_csv(folderPath + algName + ".txt") for algName in algorithmsUni] 
    pdsBi  = [ pd.read_csv(folderPath + algName + ".txt") for algName in algorithmsBi] 
    if useExtraStats:
        extraStats = pd.read_csv(folderPath + "parsData.txt")

        for i in range(1,5):
            lineplotAccuracyBinsCustomExtra(pdsUni[i],pdsUni[0],extraStats,xColumn, xLabel, algorithmsUni[i],nrBins=nrBins,useErrorBar=useErrorBar)
        for i in range(1,5):
            lineplotAccuracyBinsCustomExtra(pdsBi[i],pdsBi[0],extraStats,xColumn, xLabel, algorithmsBi[i],nrBins=nrBins,useErrorBar=useErrorBar)
    else:
        for i in range(1,5):
            lineplotAccuracyBinsCustom(pdsUni[i],pdsUni[0],xColumn, xLabel, algorithmsUni[i],nrBins=nrBins,useErrorBar=useErrorBar)
        for i in range(1,5):
            lineplotAccuracyBinsCustom(pdsBi[i],pdsBi[0],xColumn, xLabel, algorithmsBi[i],nrBins=nrBins,useErrorBar=useErrorBar)



def lineplotEfficiencyBinsCustom(df,xColumn, xLabel, plotTitle=None,nrBins=20, useErrorBar = False):
    # mask =  (exactdf["solvedToCompletion"]==True) & (df["solvedToCompletion"]==True)
    # mask =  (exactdf["solvedToCompletion"]==True)

    x = df[xColumn]

    x = pd.cut(x,nrBins).apply(lambda x: (x.left+x.right)/2)

    y = df["timeElapsed"]
    hue = df["nrLayers"]

    if useErrorBar:
        sns.lineplot(x=x,y=y,hue=hue)
    else:
        sns.lineplot(x=x,y=y,hue=hue,errorbar=None)

    if plotTitle != None:
        plt.title(plotTitle)
    
    plt.xlabel(xLabel)
    plt.ylabel("Time Elapsed(seconds)")


    plt.show()

def lineplotEfficiencyBinsCustomExtra(df, extraStats, xColumn, xLabel, plotTitle=None, nrBins=20, useErrorBar = False):
    # mask =  (exactdf["solvedToCompletion"]==True) & (df["solvedToCompletion"]==True)
    # mask =  (exactdf["solvedToCompletion"]==True)

    x = extraStats[xColumn]

    x = pd.cut(x,nrBins).apply(lambda x: (x.left+x.right)/2)

    y = df["timeElapsed"]
    hue = df["nrLayers"]

    if useErrorBar:
        sns.lineplot(x=x,y=y,hue=hue)
    else:
        sns.lineplot(x=x,y=y,hue=hue,errorbar=None)

    if plotTitle != None:
        plt.title(plotTitle)
    
    plt.xlabel(xLabel)
    plt.ylabel("Time Elapsed(seconds)")
    plt.show()



def efficiencyFullFolderBinsCustom(folderPath,xColumn, xLabel, nrBins=20,useErrorBar=False, useExtraStats=False):
    pdsUni  = [ pd.read_csv(folderPath + algName + ".txt") for algName in algorithmsUni] 
    pdsBi  = [ pd.read_csv(folderPath + algName + ".txt") for algName in algorithmsBi] 
    if useExtraStats:
        extraStats = pd.read_csv(folderPath + "parsData.txt")

        for i in range(0,5):
            lineplotEfficiencyBinsCustomExtra(pdsUni[i],extraStats,xColumn, xLabel, algorithmsUni[i],nrBins=nrBins,useErrorBar=useErrorBar)
        for i in range(0,5):
            lineplotEfficiencyBinsCustomExtra(pdsBi[i],extraStats,xColumn, xLabel, algorithmsBi[i],nrBins=nrBins,useErrorBar=useErrorBar)
    else:
        for i in range(0,5):
            lineplotEfficiencyBinsCustom(pdsUni[i],xColumn, xLabel, algorithmsUni[i],nrBins=nrBins,useErrorBar=useErrorBar)
        for i in range(0,5):
            lineplotEfficiencyBinsCustom(pdsBi[i],xColumn, xLabel, algorithmsBi[i],nrBins=nrBins,useErrorBar=useErrorBar)




def lineplotBins(df, yColumn, yLabel, xColumn, xLabel, plotTitle=None, nrBins=20, useErrorBar = False, extraStats = None, avgDegree = True):


    if avgDegree:
        x = 2*df["nrEdges"]/df["nrNodes"]
    elif extraStats is None:
        x = df[xColumn]
    else:
        x = extraStats[xColumn]

    x = pd.cut(x,nrBins).apply(lambda x: (x.left+x.right)/2)

    y = df[yColumn]
    hue = df["nrLayers"]

    if useErrorBar:
        sns.lineplot(x=x,y=y,hue=hue)
    else:
        sns.lineplot(x=x,y=y,hue=hue,errorbar=None)

    if plotTitle != None:
        plt.title(plotTitle)
    
    plt.xlabel(xLabel)
    plt.ylabel(yLabel)
    plt.show()



def plotFullFolderHeuristic(folderPath,yColumn,yLabel,xColumn = None, xLabel = None, nrBins=20,useErrorBar=False, useExtraStats=False, avgDegree=False):
    heuristicAlgorithmsUni = algorithmsUni[1:]
    heuristicAlgorithmsBi = algorithmsBi[1:]
    pdsUni  = [ pd.read_csv(folderPath + algName + ".txt") for algName in heuristicAlgorithmsUni] 
    pdsBi  = [ pd.read_csv(folderPath + algName + ".txt") for algName in heuristicAlgorithmsBi] 
    if useExtraStats:
        extraStats = pd.read_csv(folderPath + "parsData.txt")
        for i in range(4):
            lineplotBins(pdsUni[i], yColumn, yLabel, xColumn, xLabel, heuristicAlgorithmsUni[i],nrBins=nrBins,useErrorBar=useErrorBar,extraStats=extraStats, avgDegree=avgDegree)
        for i in range(4):
            lineplotBins(pdsBi[i],yColumn,yLabel,xColumn, xLabel, heuristicAlgorithmsBi[i],nrBins=nrBins,useErrorBar=useErrorBar,extraStats=extraStats, avgDegree=avgDegree)
    else:
        for i in range(4):
            lineplotBins(pdsUni[i],yColumn,yLabel,xColumn, xLabel, heuristicAlgorithmsUni[i],nrBins=nrBins,useErrorBar=useErrorBar, avgDegree=avgDegree)
        for i in range(4):
            lineplotBins(pdsBi[i],yColumn,yLabel,xColumn, xLabel, heuristicAlgorithmsBi[i],nrBins=nrBins,useErrorBar=useErrorBar, avgDegree=avgDegree)



colorMap = {
    "altGurobiUni.txt" : "purple",
    "altGurobiBi.txt" : "purple",
    "greedySinglePathsUni.txt" : "red", 
    "greedySinglePathsBi.txt" : "red", 
    "greedyPeelingUni.txt" : "green",
    "greedyPeelingBi.txt" : "green",
    "dynSinglePathsUni.txt" : "orange", 
    "dynSinglePathsBi.txt" : "orange", 
    "baselineEverythingUni.txt" : "blue",
    "baselineEverythingBi.txt" : "blue",
}

# nameMap = {
#     "altGurobiUni.txt" : "Gurobi Uni",
#     "altGurobiBi.txt" : "Gurobi Bi",
#     "greedySinglePathsUni.txt" : "Greedy Single Paths Uni", 
#     "greedySinglePathsBi.txt" : "Greedy Single Paths Bi", 
#     "greedyPeelingUni.txt" : "Greedy Peeling Uni",
#     "greedyPeelingBi.txt" : "Greedy Peeling Bi",
#     "dynSinglePathsUni.txt" : "Dyn Single Paths Uni", 
#     "dynSinglePathsBi.txt" : "Dyn Single Paths Bi", 
#     "baselineEverythingUni.txt" : "Pick Everything Uni",
#     "baselineEverythingBi.txt" : "Pick Everything Bi",
# }

nameMap = {
    "altGurobiUni.txt" : "MILPAlt PDRS",
    "altGurobiBi.txt" : "MILPAlt NDRS",
    "greedySinglePathsUni.txt" : "Greedy Single Paths PDRS", 
    "greedySinglePathsBi.txt" : "Greedy Single Paths NDRS", 
    "greedyPeelingUni.txt" : "Greedy Peeling PDRS",
    "greedyPeelingBi.txt" : "Greedy Peeling NDRS",
    "dynSinglePathsUni.txt" : "Dynamic Path PDP", 
    "dynSinglePathsBi.txt" : "Dynamic Path NDP", 
    "baselineEverythingUni.txt" : "Pick Everything PDRS",
    "baselineEverythingBi.txt" : "Pick Everything NDRS",
}


def folderSinglePlot(folderPath,yColumn="objectiveValue",yLabel="Objective Value",xColumn="nrNodes" , xLabel="Number of Nodes", nrBins=10,useErrorBar=False, splitLayers = False, pltTitle = "Plots", yScaleLog=False):
    folderName = folderPath.split("/")[-1]
    uniResults = [f for f in os.listdir(folderPath) if os.path.isfile(os.path.join(folderPath, f)) and f[-7:]=="Uni.txt"]
    uniDfs = [pd.read_csv(os.path.join(folderPath,f)) for f in uniResults]

    biResults = [f for f in os.listdir(folderPath) if os.path.isfile(os.path.join(folderPath, f)) and f[-6:]=="Bi.txt"]
    biDfs = [pd.read_csv(os.path.join(folderPath,f)) for f in biResults]



    if xColumn in uniDfs[0].columns:
        x = uniDfs[0][xColumn]
    elif os.path.isfile(os.path.join(folderPath,"parsData.txt")):
        extraStats = pd.read_csv(os.path.join(folderPath,"parsData.txt"))
        if xColumn in extraStats.column:
            x = extraStats[x]
        else:
            print("X axis couldnt be found!")
            return
    
    x = pd.cut(x,nrBins).apply(lambda x: (x.left+x.right)/2)
    if not splitLayers:
        for df,name in zip(uniDfs,uniResults):
            #to do labels
            # plt.plot(df[yColumn],label=name)
            sns.lineplot(x=x,y=df[yColumn],label=nameMap[name],color=colorMap[name],markers=markerMap[name])
        plt.legend()
        plt.xlabel(xLabel)
        plt.ylabel(yLabel)
        # plt.title(pltTitle + " for uni algorithms on folder " + folderPath[16:] )
        if yScaleLog: 
            plt.yscale("log")
        plt.show()

        for df,name in zip(biDfs,biResults):
            #to do labels
            sns.lineplot(x=x,y=df[yColumn],label=nameMap[name],color=colorMap[name],markers=markerMap[name])
        plt.legend()
        plt.xlabel(xLabel)
        plt.ylabel(yLabel)
        # plt.title(pltTitle + " for bi algorithms")
        if yScaleLog: 
            plt.yscale("log")
        plt.show()
    else:
        minLayer = uniDfs[0]["nrLayers"].min()
        maxLayer = uniDfs[1]["nrLayers"].max()
        layers= maxLayer-minLayer+1
        fig, axes = plt.subplots(1+(layers-1)//4, 4, figsize=(16, 8))  # 2 rows, 4 columns
        axes = axes.flatten() # flattens to make indexing easier (thanks chatgpt)
# First for uni algorithms
        for i, ax in enumerate(axes): 
            if i < layers:
                nrLayers = minLayer+i
                mask = uniDfs[0]["nrLayers"] == nrLayers
                for df,name in zip(uniDfs,uniResults):
                    #to do labels
                    # plt.plot(df[yColumn],label=name)
                    sns.lineplot(x=x[mask],y=df[yColumn][mask],label=nameMap[name],color=colorMap[name],markers=markerMap[name], ax=ax)
                # ax.legend()
                ax.legend_.remove()
                ax.set_xlabel(xLabel)
                ax.set_ylabel(yLabel)
                ax.set_title("Number of Layers is " + str(nrLayers))
                if yScaleLog: 
                    ax.set_yscale("log")
            else:
                ax.axis("off")
        # plt.tight_layout()
        fig.suptitle(pltTitle + " for uni algorithms on folder " + folderPath[16:],fontsize=16 )
        handles, labels = axes[0].get_legend_handles_labels()
        fig.legend(handles, labels, loc='lower center',bbox_to_anchor=(0.5, -0.05), ncol=len(labels), fontsize='medium') 


        plt.tight_layout(rect=[0, 0, 1, 0.93])
        plt.show()


        fig, axes = plt.subplots(1+(layers-1)//4, 4, figsize=(16, 8))  # 2 rows, 4 columns
        axes = axes.flatten() # flattens to make indexing easier (thanks chatgpt)

        for i, ax in enumerate(axes): 
            if i < layers:
                nrLayers = minLayer+i
                mask = biDfs[0]["nrLayers"] == nrLayers
                for df,name in zip(biDfs,biResults):
                    #to do labels
                    # plt.plot(df[yColumn],label=name)
                    sns.lineplot(x=x[mask],y=df[yColumn][mask],label=nameMap[name],color=colorMap[name],markers=markerMap[name], ax=ax)
                # ax.legend()
                ax.legend_.remove()
                ax.set_xlabel(xLabel)
                ax.set_ylabel(yLabel)
                ax.set_title("Number of Layers is " + str(nrLayers))
                if yScaleLog: 
                    ax.set_yscale("log")
            else:
                ax.axis("off")
        # plt.tight_layout()
        fig.suptitle(pltTitle + " for bi algorithms on folder " + folderPath[16:],fontsize=16 )
        handles, labels = axes[0].get_legend_handles_labels()
        fig.legend(handles, labels, loc='lower center',bbox_to_anchor=(0.5, -0.05), ncol=len(labels), fontsize='medium') 
        plt.tight_layout(rect=[0, 0, 1, 0.93])
        # plt.legend()

        plt.show()
    


folderNameMap = {
    "doubleDegreeAllNight" : "varAll_wDUni_dDeg_1",
    "doubleDegreeAllNightHeuristicTotal" : "varAll_wDUni_dDeg_2",
    "allNightVariableNodes" : "varNodes_wDUni_edges",
    "allNightVariableEdgesNew" : "varEdges_wDUni_edges",
    "allNightVariableLayers" : "varLayers_wDUni_edges",
    "allNightAllAlgorithmsFast" : "varAll_wDUni_edges"

}

yTitleMap = {
    "timeElapsed" : "Running Times",
    "objectiveValue" : "Objective Value" 
}

yLabelMap = {
    "timeElapsed" : "Time Elapsed (Seconds)",
    "objectiveValue" : "Objective Value" 
}

xTitleMap = {
    "nrNodes" : "Number of Nodes",
    "nrEdges" : "Number of Edges",
    "nrLayers" : "Number of Layers"

}

xLabelMap = {
    "nrNodes" : "Number of Nodes",
    "nrEdges" : "Number of Edges",
    "nrLayers" : "Number of Layers"
}

markerMap = {
    "altGurobiUni.txt" : "D",
    "altGurobiBi.txt" : "D",
    "greedySinglePathsUni.txt" : "s", 
    "greedySinglePathsBi.txt" : "s", 
    "greedyPeelingUni.txt" : "v",
    "greedyPeelingBi.txt" : "v",
    "dynSinglePathsUni.txt" : "^", 
    "dynSinglePathsBi.txt" : "^", 
    "baselineEverythingUni.txt" : "o",
    "baselineEverythingBi.txt" : "o",
}

def folderPlotFinal(folderPath,yColumn,xColumn, nrBins=10,useErrorBar=True, splitLayers = False, yScaleLog=False,binsOff=False,savePlots=False):
    dataSetName = folderNameMap[folderPath.split("/")[-1]]
    yTitle = yTitleMap[yColumn]
    yLabel = yLabelMap[yColumn]
    xTitle = xTitleMap[xColumn]
    xLabel = xLabelMap[xColumn]
    
    uniResults = [f for f in os.listdir(folderPath) if os.path.isfile(os.path.join(folderPath, f)) and f[-7:]=="Uni.txt"]
    uniResults.sort()
    uniDfs = [pd.read_csv(os.path.join(folderPath,f)) for f in uniResults]

    biResults = [f for f in os.listdir(folderPath) if os.path.isfile(os.path.join(folderPath, f)) and f[-6:]=="Bi.txt"]
    biResults.sort()
    biDfs = [pd.read_csv(os.path.join(folderPath,f)) for f in biResults]

    if "altGurobiUni.txt" in uniResults: 
        pltTitleUni = yTitle + " vs " + xTitle + "\nfor PDRS algorithms on " + dataSetName
        fileNameUni = yColumn + "VS" + xColumn + "_PDRS_on_" + dataSetName + ".png"
        pltTitleBi = yTitle + " vs " + xTitle + " \nfor NDRS algorithms on " + dataSetName
        fileNameBi = yColumn + "VS" + xColumn + "_NDRS_on_" + dataSetName + ".png"
    else:
        pltTitleUni = yTitle + " vs " + xTitle + "\nfor heuristic PDRS algorithms on " + dataSetName
        fileNameUni = yColumn + "VS" + xColumn + "_heur_PDRS_on_" + dataSetName  + ".png"
        pltTitleBi = yTitle + " vs " + xTitle + "\nfor heuristic NDRS algorithms on " + dataSetName
        fileNameBi = yColumn + "VS" + xColumn + "_heur_NDRS_on_" + dataSetName + ".png"



    if xColumn in uniDfs[0].columns:
        x = uniDfs[0][xColumn]
    elif os.path.isfile(os.path.join(folderPath,"parsData.txt")):
        extraStats = pd.read_csv(os.path.join(folderPath,"parsData.txt"))
        if xColumn in extraStats.column:
            x = extraStats[x]
        else:
            print("X axis couldnt be found!")
            return
    
    if not binsOff:
        x = pd.cut(x,nrBins).apply(lambda x: (x.left+x.right)/2)

    if not splitLayers:
        for df,name in zip(uniDfs,uniResults):
            #to do labels
            # plt.plot(df[yColumn],label=name)
            sns.lineplot(x=x,y=df[yColumn],label=nameMap[name],color=colorMap[name],marker=markerMap[name])
        plt.legend()
        plt.xlabel(xLabel)
        plt.ylabel(yLabel)
        # plt.title( pltTitleUni )
        if yScaleLog: 
            plt.yscale("log")
        if savePlots:
            plt.savefig("plots/"+ fileNameUni,bbox_inches='tight')
        plt.show()

        for df,name in zip(biDfs,biResults):
            #to do labels
            sns.lineplot(x=x,y=df[yColumn],label=nameMap[name],color=colorMap[name],marker=markerMap[name])
        plt.legend()
        plt.xlabel(xLabel)
        plt.ylabel(yLabel)
        # plt.title(pltTitleBi)
        if yScaleLog: 
            plt.yscale("log")
        if savePlots:
            plt.savefig("plots/"+ fileNameBi,bbox_inches='tight')
        plt.show()
    else:
        minLayer = uniDfs[0]["nrLayers"].min()
        maxLayer = uniDfs[1]["nrLayers"].max()
        layers= maxLayer-minLayer+1
        fig, axes = plt.subplots(1+(layers-1)//4, 4, figsize=(16, 8))  # 2 rows, 4 columns
        axes = axes.flatten() # flattens to make indexing easier (thanks chatgpt)
# First for uni algorithms
        for i, ax in enumerate(axes): 
            if i < layers:
                nrLayers = minLayer+i
                mask = uniDfs[0]["nrLayers"] == nrLayers
                for df,name in zip(uniDfs,uniResults):
                    #to do labels
                    # plt.plot(df[yColumn],label=name)
                    sns.lineplot(x=x[mask],y=df[yColumn][mask],label=nameMap[name],color=colorMap[name],marker=markerMap[name], ax=ax)
                # ax.legend()
                ax.legend_.remove()
                ax.set_xlabel(xLabel)
                ax.set_ylabel(yLabel)
                ax.set_title("Number of Layers is " + str(nrLayers))
                if yScaleLog: 
                    ax.set_yscale("log")
            else:
                ax.axis("off")
        # plt.tight_layout()
        fig.suptitle(pltTitleUni,fontsize=16 )
        handles, labels = axes[0].get_legend_handles_labels()
        fig.legend(handles, labels, loc='lower center',bbox_to_anchor=(0.5, -0.05), ncol=len(labels), fontsize='medium') 


        plt.tight_layout(rect=[0, 0, 1, 0.93])
        if savePlots:
            plt.savefig( "plots/split_layers_"+ fileNameUni,bbox_inches='tight')
        plt.show()


        fig, axes = plt.subplots(1+(layers-1)//4, 4, figsize=(16, 8))  # 2 rows, 4 columns
        axes = axes.flatten() # flattens to make indexing easier (thanks chatgpt)

        for i, ax in enumerate(axes): 
            if i < layers:
                nrLayers = minLayer+i
                mask = biDfs[0]["nrLayers"] == nrLayers
                for df,name in zip(biDfs,biResults):
                    #to do labels
                    # plt.plot(df[yColumn],label=name)
                    sns.lineplot(x=x[mask],y=df[yColumn][mask],label=nameMap[name],color=colorMap[name],marker=markerMap[name], ax=ax)
                # ax.legend()
                ax.legend_.remove()
                ax.set_xlabel(xLabel)
                ax.set_ylabel(yLabel)
                ax.set_title("Number of Layers is " + str(nrLayers))
                if yScaleLog: 
                    ax.set_yscale("log")
            else:
                ax.axis("off")
        # plt.tight_layout()
        fig.suptitle(pltTitleBi,fontsize=16 )
        handles, labels = axes[0].get_legend_handles_labels()
        fig.legend(handles, labels, loc='lower center',bbox_to_anchor=(0.5, -0.05), ncol=len(labels), fontsize='medium') 
        plt.tight_layout(rect=[0, 0, 1, 0.93])
        # plt.legend()
        if savePlots:
            plt.savefig("plots/split_layers_"+ fileNameBi,bbox_inches='tight')
        plt.show()



def dspPlotFolder(folderPath,yColumn,xColumn, nrBins=10,useErrorBar=True,binsOff=False,savePlots=False):
    dataSetName = folderNameMap[folderPath.split("/")[-1]]
    yTitle = yTitleMap[yColumn]
    yLabel = yLabelMap[yColumn]
    xTitle = xTitleMap[xColumn]
    xLabel = xLabelMap[xColumn]
    
    results = ["dynSinglePathsUni.txt","dynSinglePathsBi.txt"]
    dfs = [pd.read_csv(os.path.join(folderPath,f)) for f in results]



    if xColumn in dfs[0].columns:
        x = dfs[0][xColumn]
    elif os.path.isfile(os.path.join(folderPath,"parsData.txt")):
        extraStats = pd.read_csv(os.path.join(folderPath,"parsData.txt"))
        if xColumn in extraStats.column:
            x = extraStats[x]
        else:
            print("X axis couldnt be found!")
            return
    
    if not binsOff:
        x = pd.cut(x,nrBins).apply(lambda x: (x.left+x.right)/2)



    # pltTitle = yTitle + " vs " + xTitle + "\n dyn algorithms on " + dataSetName
    fileName = yColumn + "VS" + xColumn + "_exact_PDRP_on_" + dataSetName + ".png"
    sns.lineplot(x=x,y=dfs[0][yColumn],label=nameMap[results[0]],color=colorMap[results[0]])
    plt.legend()
    plt.xlabel(xLabel)
    plt.ylabel(yLabel)
    # plt.title( pltTitle)
    if savePlots:
        plt.savefig("plots/"+ fileName,bbox_inches='tight')
    plt.show()


    fileName = yColumn + "VS" + xColumn + "_exact_NDRP_on_" + dataSetName + ".png"
    sns.lineplot(x=x,y=dfs[1][yColumn],label=nameMap[results[1]],color=colorMap[results[1]])
    plt.legend()
    plt.xlabel(xLabel)
    plt.ylabel(yLabel)
    # plt.title( pltTitle)
    if savePlots:
        plt.savefig("plots/"+ fileName,bbox_inches='tight')
    plt.show()


def folderPlotFinalOnlyTerminated(folderPath,yColumn,xColumn, nrBins=10,useErrorBar=True,  yScaleLog=False,binsOff=False,savePlots=False):
    dataSetName = folderNameMap[folderPath.split("/")[-1]]
    yTitle = yTitleMap[yColumn]
    yLabel = yLabelMap[yColumn]
    xTitle = xTitleMap[xColumn]
    xLabel = xLabelMap[xColumn]
    
    uniResults = [f for f in os.listdir(folderPath) if os.path.isfile(os.path.join(folderPath, f)) and f[-7:]=="Uni.txt"]
    uniResults.sort()
    uniDfs = [pd.read_csv(os.path.join(folderPath,f)) for f in uniResults]

    biResults = [f for f in os.listdir(folderPath) if os.path.isfile(os.path.join(folderPath, f)) and f[-6:]=="Bi.txt"]
    biResults.sort()
    biDfs = [pd.read_csv(os.path.join(folderPath,f)) for f in biResults]

    if "altGurobiUni.txt" in uniResults: 
        pltTitleUni = yTitle + " vs " + xTitle + "\nfor PDRS algorithms on " + dataSetName
        fileNameUni = yColumn + "VS" + xColumn + "_PDRS_on_" + dataSetName + ".png"
        pltTitleBi = yTitle + " vs " + xTitle + " \nfor NDRS algorithms on " + dataSetName
        fileNameBi = yColumn + "VS" + xColumn + "_NDRS_on_" + dataSetName + ".png"
    else:
        pltTitleUni = yTitle + " vs " + xTitle + "\nfor heuristic PDRS algorithms on " + dataSetName
        fileNameUni = yColumn + "VS" + xColumn + "_heur_PDRS_on_" + dataSetName  + ".png"
        pltTitleBi = yTitle + " vs " + xTitle + "\nfor heuristic NDRS algorithms on " + dataSetName
        fileNameBi = yColumn + "VS" + xColumn + "_heur_NDRS_on_" + dataSetName + ".png"



    if xColumn in uniDfs[0].columns:
        x = uniDfs[0][xColumn]
    elif os.path.isfile(os.path.join(folderPath,"parsData.txt")):
        extraStats = pd.read_csv(os.path.join(folderPath,"parsData.txt"))
        if xColumn in extraStats.column:
            x = extraStats[x]
        else:
            print("X axis couldnt be found!")
            return
    
    if not binsOff:
        x = pd.cut(x,nrBins).apply(lambda x: (x.left+x.right)/2)
    
    dfForMaskUni = pd.read_csv(folderPath + "/altGurobiUni.txt")
    uniMask = dfForMaskUni["solvedToCompletion"]==1
    print("Uni unterminated:",uniMask.size -  uniMask.sum())
    dfForMaskBi = pd.read_csv(folderPath + "/altGurobiBi.txt")
    biMask = dfForMaskBi["solvedToCompletion"]==1
    print("Bi unterminated:", biMask.size -biMask.sum())

    for df,name in zip(uniDfs,uniResults):
        #to do labels
        # plt.plot(df[yColumn],label=name)
        sns.lineplot(x=x[uniMask],y=df[yColumn][uniMask],label=nameMap[name],color=colorMap[name],marker=markerMap[name])
    plt.legend()
    plt.xlabel(xLabel)
    plt.ylabel(yLabel)
    # plt.title( pltTitleUni )
    if yScaleLog: 
        plt.yscale("log")
    if savePlots:
        plt.savefig("plots/"+ fileNameUni,bbox_inches='tight')
    plt.show()

    for df,name in zip(biDfs,biResults):
        #to do labels
        sns.lineplot(x=x[biMask],y=df[yColumn][biMask],label=nameMap[name],color=colorMap[name],marker=markerMap[name])
    plt.legend()
    plt.xlabel(xLabel)
    plt.ylabel(yLabel)
    # plt.title(pltTitleBi)
    if yScaleLog: 
        plt.yscale("log")
    if savePlots:
        plt.savefig("plots/"+ fileNameBi,bbox_inches='tight')
    plt.show()



