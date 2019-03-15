#pragma once

#include <SFML\Graphics.hpp>
#include <Box2D\Box2D.h>

class Entity
{
public:
	Entity();
	~Entity();

	void InitDynamicBody(b2World* world);

	void addRectangularFixtureToBody(sf::Vector2f size);
	void addCircularFixtureToBody(float radius);
	void createFixture(b2Shape * shape);

	bool IsSleep() { return body_->IsAwake(); }
	void SetAsleep() { body_->SetAwake(false); }

	virtual void Update();
	virtual void Render(sf::RenderWindow &Window);

	b2Body* GetBody() { return body_; }

	void SetInteracting(bool active) { networkEntityActive = active; }

	float GetStatePriority() { return statePriority; }
	float* GetStatePriorityPointer() { return &statePriority; }
	void SetStatePriority(int priority) { statePriority = priority; }
	void UpdateStatePriority(int priority) { statePriority += priority; }

	sf::Vector2f GetStateErrorPosition() { return stateErrorPosition_; }
	float GetStateErrorAngle() { return stateErrorAngle_; }
	void SetStateErrorPosition(sf::Vector2f errorPos){ stateErrorPosition_ = errorPos; }
	void SetStateErrorAngle(float errorAngle) { stateErrorAngle_ = errorAngle; }

	void SetFitter();

protected:
	// Physics
	b2Body* body_;
	float scale_;

	// Settings
	sf::Vector2i position_;

	sf::Vector2f stateErrorPosition_;
	float stateErrorAngle_;

	bool entityActive{ false };
	bool networkEntityActive{ false };
	sf::Color activeColour_{ sf::Color::Red };
	sf::Color sleepColour_{ sf::Color(175, 175, 175) }; // Gray

	float priorityAccumulator{ 1 };
	float statePriority{ 0 };

	bool networkPlayerFix = false;
};

