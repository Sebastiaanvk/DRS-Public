#include "../include/SFMLGraphs.h"


void drawVertex(sf::RenderTexture& renderTexture, int horizontalPos, int verticalPos, float radius, sf::Color color){
    sf::CircleShape circle(radius);
    circle.setOutlineThickness(1.f);
    sf::FloatRect textRect = circle.getLocalBounds();

    // Set the origin to the center of the text
    circle.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);

//    circle.setOrigin(circle.getRadius(),circle.getRadius());
    circle.setPosition(horizontalPos,verticalPos);
    circle.setFillColor(color);
    circle.setOutlineColor(sf::Color::Black);
    renderTexture.draw(circle);
}

void drawEdge(sf::RenderTexture& renderTexture, sf::Vector2f start, sf::Vector2f end, float offset, sf::Color color ){

    sf::Vector2f direction = end - start;
    sf::Vector2f normalizedDirection = direction/std::sqrt(direction.x*direction.x+direction.y*direction.y);

    sf::Vector2f startOffset = start + offset * normalizedDirection;
    sf::Vector2f endOffset = end - offset * normalizedDirection;


    sf::Vertex line[] = {
        sf::Vertex(startOffset,color),
        sf::Vertex(endOffset,color)
    };

    renderTexture.draw(line, 2, sf::Lines);
}

void drawValueInVertex(sf::RenderTexture& renderTexture, int horizontalPos, int verticalPos, float radius, sf::Color color, int value, sf::Font font){
    sf::Text text;
    text.setFont(font);
    text.setString(std::to_string(value));
    text.setCharacterSize(radius*0.9);
   // Get the bounding box of the text
    sf::FloatRect textRect = text.getLocalBounds();

    // Set the origin to the center of the text
    text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    text.setPosition(horizontalPos,verticalPos);
    text.setFillColor(sf::Color::Black);
    renderTexture.draw(text);
}


void displayGraphPNG(const MPGraphInterface& G, int width, int height, std::string filename){
    int nr_layers = G.getLayers();
    int max_layer_size = 0;
    for(int i=0; i<nr_layers; i++){
        max_layer_size = std::max(max_layer_size,G.getElementsInLayer(i));
    }
    
    int min_vertical_step = height/max_layer_size;
    int horizontal_step = width/(nr_layers*2-1);

    float vertexRadius = std::min(min_vertical_step/2*0.5,min_vertical_step/2*0.5);

    sf::RenderTexture renderTexture;

    if(!renderTexture.create(width,height)) {
        std::cout << "Couldn't create texture!\n";
        return;
    }
    
    sf::Font font;
    if (!font.loadFromFile("assets/DejaVuSans.ttf"))
    {
        std::cout << "Couldn't load font\n";
    }
    
    for(int layer=0; layer<nr_layers; layer++){
        int horizontal_pos = 2*layer*horizontal_step+horizontal_step/2; 
        int nr_elements_layer = G.getElementsInLayer(layer);
        int vertical_step = height/nr_elements_layer;
        for(int i=0; i<nr_elements_layer; i++){
            int vertical_pos = i*vertical_step + vertical_step/2;
            drawVertex(renderTexture, horizontal_pos, vertical_pos, vertexRadius, sf::Color::Red);
            drawValueInVertex(renderTexture, horizontal_pos, vertical_pos, vertexRadius, sf::Color::Black, G.getValue({layer,i}) ,font);
        }
    }
    std::unique_ptr<EdgeIterator> it = G.getEdgeIterator();
    while(it->has_next()){
        Edge edge = it->current();
        sf::Vector2f start(edge.first.layer*2*horizontal_step+horizontal_step/2,height/G.getElementsInLayer(edge.first.layer)*(edge.first.element+0.5));
        sf::Vector2f end(edge.second.layer*2*horizontal_step+horizontal_step/2,height/G.getElementsInLayer(edge.second.layer)*(edge.second.element+0.5));

        drawEdge(renderTexture, start, end, vertexRadius, sf::Color::Black);
        it->next();
    }

    renderTexture.display();
    sf::Image image = renderTexture.getTexture().copyToImage();

    if (!image.saveToFile("output/images/"+filename +".png")) {
        std::cout << "Saving to image failed!\n";
    }
}



