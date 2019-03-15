#pragma once

#include <iostream>
#include "Screen.h"

#include "TitleScreen.h"
#include "GameScreen.h"

class ScreenManager
{
public:
	~ScreenManager(); 
	static ScreenManager* GetInstance();

	void Initialize();
	void LoadContent(Network* network, sf::Clock* clock);
	void Update(sf::RenderWindow &Window);
	void UIUpdate(sf::RenderWindow &Window);
	void Render(sf::RenderWindow &Window);

	void AddScreen(Screen *screen, Network* network, sf::Clock* clock);

private:
	ScreenManager(); // Private constructor to prevent instancing
	ScreenManager(ScreenManager const&) {}; // = delete; // Copy constructor is private
	ScreenManager& operator=(ScreenManager const&) {}; // = delete; // Assignment operator is private

	static ScreenManager* instance;
	Screen *currentScreen;

};

