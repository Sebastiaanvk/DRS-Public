#include <SFML/Graphics.hpp>
#include "graphs.h"
#include <thread>
#include <cmath>
//#include <X11/Xlib.h>

//void function(bool small_Green){
void function(){
    sf::RenderWindow window(sf::VideoMode(1000, 800), "SFML works!");
    sf::RenderWindow window2(sf::VideoMode(1000, 800), "SFML works!");

    sf::CircleShape shape(200.f);
    //if(small_Green)
        shape.setFillColor(sf::Color::Green);
    sf::CircleShape shape2(300.f);
    shape2.setFillColor(sf::Color::Red);


    int iteration = 0;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(shape);
        window.draw(shape2);
        window.display();

        window2.clear();
        window2.draw(shape2);
        window2.display();
        if(iteration%10==0) 
            shape2.setFillColor(sf::Color::Blue);
        else if(iteration%10==5)
            shape2.setFillColor(sf::Color::Red);
        iteration++;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }


}

void windowTest(){

// Initialize Xlib for thread safety
    //    XInitThreads();
    //    function();
        std::thread windowThread1(function);
    //    std::thread windowThread2(function);

        if (windowThread1.joinable())
            windowThread1.join();
    //    if (windowThread2.joinable())
    //        windowThread2.join();
 
}


void drawVertex(sf::RenderTexture& renderTexture, int horizontalPos, int verticalPos, float radius, sf::Color color){
    sf::CircleShape circle(radius);
    circle.setOrigin(circle.getRadius(),circle.getRadius());
    circle.setPosition(horizontalPos,verticalPos);
    circle.setFillColor(sf::Color::Red);
    circle.setOutlineThickness(3.f);
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
    text.setCharacterSize(radius);
   // Get the bounding box of the text
    sf::FloatRect textRect = text.getLocalBounds();

    // Set the origin to the center of the text
    text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    text.setPosition(horizontalPos,verticalPos);
    text.setFillColor(sf::Color::Black);
    renderTexture.draw(text);
}

void displayGraphTest(const MultipartiteGraph& G, int width, int height){
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

    if (!image.saveToFile("testImage.png")) {
        std::cout << "Saving to image failed!\n";
    }

}

void graphTest(){
    unsigned int seed =12;

    int nr_layers = 6;
    int layer_size = 20;
    int max_weight = 5;
    float edge_density = 0.3;


    std::map<int,int> elementsPerLayer = {
        {0,10},
        {1,12},
        {2,7},
        {3,5},
        {4,6},
        {5,10}
    };

//    for(int i=0; i<nr_layers; i++){
        //elementsPerLayer[i] = layer_size;
    //}
    MultipartiteGraph G = generate_random_graph(nr_layers,elementsPerLayer,max_weight,edge_density,seed);

    display_graph(G);
    std::cout << "Starting displaying in sfml\n";
    displayGraphTest(G,800,600);
    std::cout << "Finished making image\n";

}

int main(){

    graphTest();

   return 0;
}