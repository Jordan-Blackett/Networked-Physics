#pragma once

#include <iostream>
#include <SFML\Graphics.hpp>

#include "Network.h"
#include "NetworkLockStep.h"
#include "NetworkSnapshot.h"
#include "NetworkState.h"
#include "NetworkPredictionAndReconciliation.h"

#define ScreenWidth 1024
#define ScreenHeight 768

class Screen
{
public:
	Screen();
	~Screen();

	virtual void LoadContent(Network* network, sf::Clock* clock);
	virtual void UnloadContent();
	virtual void Update(sf::RenderWindow &Window);
	virtual void UIUpdate(sf::RenderWindow &Window);
	virtual void Render(sf::RenderWindow &Window);

protected:
	Network* network_{ nullptr };
	// Tick Time
	sf::Clock* clock_{ nullptr };
	sf::Time timeLastUpdated_;
	sf::Time ticktime_ = sf::milliseconds(16); // 16 = 60 ticks (100 = 10 ticks)

private:
};

