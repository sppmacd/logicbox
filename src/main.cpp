#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>

// let's start from some basic tiling
// fixed array for now

#define TYPE_OTHER 0 // uses connections as additional data, so connections are not allowed
#define TYPE_CABLE 1
#define TYPE_LAYER 2
#define TYPE_GATE 3

#define TYPEOTHER_NONE 0
#define TYPEOTHER_INPUT 1
#define TYPEOTHER_OUTPUT 2

// order as in CSS :)
#define CONNECTION_UP 0x1
#define CONNECTION_RIGHT 0x2
#define CONNECTION_DOWN 0x4
#define CONNECTION_LEFT 0x8

struct Tile
{
    uint8_t type : 3;
    uint8_t connections : 4;
    bool power_state : 1;
};

Tile tiles[16][16];

void draw_tile(Tile& tile, uint8_t hovered_connection, sf::Vector2i position, sf::VertexArray& array)
{
    if(tile.type == TYPE_OTHER && tile.connections == TYPEOTHER_NONE)
        return;

    // Draw connections.
    if(tile.type != TYPE_OTHER)
    {
        if(tile.connections & CONNECTION_DOWN || hovered_connection & CONNECTION_DOWN)
        {
            sf::Color color = hovered_connection & CONNECTION_DOWN ? sf::Color::Red : sf::Color::White;
            array.append({sf::Vector2f(position.x - 1/16.f, position.y + 1/16.f), color});
            array.append({sf::Vector2f(position.x + 1/16.f, position.y + 1/16.f), color});
            array.append({sf::Vector2f(position.x + 1/16.f, position.y + 0.5), color});
            array.append({sf::Vector2f(position.x - 1/16.f, position.y + 0.5), color});
        }
        if(tile.connections & CONNECTION_UP || hovered_connection & CONNECTION_UP)
        {
            sf::Color color = hovered_connection & CONNECTION_UP ? sf::Color::Red : sf::Color::White;
            array.append({sf::Vector2f(position.x - 1/16.f, position.y - 1/16.f), color});
            array.append({sf::Vector2f(position.x + 1/16.f, position.y - 1/16.f), color});
            array.append({sf::Vector2f(position.x + 1/16.f, position.y - 0.5), color});
            array.append({sf::Vector2f(position.x - 1/16.f, position.y - 0.5), color});
        }

        if(tile.connections & CONNECTION_RIGHT || hovered_connection & CONNECTION_RIGHT)
        {
            sf::Color color = hovered_connection & CONNECTION_RIGHT ? sf::Color::Red : sf::Color::White;
            array.append({sf::Vector2f(position.x + 1/16.f, position.y - 1/16.f), color});
            array.append({sf::Vector2f(position.x + 1/16.f, position.y + 1/16.f), color});
            array.append({sf::Vector2f(position.x + 0.5, position.y + 1/16.f), color});
            array.append({sf::Vector2f(position.x + 0.5, position.y - 1/16.f), color});
        }
        if(tile.connections & CONNECTION_LEFT || hovered_connection & CONNECTION_LEFT)
        {
            sf::Color color = hovered_connection & CONNECTION_LEFT ? sf::Color::Red : sf::Color::White;
            array.append({sf::Vector2f(position.x - 1/16.f, position.y - 1/16.f), color});
            array.append({sf::Vector2f(position.x - 1/16.f, position.y + 1/16.f), color});
            array.append({sf::Vector2f(position.x - 0.5, position.y + 1/16.f), color});
            array.append({sf::Vector2f(position.x - 0.5, position.y - 1/16.f), color});
        }
    }

    // Tile-specific (foreground)
    switch(tile.type)
    {
        case TYPE_CABLE:
        {
            // always render some center
            array.append({sf::Vector2f(position.x - 1/16.f, position.y + 1/16.f)});
            array.append({sf::Vector2f(position.x + 1/16.f, position.y + 1/16.f)});
            array.append({sf::Vector2f(position.x + 1/16.f, position.y - 1/16.f)});
            array.append({sf::Vector2f(position.x - 1/16.f, position.y - 1/16.f)});
            break;
        }
        case TYPE_LAYER:
        {
            array.append({sf::Vector2f(position.x - 1/8.f, position.y + 1/8.f), sf::Color(0, 128, 0)});
            array.append({sf::Vector2f(position.x + 1/8.f, position.y + 1/8.f), sf::Color(0, 128, 0)});
            array.append({sf::Vector2f(position.x + 1/8.f, position.y - 1/8.f), sf::Color(0, 128, 0)});
            array.append({sf::Vector2f(position.x - 1/8.f, position.y - 1/8.f), sf::Color(0, 128, 0)});
            break;
        }
        case TYPE_GATE:
        {
            array.append({sf::Vector2f(position.x - 1/8.f, position.y + 1/8.f), sf::Color(128, 0, 0)});
            array.append({sf::Vector2f(position.x + 1/8.f, position.y + 1/8.f), sf::Color(128, 0, 0)});
            array.append({sf::Vector2f(position.x + 1/8.f, position.y - 1/8.f), sf::Color(128, 0, 0)});
            array.append({sf::Vector2f(position.x - 1/8.f, position.y - 1/8.f), sf::Color(128, 0, 0)});
            break;
        }
    }

}

