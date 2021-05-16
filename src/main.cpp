#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <unistd.h>
#include <functional>

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

// let's display connection highlight also for empty tiles

void draw_connection_highlight(Tile& tile, uint8_t hovered_connection, sf::Vector2i position, sf::VertexArray& array)
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

void draw_tile(Tile& tile, uint8_t hovered_connection, bool is_hovered_tile, sf::Vector2i position, sf::VertexArray& array)
{
    // Draw connections.
    draw_connection_highlight(tile, hovered_connection, position, array);

    if(tile.type == TYPE_OTHER && tile.connections == TYPEOTHER_NONE)
    {
        // some center
        if(is_hovered_tile)
        {
            array.append({sf::Vector2f(position.x - 1/16.f, position.y + 1/16.f), sf::Color::Red});
            array.append({sf::Vector2f(position.x + 1/16.f, position.y + 1/16.f), sf::Color::Red});
            array.append({sf::Vector2f(position.x + 1/16.f, position.y - 1/16.f), sf::Color::Red});
            array.append({sf::Vector2f(position.x - 1/16.f, position.y - 1/16.f), sf::Color::Red});
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
    return Tile{TYPE_OTHER, TYPEOTHER_NONE, false};
}

enum class PlaceMode
{
    Cable,
    Layer,
    Gate,
    Empty, // = delete
    __Count // keep last
};

class Button
{
public:
    Button(sf::Font& font, sf::Vector2f pos, sf::Vector2f size, std::string label)
        : m_position(pos), m_size(size), m_label(label), m_font(font) {}

    void render(sf::RenderTarget& target) const
    {
        sf::RectangleShape rs(m_size);
        rs.setPosition(m_position);

        // TODO: change appearance on hover and active
        if(m_hover)
            rs.setFillColor(sf::Color(200, 200, 200));
        else
            rs.setFillColor(sf::Color(160, 160, 160));
        rs.setOutlineColor(sf::Color(100, 100, 100));
        rs.setOutlineThickness(m_active ? 3.f : 1.f);
        target.draw(rs);

        sf::Text text(m_label, m_font, 15);
        auto text_bounds = text.getLocalBounds();
        text.setOrigin(text_bounds.width / 2, text_bounds.height / 2);
        text.setPosition(m_position + m_size / 2.f);
        text.setFillColor(sf::Color::Black);
        target.draw(text);
    }

    // returns true if event was handled
    bool handle_event(sf::Event& event)
    {
        if(event.type == sf::Event::MouseMoved)
        {
            m_hover = is_hover({static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y)});
        }
        else if(event.type == sf::Event::MouseButtonPressed && m_hover)
        {
            // don't allow starting drag on buttons
            return true;
        }
        else if(event.type == sf::Event::MouseButtonReleased && m_hover)
        {
            std::cout << "button " << m_label << " clicked!" << std::endl;
            if(m_callback)
                m_callback();
            return true;
        }
        return false;
    }

    bool is_hover(sf::Vector2f pos) const
    {
        return pos.x >= m_position.x && pos.y >= m_position.y && pos.x < m_position.x + m_size.x && pos.y < m_position.y + m_size.y;
    }

    void set_callback(std::function<void()> callback)
    {
        m_callback = callback;
    }

    void set_active(bool active) { m_active = active; }

private:
    std::function<void()> m_callback;
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    std::string m_label;
    sf::Font m_font;
    bool m_hover = false;
    bool m_active = false;
};

int main()
{
    std::cout << "Starting LogicBox..." << std::endl;
    sf::RenderWindow wnd(sf::VideoMode(500, 500), "LogicBox");

    sf::Font font;
    if(!font.loadFromFile("res/font.ttf"))
    {
        std::cout << "Failed to load font :(" << std::endl;
        return 1;
    }

    // generate some random stuff
    for(int x = 0; x < 16; x++)
    for(int y = 0; y < 16; y++)
    {
        tiles[x][y] = generate_tile();
    }

    float zoom = 1/16.f;

    sf::View map_view;
    map_view.setCenter({0.f, 0.f});

    sf::View gui_view;
;
    auto update_view = [&](sf::Vector2u wnd_size) {
        map_view.setSize({static_cast<float>(wnd_size.x), static_cast<float>(wnd_size.y)});
        map_view.zoom(zoom);
        gui_view = sf::View(sf::FloatRect(0, 0, static_cast<float>(wnd_size.x), static_cast<float>(wnd_size.y)));
    };
    update_view({500, 500});

    bool dragging = false;
    bool actually_dragging = false;
    sf::Vector2i last_mouse_pos;

    // GUI
    std::vector<std::shared_ptr<Button>> buttons;

    auto place_mode_to_string = [](PlaceMode mode)
    {
        switch(mode)
        {
        case PlaceMode::Cable: return "Cable";
        case PlaceMode::Layer: return "Layer";
        case PlaceMode::Gate: return "Gate";
        case PlaceMode::Empty: return "Delete";
        }
        return "??";
    };

    PlaceMode current_place_mode = PlaceMode::Cable;

    const float BUTTON_SIZE = 64.f;
    for(size_t s = 0; s < static_cast<size_t>(PlaceMode::__Count); s++)
    {
        auto button = std::make_shared<Button>(font,
                                               sf::Vector2f(10.f, s * (BUTTON_SIZE + 10.f) + 10.f),
                                               sf::Vector2f(BUTTON_SIZE, BUTTON_SIZE),
                                               place_mode_to_string(static_cast<PlaceMode>(s)));
        button->set_callback([&,s,button]() {
            current_place_mode = static_cast<PlaceMode>(s);
            for(auto& other_button: buttons)
            {
                other_button->set_active(other_button == button);
            }
        });
        buttons.push_back(button);
    }
    buttons[0]->set_active(true);

    while(wnd.isOpen())
    {
        // Calculate the tile which the mouse is over now.
        sf::Vector2f mouse_pos_in_world = wnd.mapPixelToCoords(sf::Mouse::getPosition(wnd), map_view);
        sf::Vector2i hovered_tile = { static_cast<int>(mouse_pos_in_world.x + (mouse_pos_in_world.x < 0 ? -0.5 : 0.5)),
                                          static_cast<int>(mouse_pos_in_world.y + (mouse_pos_in_world.y < 0 ? -0.5 : 0.5)) };

        // Calculate currently hovered connection.
        uint8_t hovered_connection = 0;
        sf::Vector2f mouse_pos_relative_to_hovered_tile = mouse_pos_in_world - sf::Vector2f(hovered_tile);

        // let's improve connection hover range
        if(mouse_pos_relative_to_hovered_tile.y >= -1/8.f && mouse_pos_relative_to_hovered_tile.y <= 1/8.f)
        {
            if(mouse_pos_relative_to_hovered_tile.x < -1/8.f)
                hovered_connection |= CONNECTION_LEFT;
            else if(mouse_pos_relative_to_hovered_tile.x > 1/8.f)
                hovered_connection |= CONNECTION_RIGHT;
        }
        else if(mouse_pos_relative_to_hovered_tile.x >= -1/8.f && mouse_pos_relative_to_hovered_tile.x <= 1/8.f)
        {
            if(mouse_pos_relative_to_hovered_tile.y < -1/8.f)
                hovered_connection |= CONNECTION_UP;
            else if(mouse_pos_relative_to_hovered_tile.y > 1/8.f)
                hovered_connection |= CONNECTION_DOWN;
        }

        sf::Event event;
        while(wnd.pollEvent(event))
        {
            bool event_handled_by_gui = false;
            for(auto& button: buttons)
            {
                event_handled_by_gui |= button->handle_event(event);
            }
            if(event_handled_by_gui)
                continue;

            switch(event.type)
            {
                case sf::Event::Closed:
                    wnd.close();
                    break;
                case sf::Event::Resized:
                    update_view({event.size.width, event.size.height});
                    break;
                case sf::Event::MouseButtonPressed:
                    // or everything the same button :)
                    if(event.mouseButton.button == sf::Mouse::Left)
                    {
                        dragging = true;
                        last_mouse_pos = {event.mouseButton.x, event.mouseButton.y};
                    }
                    break;
                case sf::Event::MouseButtonReleased:
                    if(event.mouseButton.button == sf::Mouse::Left)
                    {
                        if(!actually_dragging)
                        {
                            // Place if not dragged.
                            if(hovered_tile.x >= 0 && hovered_tile.x < 16 && hovered_tile.y >= 0 && hovered_tile.y < 16)
                            {
                                auto& tile = tiles[hovered_tile.x][hovered_tile.y];

                                std::cout << static_cast<size_t>(current_place_mode) << std::endl;

                                // Empty tile
                                // TODO: Implement place modes for TYPE_OTHER (input, output etc.)
                                switch(current_place_mode)
                                {
                                case PlaceMode::Cable:
                                    tile.type = TYPE_CABLE;
                                    break;
                                case PlaceMode::Layer:
                                    tile.type = TYPE_LAYER;
                                    break;
                                case PlaceMode::Gate:
                                    tile.type = TYPE_GATE;
                                    break;
                                case PlaceMode::Empty:
                                    if(tile.type != TYPE_OTHER || tile.type != TYPEOTHER_NONE)
                                    {
                                        tile.type = TYPE_OTHER;
                                        tile.connections = TYPEOTHER_NONE;
                                    }
                                }

                                // Place connection.
                                if(tile.type != TYPE_OTHER)
                                {
                                    if(hovered_connection)
                                        tile.connections ^= hovered_connection; // "toggle" connection
                                }
                            }
                        }
                        dragging = false;
                        actually_dragging = false;
                        last_mouse_pos = {};
                    }
                    break;
                case sf::Event::MouseMoved:
                {
                    if(dragging)
                    {
                        sf::Vector2i new_mouse_pos = { event.mouseMove.x, event.mouseMove.y };
                        sf::Vector2i screen_diff = new_mouse_pos - last_mouse_pos;

                        // Must move at least by 50 pixels to trigger
                        if(screen_diff.x*screen_diff.x+screen_diff.y*screen_diff.y < 50*50 && !actually_dragging)
                            break;

                        actually_dragging = true;
                        map_view.move(wnd.mapPixelToCoords(last_mouse_pos, map_view) - wnd.mapPixelToCoords(new_mouse_pos, map_view));
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

        // map
        wnd.setView(map_view);

        // map border
        sf::RectangleShape border(sf::Vector2f(16.f, 16.f));
        border.setPosition(-0.5f, -0.5f);
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineColor(sf::Color::Red);
        border.setOutlineThickness(1/32.f);
        wnd.draw(border);

        sf::VertexArray array(sf::Quads, 4);
        for(int x = 0; x < 16; x++)
        for(int y = 0; y < 16; y++)
        {
            bool is_hovered_tile = sf::Vector2i(x, y) == hovered_tile;
            draw_tile(tiles[x][y], is_hovered_tile ? hovered_connection : 0x0, is_hovered_tile, {x, y}, array);
        }
        wnd.draw(array);

        // don't display highlight for out-of-range tiles
        if(hovered_tile.x >= 0 && hovered_tile.x < 16 && hovered_tile.y >= 0 && hovered_tile.y < 16)
        {
            sf::RectangleShape highlight(sf::Vector2f(1, 1));
            highlight.setPosition(sf::Vector2f(hovered_tile) - sf::Vector2f(0.5, 0.5));
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setOutlineColor(sf::Color::Red);
            highlight.setOutlineThickness(1/32.f);
            wnd.draw(highlight);
        }

        // gui
        wnd.setView(gui_view);

        for(auto& button: buttons)
        {
            button->render(wnd);
        }

        wnd.display();
    }

    return 0;
}
