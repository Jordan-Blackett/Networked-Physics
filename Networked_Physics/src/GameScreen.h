#pragma once

#include "Screen.h"

#include <Box2D\Box2D.h>

#include "Player.h"
#include "NetworkPlayer.h"
#include "EntityBox.h"

#include "DebugGraph.h"
#include "MyQueryCallBack.h"

#include <algorithm>

struct WorldWall
{
	sf::RectangleShape rectangle;
	b2Body* body;
};

class GameScreen : public Screen
{
public:
	GameScreen();
	~GameScreen();

	void LoadContent(Network* network, sf::Clock* clock);
	void UnloadContent();
	void Update(sf::RenderWindow &Window);
	void UIUpdate(sf::RenderWindow &Window);
	void Render(sf::RenderWindow &Window);

	void InitPhysicalWorld();
	void InitDebugText();

	void CreateWall(float posX, float posY, sf::Vector2f size);

	void GameScreen::FindAllCubesAndApplyBlastImpulse(b2Vec2 center, float blastRadius, float blastPower);
	void ApplyBlastImpulse(b2Body* body, b2Vec2 blastCenter, b2Vec2 applyPoint, float blastPower);

	SnapshotPacket* initialStatePacket();

	float lerp(float a, float b, float s);
	float hermiteLerp(float t, float pos1, float pos2, float vel1, float vel2);
	float slerp(float v1, float v2, float alpha);

	FrameData* CaptureFrame();
	bool CompareEntitiesPositions(b2Vec2 pos1, b2Vec2 pos2, float threshold);

	void GameScreen::SendState(std::vector<InputStateData*> packetInputs);

	void GameScreen::Reconciliation();

	float VisualSmoothing(float offsetValue);

private:
	// Physics
	b2World* world;
	const float scale = 32.f; // Convert between pixel and real-world coordinates
	const float gravity = 0.0f; // 9.8; // -scale / 0.7f;

	//Entities
	// Player
	Player player;
	sf::Vector2f playerSize{ sf::Vector2f(40, 40) };

	// Network Players
	std::vector<NetworkPlayer*> networkPlayers;
	sf::Vector2f networkPlayersSize{ sf::Vector2f(40, 40) };

	// Boxes
	sf::Vector2f boxSize{ sf::Vector2f(15, 15) };
	std::vector<EntityBox*> boxes;
	std::vector<WorldWall*> walls;

	// Grid
	int rows = 51; // 20 15 50 = Testing 39 29 25 = Release // 32 24 30 // 51 38 19 = 1938
	int columns = 38;
	int gridOffset = 19;

	// Debug Text
	sf::Text debugConnectedStatusText;
	sf::Text debugSynchronisationMethodText;
	sf::Text debugPacketSizeText;
	sf::Text debugNumOfBoxesText;
	sf::Text debugNumOfActiveBoxesText;
	sf::Text debugHostText;

	bool host = false;
	NetworkSynchronisationMethod networkMethod{ NetworkSynchronisationMethod::LockStep };

	sf::Font debugFont;
	int debugTextSize = 22; //22
	sf::Color debugTextColor{ sf::Color::Black };
	
	// Debug Graph's
	DebugGraph debugDataInGraph;
	DebugGraph debugDataOutGraph;

	// Physics tick rate - Timestep
	sf::Clock simulationClock_;
	float dt = 1.0f / 60.0f;
	float accumulator;

	double simulationTimeLastUpdated_;
	sf::Time simulationTicktime_;

	// Lockstep

	// Snapshot
	std::queue<SnapshotPacket*>* snapshotQueue{ nullptr };

	// Input Prediction + reconcilation
	int inputSequence = 1;
	clientInputs* playerInputSnapshot = new clientInputs();
	std::vector<clientInputs*> pendingInputs;
	std::queue<clientInputs*>* ClientPendingInputsQueue;

	// Buffer
	sf::Clock packetClock;
	sf::Clock packetClockBuffer;
	SnapshotPacket* last_snapshotState = new SnapshotPacket();
	float lastBufferUpdateTime;
	float d; // buffer time // delayed
	bool initialized = false;

	// State
	std::queue<StatePacket*>* stateQueue{ nullptr };
	std::vector<PriorityIndex> statePriority;
	sf::Time stateTimeLastUpdated_;
	sf::Time stateJitterBufferTicktime_;
	sf::Clock stateJitterBufferClock_;
	int stateSequence;

	InputStateData* playerInput = new InputStateData();

	JitterBuffer* stateJitterBuffer{ nullptr };

	// 
	std::map<int, FrameData*> frameData;
	int frameNum = 0;
	std::queue<StatePRPacket*>* statePRQueue{ nullptr };


	std::vector<InputStateData*> inputstemp;

	float timeRequestAccumulator = 10;
	float timeRequestDT = 5;

	// Visual Smoothing
	b2Vec2 positionError;
	float angleError;
	float smoothingValueSmall = 0.95f;
	float smoothingValueLarge = 0.75f;
	bool desync = false;
	int sequenceVS;

	bool wasButtonPressed;
};