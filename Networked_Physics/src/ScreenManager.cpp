#include "ScreenManager.h"



// Singleton
ScreenManager* ScreenManager::GetInstance()
{
	static ScreenManager instance;
	return &instance;
}

ScreenManager::ScreenManager()
{
}

ScreenManager::~ScreenManager()
{
}

void ScreenManager::Initialize()
{
	currentScreen = new TitleScreen();
}

void ScreenManager::LoadContent(Network* network, sf::Clock* clock)
{
	currentScreen->LoadContent(network, clock);
}

void ScreenManager::Update(sf::RenderWindow &Window)
{
	currentScreen->Update(Window);
}

void ScreenManager::UIUpdate(sf::RenderWindow & Window)
{
	currentScreen->UIUpdate(Window);
}

void ScreenManager::Render(sf::RenderWindow &Window)
{
	currentScreen->Render(Window);
}

void ScreenManager::AddScreen(Screen *screen, Network* network, sf::Clock* clock)
{
	currentScreen->UnloadContent();
	delete currentScreen;
	currentScreen = screen;
	currentScreen->LoadContent(network, clock);
}