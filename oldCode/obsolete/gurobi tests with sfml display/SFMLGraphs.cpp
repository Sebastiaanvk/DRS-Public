#include "SFMLGraphs.h"


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


void displayGraphPNG(const MultipartiteGraph& G, int width, int height, std::string filename){
    int nr_layers = G.layers;
    int max_layer_size = 0;
    for(int i=0; i<nr_layers; i++){
        max_layer_size = std::max(max_layer_size,G.elementsPerLayer.at(i));
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
    if (!font.loadFromFile("DejaVuSans.ttf"))
    {
        std::cout << "Couldn't load font\n";
    }
    
    for(int layer=0; layer<nr_layers; layer++){
        int horizontal_pos = 2*layer*horizontal_step+horizontal_step/2; 
        int nr_elements_layer = G.elementsPerLayer.at(layer);
        int vertical_step = height/nr_elements_layer;
        for(int i=0; i<nr_elements_layer; i++){
            int vertical_pos = i*vertical_step + vertical_step/2;
            drawVertex(renderTexture, horizontal_pos, vertical_pos, vertexRadius, sf::Color::Red);
            if(layer>0){
                drawValueInVertex(renderTexture, horizontal_pos, vertical_pos, vertexRadius, sf::Color::Black, G.values.at({layer,i}),font);
            }
        }
    }

    for(const auto& edge:G.edges){

        sf::Vector2f start(edge.first.layer*2*horizontal_step+horizontal_step/2,height/G.elementsPerLayer.at(edge.first.layer)*(edge.first.element+0.5));
        sf::Vector2f end(edge.second.layer*2*horizontal_step+horizontal_step/2,height/G.elementsPerLayer.at(edge.second.layer)*(edge.second.element+0.5));

        drawEdge(renderTexture, start, end, vertexRadius, sf::Color::Black);

    }




    renderTexture.display();
    sf::Image image = renderTexture.getTexture().copyToImage();

    if (!image.saveToFile("Images/"+filename +".png")) {
        std::cout << "Saving to image failed!\n";
    }
}



void displayGraphPNGPostOpt(const MultipartiteGraph& G, int width, int height, const ModelWrapper& modelWrapper, std::string filename){
    int nr_layers = G.layers;
    int max_layer_size = 0;
    for(int i=0; i<nr_layers; i++){
        max_layer_size = std::max(max_layer_size,G.elementsPerLayer.at(i));
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
    if (!font.loadFromFile("DejaVuSans.ttf"))
    {
        std::cout << "Couldn't load font\n";
    }
    
    for(int layer=0; layer<nr_layers; layer++){
        int horizontal_pos = 2*layer*horizontal_step+horizontal_step/2; 
        int nr_elements_layer = G.elementsPerLayer.at(layer);
        int vertical_step = height/nr_elements_layer;
        for(int i=0; i<nr_elements_layer; i++){
            int vertical_pos = i*vertical_step + vertical_step/2;
            sf::Color vertexColor = sf::Color::Red;            
            if(layer>0 && modelWrapper.vertexPicked({layer,i})){
                vertexColor = sf::Color::Green;
            } else if( modelWrapper.vertexPickedAsNeighbor({layer,i})) {
                vertexColor = sf::Color::Yellow;
            }

            drawVertex(renderTexture, horizontal_pos, vertical_pos, vertexRadius, vertexColor);
            if(layer>0){
                drawValueInVertex(renderTexture, horizontal_pos, vertical_pos, vertexRadius, sf::Color::Black, G.values.at({layer,i}),font);
            }
        }
    }

    for(const auto& edge:G.edges){

        sf::Vector2f start(edge.first.layer*2*horizontal_step+horizontal_step/2,height/G.elementsPerLayer.at(edge.first.layer)*(edge.first.element+0.5));
        sf::Vector2f end(edge.second.layer*2*horizontal_step+horizontal_step/2,height/G.elementsPerLayer.at(edge.second.layer)*(edge.second.element+0.5));

        drawEdge(renderTexture, start, end, vertexRadius, sf::Color::Black);

    }




    renderTexture.display();
    sf::Image image = renderTexture.getTexture().copyToImage();

    if (!image.saveToFile("Images/"+filename +".png")) {
        std::cout << "Saving to image failed!\n";
    }   



}


