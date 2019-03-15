#pragma once

#include <SFML\Graphics.hpp>
#include <vector>

#include <iostream>
#include <iomanip> // setprecision
#include <sstream> // stringstream


class DebugGraph
{
public:
	DebugGraph();
	~DebugGraph();

	void Init(sf::Vector2i position, std::string text);

	void Update(float data);
	void Render(sf::RenderWindow &Window);

private:
	// Bars
	sf::RectangleShape debugBars_[60];
	sf::Vector2i position_;
	float scrollSpeed_{ 0.6f };
	sf::Color barColour_{ sf::Color::Green };
	int maxDataSize{ 1400 };

	int numberOfBars_{ 60 };
	int barSpaceOffset_{ 5 };

	int lastBarIndex_;

	// Debug Text
	std::string text_;
	sf::Text debugText;

	sf::Font debugFont;
	int debugTextSize = 22;
	sf::Color debugTextColor{ sf::Color::Black };

	sf::Clock clock;
	sf::Time timeLastUpdated_;

};

