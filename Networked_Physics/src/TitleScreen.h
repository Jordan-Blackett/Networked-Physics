#pragma once

#include "Screen.h"
#include "ScreenManager.h"

class TitleScreen : public Screen
{
public:
	TitleScreen();
	~TitleScreen();

	void LoadContent(Network* network, sf::Clock* clock);
	void UnloadContent();
	void Update(sf::RenderWindow &Window);
	void Render(sf::RenderWindow &Window);

private:

};

