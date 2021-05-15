#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    std::cout << "Starting LogicBox..." << std::endl;
    sf::RenderWindow wnd(sf::VideoMode(500, 500), "LogicBox");

    while(wnd.isOpen())
    {
        sf::Event event;
        while(wnd.pollEvent(event))
        {
            switch(event.type)
            {
            case sf::Event::Closed:
                wnd.close();
            }
        }

        // tick

        // render
        wnd.clear();
        wnd.display();
    }

    return 0;
}