void displayGraphPNGPostOpt(const MPGraphInterface& G, int width, int height, OptimizableInterface *modelWrapper, std::string filename){
    int nr_layers = G.getLayers();
    int max_layer_size = 0;
    for(int i=0; i<nr_layers; i++){
        max_layer_size = std::max(max_layer_size, G.getElementsInLayer(i));
    }
    
    int min_vertical_step = height/max_layer_size;
    int horizontal_step = width/(nr_layers*2-1);

    float vertexRadius = std::min(min_vertical_step/2*0.5,min_vertical_step/2*0.5);

    sf::RenderTexture renderTexture;

    if(!renderTexture.create(width,height)) {
        std::cout << "Couldn't create texture!\n";
        return;
    }
    
    sf::Font font;
    if (!font.loadFromFile("assets/DejaVuSans.ttf"))
    {
        std::cout << "Couldn't load font\n";
    }
    
    for(int layer=0; layer<nr_layers; layer++){
        int horizontal_pos = 2*layer*horizontal_step+horizontal_step/2; 
        int nr_elements_layer = G.getElementsInLayer(layer);
        int vertical_step = height/nr_elements_layer;
        for(int i=0; i<nr_elements_layer; i++){
            int vertical_pos = i*vertical_step + vertical_step/2;
            sf::Color vertexColor = sf::Color::Red;            
            if( modelWrapper->vertexPicked({layer,i})){
                vertexColor = sf::Color::Green;
            } else if( modelWrapper->vertexPickedAsNeighbor({layer,i})) {
                vertexColor = sf::Color::Yellow;
            }

            drawVertex(renderTexture, horizontal_pos, vertical_pos, vertexRadius, vertexColor);
            drawValueInVertex(renderTexture, horizontal_pos, vertical_pos, vertexRadius, sf::Color::Black, G.getValue({layer,i}),font);
        }
    }
    std::unique_ptr<EdgeIterator> it = G.getEdgeIterator();
    while(it->has_next()){
        Edge edge = it->current();
        sf::Vector2f start(edge.first.layer*2*horizontal_step+horizontal_step/2,height/G.getElementsInLayer(edge.first.layer)*(edge.first.element+0.5));
        sf::Vector2f end(edge.second.layer*2*horizontal_step+horizontal_step/2,height/G.getElementsInLayer(edge.second.layer)*(edge.second.element+0.5));

        drawEdge(renderTexture, start, end, vertexRadius, sf::Color::Black);
        it->next();
    }




    renderTexture.display();
    sf::Image image = renderTexture.getTexture().copyToImage();

    if (!image.saveToFile("output/images/"+filename +".png")) {
        std::cout << "Saving to image failed!\n";
    }   



}


sf::Color enumColToSF(Colour col){
    switch (col){
        case Green:
            return sf::Color::Green;
            break;
        case Yellow:
            return sf::Color::Yellow;
            break;
        default:
            return sf::Color::Red;
            break;
    }
}

bool nodeInGraph(const VertexIndex& vi, const std::map<VertexIndex, Colour>& nodeColorMap ){
    if(nodeColorMap.count(vi)==0)
        return true;
    return !(nodeColorMap.at(vi)==0);
}

