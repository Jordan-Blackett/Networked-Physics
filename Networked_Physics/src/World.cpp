//#include "World.h"
//
//
//
//World::World()
//{
//}
//
//
//World::~World()
//{
//}
//
//void World::Init()
//{
//	// Prepare the world
//	InitPhysicalWorld();
//
//	// Init Walls
//	sf::Vector2f wallTopBottom = sf::Vector2f(ScreenWidth, 10);
//	sf::Vector2f wallLeftRight = sf::Vector2f(10, ScreenHeight);
//
//	CreateWall(ScreenWidth / 2, 0, wallTopBottom); // Top
//	CreateWall(ScreenWidth / 2, ScreenHeight, wallTopBottom); // Bottom
//	CreateWall(0, ScreenHeight / 2, wallLeftRight); // Left
//	CreateWall(ScreenWidth, ScreenHeight / 2, wallLeftRight); // Right
//
//	// Init Player and Boxes
//	player.init(world, sf::Vector2i(200, 400), playerSize, scale);
//
//	// Grid
//	for (int i = 0; i < rows; i++)
//	{
//		for (int j = 0; j < columns; j++)
//		{
//			EntityBox* box = new EntityBox();
//			box->init(world, sf::Vector2i((i * gridOffset) + 40, (j * gridOffset) + 40), boxSize, scale);
//			boxes.push_back(box);
//		}
//	}
//
//	// Init Network
//	switch (networkMethod) {
//	case NetworkSynchronisationMethod::LockStep:
//			network = new NetworkLockStep();
//		break;
//	}
//
//	// Debug Text
//	InitDebugText();
//
//	//SFGUI
//	// Create the label
//	auto label = sfg::Label::Create("Hello world!");
//
//	// Create a simple button and connect the click signal
//	auto button = sfg::Button::Create("Greet SFGUI!");
//	button->GetSignal(sfg::Widget::OnLeftClick).Connect([label] { label->SetText("Hello SFGUI, pleased to meet you!"); });
//}
//
//void World::InitPhysicalWorld()
//{
//	world = new b2World(b2Vec2(0.0f, gravity));
//	world->SetAllowSleeping(true);
//	world->SetContinuousPhysics(true);
//}
//
//void World::InitDebugText()
//{
//	if (!debugFont.loadFromFile("Resources/Fonts/closeness/Closeness.ttf")){}
//
//	debugConnectedStatusText.setFont(debugFont);
//	debugConnectedStatusText.setCharacterSize(debugTextSize);
//	debugConnectedStatusText.setFillColor(debugTextColor);
//	debugConnectedStatusText.setPosition(sf::Vector2f(540, 10));
//	debugConnectedStatusText.setString("Not Connected");
//
//	debugPacketSizeText.setFont(debugFont);
//	debugPacketSizeText.setCharacterSize(debugTextSize);
//	debugPacketSizeText.setFillColor(debugTextColor);
//	debugPacketSizeText.setPosition(sf::Vector2f(140, 10));
//
//	debugNumOfBoxesText.setFont(debugFont);
//	debugNumOfBoxesText.setCharacterSize(debugTextSize);
//	debugNumOfBoxesText.setFillColor(debugTextColor);
//	debugNumOfBoxesText.setPosition(sf::Vector2f(10, 10));
//	debugNumOfBoxesText.setString("Boxes: " + std::to_string(boxes.size()));
//
//	debugNumOfActiveBoxesText.setFont(debugFont);
//	debugNumOfActiveBoxesText.setCharacterSize(debugTextSize);
//	debugNumOfActiveBoxesText.setFillColor(debugTextColor);
//	debugNumOfActiveBoxesText.setPosition(sf::Vector2f(10, 50));
//
//	debugSynchronisationMethodText.setFont(debugFont);
//	debugSynchronisationMethodText.setCharacterSize(debugTextSize);
//	debugSynchronisationMethodText.setFillColor(debugTextColor);
//	debugSynchronisationMethodText.setPosition(sf::Vector2f(700, 50));
//
//	switch (networkMethod) {
//	case NetworkSynchronisationMethod::LockStep:
//		debugSynchronisationMethodText.setString("LockStep");
//		break;
//	case NetworkSynchronisationMethod::SnapShot:
//		debugSynchronisationMethodText.setString("SnapShot");
//		break;
//	case NetworkSynchronisationMethod::State:
//		debugSynchronisationMethodText.setString("State");
//		break;
//	}
//}
//
//void World::Update(sf::RenderWindow &Window)
//{
//	if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
//	{
//		sf::Vector2i mousePosition = sf::Mouse::getPosition(Window);
//		EntityBox* box = new EntityBox();
//		box->init(world, mousePosition, boxSize, scale);
//		boxes.push_back(box);
//	}
//
//	switch (networkMethod) {
//	case NetworkSynchronisationMethod::LockStep:
//		network = new NetworkLockStep();
//		break;
//	}
//
//	world->Step(1 / 60.0f, 8, 3);
//
//	player.input();
//
//	Debug();
//}
//
//void World::Render(sf::RenderWindow & Window)
//{
//	for (auto w : walls)
//		Window.draw(w->rectangle);
//
//	for (auto b : boxes)
//		b->Render(Window);
//
//	player.Render(Window);
//
//	Window.draw(debugConnectedStatusText);
//	Window.draw(debugPacketSizeText);
//	Window.draw(debugNumOfBoxesText);
//	Window.draw(debugNumOfActiveBoxesText);
//	Window.draw(debugSynchronisationMethodText);
//
//	sfgui.Display(Window);
//}
//
//void World::Debug()
//{
//	// Connected Status
//	if(network->isConnected())
//		debugConnectedStatusText.setString("Connected");
//
//	// Packet Size
//	int temp = 10;
//	debugPacketSizeText.setString("" + std::to_string(temp) + "mb/s");
//
//	// Active Boxes
//	int sleepCount = 0;
//	for (auto b : boxes)
//		if (b->IsSleep())
//			sleepCount++;
//
//	debugNumOfActiveBoxesText.setString("Active: " + std::to_string(sleepCount));
//}
//
//void World::CreateWall(float posX, float posY, sf::Vector2f size)
//{
//	// Wall
//	WorldWall* wall = new WorldWall();
//	walls.push_back(wall);
//
//	// Body
//	b2BodyDef BodyDef;
//	BodyDef.position = b2Vec2(posX / scale, posY / scale);
//	BodyDef.type = b2_staticBody;
//	wall->body = world->CreateBody(&BodyDef);
//
//	// Shape
//	b2PolygonShape Shape;
//	//Shape.SetAsBox(size.x / scale, size.y / scale);
//	Shape.SetAsBox((size.x / 2) / scale, (size.y / 2) / scale);
//	
//	//FixtureDef
//	b2FixtureDef fixtureDef;
//	fixtureDef.shape = &Shape; // Sets the shape
//	fixtureDef.density = 0.f;  // Sets the density of the body
//	wall->body->CreateFixture(&fixtureDef); // Apply the fixture definition
//
//	// Rectangle
//	wall->rectangle.setSize(size);
//	wall->rectangle.setOrigin(size.x / 2, size.y / 2);
//	wall->rectangle.setPosition(wall->body->GetPosition().x * scale, wall->body->GetPosition().y * scale);
//	wall->rectangle.setFillColor(sf::Color::Black);
//}