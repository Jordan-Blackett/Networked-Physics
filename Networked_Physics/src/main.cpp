#include <iostream>
#include <SFML\Graphics.hpp>

#include "ScreenManager.h"

#include "Network.h"

#define ScreenWidth 1024
#define ScreenHeight 768

int main()
{
	std::cout << "Hello World" << std::endl;

	sf::RenderWindow window(sf::VideoMode(ScreenWidth, ScreenHeight, 32), "Network!");
	//window.setFramerateLimit(60);

	Network* network = nullptr;
	sf::Clock clock;

	// ScreenManager
	ScreenManager::GetInstance()->Initialize();
	ScreenManager::GetInstance()->LoadContent(network, &clock);

	// VSync
	window.setVerticalSyncEnabled(false);

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}
		
		// Update
		ScreenManager::GetInstance()->Update(window);
		ScreenManager::GetInstance()->UIUpdate(window);

		// Render
		window.clear(sf::Color::White);
		ScreenManager::GetInstance()->Render(window);
		window.display();
	}

	return 0;
}