void displayGraphPNGColorMap(const MPGraphInterface& G, int width, int height, const std::map<VertexIndex, Colour>& nodeColorMap, std::string filename ){
    int nr_layers = G.getLayers();
    int max_layer_size = 0;
    for(int i=0; i<nr_layers; i++){
        max_layer_size = std::max(max_layer_size, G.getElementsInLayer(i));
    }
    
    int min_vertical_step = height/max_layer_size;
    int horizontal_step = width/(nr_layers*2-1);

    float vertexRadius = std::min(min_vertical_step/2*0.5,min_vertical_step/2*0.5);

    sf::RenderTexture renderTexture;

    if(!renderTexture.create(width,height)) {
        std::cout << "Couldn't create texture!\n";
        return;
    }
    
    sf::Font font;
    if (!font.loadFromFile("assets/DejaVuSans.ttf"))
    {
        std::cout << "Couldn't load font\n";
    }
    
    for(int layer=0; layer<nr_layers; layer++){
        int horizontal_pos = 2*layer*horizontal_step+horizontal_step/2; 
        int nr_elements_layer = G.getElementsInLayer(layer);
        int vertical_step = height/nr_elements_layer;
        for(int i=0; i<nr_elements_layer; i++){
            if(nodeInGraph({layer,i}, nodeColorMap)) {
                int vertical_pos = i*vertical_step + vertical_step/2;
                sf::Color vertexColor = sf::Color::Red;
                if(nodeColorMap.count({layer,i})!=0){
                    vertexColor = enumColToSF(nodeColorMap.at({layer,i}));
                }

                drawVertex(renderTexture, horizontal_pos, vertical_pos, vertexRadius, vertexColor);
                drawValueInVertex(renderTexture, horizontal_pos, vertical_pos, vertexRadius, sf::Color::Black, G.getValue({layer,i}),font);
            }

        }
    }
    std::unique_ptr<EdgeIterator> it = G.getEdgeIterator();
    while(it->has_next()){
        Edge edge = it->current();
        if(nodeInGraph(edge.first,nodeColorMap) && nodeInGraph(edge.second,nodeColorMap)){
            sf::Vector2f start(edge.first.layer*2*horizontal_step+horizontal_step/2,height/G.getElementsInLayer(edge.first.layer)*(edge.first.element+0.5));
            sf::Vector2f end(edge.second.layer*2*horizontal_step+horizontal_step/2,height/G.getElementsInLayer(edge.second.layer)*(edge.second.element+0.5));
            drawEdge(renderTexture, start, end, vertexRadius, sf::Color::Black);
        }
        it->next();
    }

    renderTexture.display();
    sf::Image image = renderTexture.getTexture().copyToImage();

    if (!image.saveToFile("output/images/"+filename +".png")) {
        std::cout << "Saving to image failed!\n";
    }
}


void displayGraphPNGNodesNbsAsInts(const MPGraphInterface& G, int width, int height, const std::set<int>& current_nodes, const std::set<int>& current_nbs, std::string filename ){
    int nr_layers = G.getLayers();
    int max_layer_size = 0;
    for(int i=0; i<nr_layers; i++){
        max_layer_size = std::max(max_layer_size, G.getElementsInLayer(i));
    }
    
    int min_vertical_step = height/max_layer_size;
    int horizontal_step = width/(nr_layers*2-1);

    float vertexRadius = std::min(min_vertical_step/2*0.5,min_vertical_step/2*0.5);

    sf::RenderTexture renderTexture;

    if(!renderTexture.create(width,height)) {
        std::cout << "Couldn't create texture!\n";
        return;
    }
    
    sf::Font font;
    if (!font.loadFromFile("assets/DejaVuSans.ttf"))
    {
        std::cout << "Couldn't load font\n";
    }

    std::map<VertexIndex,int> viToInt;
    int vertex = 0;
    for(int layer=0; layer<nr_layers; layer++){
        int horizontal_pos = 2*layer*horizontal_step+horizontal_step/2; 
        int nr_elements_layer = G.getElementsInLayer(layer);
        int vertical_step = height/nr_elements_layer;
        for(int i=0; i<nr_elements_layer; i++){
            int vertical_pos = i*vertical_step + vertical_step/2;
            sf::Color vertexColor = sf::Color::Red;
            if(current_nodes.count(vertex)){
                vertexColor = sf::Color::Green; 
            } else if(current_nbs.count(vertex)){
                vertexColor = sf::Color::Yellow;
            }

            drawVertex(renderTexture, horizontal_pos, vertical_pos, vertexRadius, vertexColor);
            drawValueInVertex(renderTexture, horizontal_pos, vertical_pos, vertexRadius, sf::Color::Black, G.getValue({layer,i}),font);
            viToInt[{layer,i}] = vertex;
            vertex++;
        }
    }
    std::unique_ptr<EdgeIterator> it = G.getEdgeIterator();
    while(it->has_next()){
        Edge edge = it->current();
        sf::Vector2f start(edge.first.layer*2*horizontal_step+horizontal_step/2,height/G.getElementsInLayer(edge.first.layer)*(edge.first.element+0.5));
        sf::Vector2f end(edge.second.layer*2*horizontal_step+horizontal_step/2,height/G.getElementsInLayer(edge.second.layer)*(edge.second.element+0.5));
        int firstVertex = viToInt[edge.first];
        int secondVertex = viToInt[edge.second];
        if(current_nodes.count(firstVertex)+current_nodes.count(secondVertex)>1){
            drawEdge(renderTexture, start, end, vertexRadius, sf::Color::Black);
        } else if(current_nodes.count(firstVertex)+current_nbs.count(secondVertex)>1||current_nbs.count(firstVertex)+current_nodes.count(secondVertex)>1){
            drawEdge(renderTexture, start, end, vertexRadius, sf::Color::Black);

        }

        it->next();
    }

    renderTexture.display();
    sf::Image image = renderTexture.getTexture().copyToImage();

    if (!image.saveToFile("output/images/"+filename +".png")) {
        std::cout << "Saving to image failed!\n";
    }


}



