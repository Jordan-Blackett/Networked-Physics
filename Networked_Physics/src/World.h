//#pragma once
//
//#include <iostream>
//#include <SFML\Graphics.hpp>
//#include <Box2D\Box2D.h>
//#include <vector>
//
//#include "NetworkLockStep.h"
//
//#include "Player.h"
//#include "EntityBox.h"
//
//#include <SFGUI\SFGUI.hpp>
//#include <SFGUI\Widgets.hpp>
//
//struct WorldWall
//{
//	sf::RectangleShape rectangle;
//	b2Body* body;
//};
//
//#define ScreenWidth 1024
//#define ScreenHeight 768
//
//class World
//{
//public:
//	World();
//	~World();
//
//	void Init();
//	void InitPhysicalWorld();
//	void InitDebugText();
//
//	void CreateWall(float posX, float posY, sf::Vector2f size);
//
//	void Update(sf::RenderWindow &Window);
//	void Render(sf::RenderWindow &Window);
//	void Debug();
//
//private:
//	// Network
//	Network* network;
//
//	// Physics
//	b2World* world;
//	const float scale = 32.f; // Convert between pixel and real-world coordinates
//	const float gravity = 0.0f; // 9.8; // -scale / 0.7f;
//
//	//Entities
//	// Player
//	Player player;
//	sf::Vector2f playerSize{ sf::Vector2f(40, 40) };
//	sf::Vector2f boxSize{ sf::Vector2f(15, 15) };
//
//	// Boxes
//	std::vector<EntityBox*> boxes;
//	std::vector<WorldWall*> walls;
//
//	// Grid
//	int rows = 20;
//	int columns = 15;
//	int gridOffset = 50;
//
//	// Debug Text
//	sf::Text debugConnectedStatusText;
//	sf::Text debugSynchronisationMethodText;
//	sf::Text debugPacketSizeText;
//	sf::Text debugNumOfBoxesText;
//	sf::Text debugNumOfActiveBoxesText;
//
//	sf::Font debugFont;
//	int debugTextSize = 12;
//	sf::Color debugTextColor{ sf::Color::Black };
//
//	// SFGUI
//	sfg::SFGUI sfgui;
//
//	NetworkSynchronisationMethod networkMethod{ NetworkSynchronisationMethod::LockStep };
//
//};
//
