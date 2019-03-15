#pragma once

#include "EntityBox.h"

struct clientInputs;
struct InputStateData;

class NetworkPlayer : public EntityBox
{
public:
	NetworkPlayer();
	~NetworkPlayer();

	void init(b2World* world, sf::Vector2i position, sf::Vector2f size, int scale) override;

	void Input(clientInputs* input);
	void StateInput(InputStateData* input);

	void Interp(sf::Time dt);
	void SetInterpValue(sf::Vector2f value) { interp_ = value; }
	float NetworkPlayer::lerp(float a, float b, float s);

	void SetFix(bool f) { networkPlayerFix = f; }

private:
	float speed_ = 4.0f;
	sf::Vector2f interp_;

	float lastTimeUpdated;
};

