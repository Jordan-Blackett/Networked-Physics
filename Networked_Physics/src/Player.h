#pragma once

#include "EntityBox.h"

struct clientInputs;
struct InputStateData;

class Player : public EntityBox
{
public:
	Player();
	~Player();

	void init(b2World* world, sf::Vector2i position, sf::Vector2f size, int scale) override;

	clientInputs* SnapshotInput();
	InputStateData* StateInput();
	void NetworkInput(InputStateData& packetInput);

private:
	float speed_ = 4.0f;

};