Tile generate_tile()
{
    // this is temporary so don't care about warnings :)
    return Tile{rand() % 4, CONNECTION_DOWN | CONNECTION_LEFT, false};
}

int main()
{
    std::cout << "Starting LogicBox..." << std::endl;
    sf::RenderWindow wnd(sf::VideoMode(500, 500), "LogicBox");

    // generate some random stuff
    for(int x = 0; x < 16; x++)
    for(int y = 0; y < 16; y++)
    {
        tiles[x][y] = generate_tile();
    }

    float zoom = 1/16.f;

    sf::View map_view;
    map_view.setCenter({0.f, 0.f});

    auto update_view = [&](sf::Vector2u wnd_size) {
        map_view.setSize({static_cast<float>(wnd_size.x), static_cast<float>(wnd_size.y)});
        map_view.zoom(zoom);
    };
    update_view({500, 500});

    bool dragging = false;
    bool actually_dragging = false;
    sf::Vector2i last_mouse_pos;

    while(wnd.isOpen())
    {
        // Calculate the tile which the mouse is over now.
        sf::Vector2f mouse_pos_in_world = wnd.mapPixelToCoords(sf::Mouse::getPosition(wnd), map_view);
        sf::Vector2i hovered_tile = { static_cast<int>(mouse_pos_in_world.x + (mouse_pos_in_world.x < 0 ? -0.5 : 0.5)),
                                          static_cast<int>(mouse_pos_in_world.y + (mouse_pos_in_world.y < 0 ? -0.5 : 0.5)) };

        // Calculate currently hovered connection.
        uint8_t hovered_connection = 0;
        sf::Vector2f mouse_pos_relative_to_hovered_tile = mouse_pos_in_world - sf::Vector2f(hovered_tile);

        if(mouse_pos_relative_to_hovered_tile.y >= -1/16.f && mouse_pos_relative_to_hovered_tile.y <= 1/16.f)
        {
            if(mouse_pos_relative_to_hovered_tile.x < -1/16.f)
                hovered_connection |= CONNECTION_LEFT;
            else if(mouse_pos_relative_to_hovered_tile.x > 1/16.f)
                hovered_connection |= CONNECTION_RIGHT;
        }
        else if(mouse_pos_relative_to_hovered_tile.x >= -1/16.f && mouse_pos_relative_to_hovered_tile.x <= 1/16.f)
        {
            if(mouse_pos_relative_to_hovered_tile.y < -1/16.f)
                hovered_connection |= CONNECTION_UP;
            else if(mouse_pos_relative_to_hovered_tile.y > 1/16.f)
                hovered_connection |= CONNECTION_DOWN;
        }

        std::cout << (int)hovered_connection << std::endl;

        sf::Event event;
        while(wnd.pollEvent(event))
        {
            switch(event.type)
            {
                case sf::Event::Closed:
                    wnd.close();
                    break;
                case sf::Event::Resized:
                    update_view({event.size.width, event.size.height});
                    break;
                case sf::Event::MouseButtonPressed:
                    dragging = true;
                    last_mouse_pos = {event.mouseButton.x, event.mouseButton.y};
                    break;
                case sf::Event::MouseButtonReleased:
                    dragging = false;
                    actually_dragging = false;
                    last_mouse_pos = {};
                    break;
                case sf::Event::MouseMoved:
                {
                    if(dragging)
                    {
                        sf::Vector2i new_mouse_pos = { event.mouseMove.x, event.mouseMove.y };
                        sf::Vector2i screen_diff = new_mouse_pos - last_mouse_pos;

                        // Must move at least by 25 pixels to trigger
                        if(screen_diff.x*screen_diff.x+screen_diff.y*screen_diff.y < 625 && !actually_dragging)
                            break;

                        actually_dragging = true;
                        map_view.move(wnd.mapPixelToCoords(last_mouse_pos) - wnd.mapPixelToCoords(new_mouse_pos));
                        last_mouse_pos = new_mouse_pos;
                    }
                    break;
                }
                case sf::Event::MouseWheelScrolled:
                {
                    zoom *= (event.mouseWheelScroll.delta < 0 ? 2 : 0.5);
                    update_view(wnd.getSize());
                    break;
                }
            }
        }

        // tick

        // render
        wnd.clear();

        wnd.setView(map_view);

        sf::VertexArray array(sf::Quads, 4);
        for(int x = 0; x < 16; x++)
        for(int y = 0; y < 16; y++)
        {
            draw_tile(tiles[x][y], sf::Vector2i(x, y) == hovered_tile ? hovered_connection : 0x0, {x, y}, array);
        }
        wnd.draw(array);

        sf::RectangleShape highlight(sf::Vector2f(1, 1));
        highlight.setPosition(sf::Vector2f(hovered_tile) - sf::Vector2f(0.5, 0.5));
        highlight.setFillColor(sf::Color::Transparent);
        highlight.setOutlineColor(sf::Color::Red);
        highlight.setOutlineThickness(1/32.f);
        wnd.draw(highlight);

        wnd.display();
    }

    return 0;
}