void displayGraphPNGNodesNbsPicked(const MPGraphInterface& G, int width, int height, const std::set<VertexIndex>& current_nodes, const std::set<VertexIndex>& current_nbs, std::string filename ){
    int nr_layers = G.getLayers();
    int max_layer_size = 0;
    for(int i=0; i<nr_layers; i++){
        max_layer_size = std::max(max_layer_size, G.getElementsInLayer(i));
    }
    
    int min_vertical_step = height/max_layer_size;
    int horizontal_step = width/(nr_layers*2-1);

    float vertexRadius = std::min(min_vertical_step/2*0.5,min_vertical_step/2*0.5);

    sf::RenderTexture renderTexture;

    if(!renderTexture.create(width,height)) {
        std::cout << "Couldn't create texture!\n";
        return;
    }
    
    sf::Font font;
    if (!font.loadFromFile("assets/DejaVuSans.ttf"))
    {
        std::cout << "Couldn't load font\n";
    }

    for(int layer=0; layer<nr_layers; layer++){
        int horizontal_pos = 2*layer*horizontal_step+horizontal_step/2; 
        int nr_elements_layer = G.getElementsInLayer(layer);
        int vertical_step = height/nr_elements_layer;
        for(int i=0; i<nr_elements_layer; i++){
            int vertical_pos = i*vertical_step + vertical_step/2;
            sf::Color vertexColor = sf::Color::Red;
            if(current_nodes.count({layer,i})){
                vertexColor = sf::Color::Green; 
            } else if(current_nbs.count({layer,i})){
                vertexColor = sf::Color::Yellow;
            }

            drawVertex(renderTexture, horizontal_pos, vertical_pos, vertexRadius, vertexColor);
            drawValueInVertex(renderTexture, horizontal_pos, vertical_pos, vertexRadius, sf::Color::Black, G.getValue({layer,i}),font);
        }
    }
    std::unique_ptr<EdgeIterator> it = G.getEdgeIterator();
    while(it->has_next()){
        Edge edge = it->current();
        sf::Vector2f start(edge.first.layer*2*horizontal_step+horizontal_step/2,height/G.getElementsInLayer(edge.first.layer)*(edge.first.element+0.5));
        sf::Vector2f end(edge.second.layer*2*horizontal_step+horizontal_step/2,height/G.getElementsInLayer(edge.second.layer)*(edge.second.element+0.5));
        drawEdge(renderTexture, start, end, vertexRadius, sf::Color::Black);
        it->next();
    }

    renderTexture.display();
    sf::Image image = renderTexture.getTexture().copyToImage();

    if (!image.saveToFile("output/images/"+filename +".png")) {
        std::cout << "Saving to image failed!\n";
    }


}