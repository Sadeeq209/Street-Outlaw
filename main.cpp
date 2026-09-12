#include <SFML/Graphics.hpp>
#include "Game.h"

int main() {
    // Style explicitly excludes Resize -- window can't be dragged
    // bigger/smaller or maximized on ANY machine, so every player sees
    // the exact same fixed view this game was tuned for.
    sf::RenderWindow window(sf::VideoMode({800, 600}), "2D Endless Runner",
        sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    
    Game game(window);
    game.run();
    
    return 0;
}