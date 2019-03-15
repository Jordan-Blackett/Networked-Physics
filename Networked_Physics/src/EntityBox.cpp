#include "EntityBox.h"

#include <iostream>

EntityBox::EntityBox()
{
}


EntityBox::~EntityBox()
{
}

void EntityBox::init(b2World* world, sf::Vector2i position, sf::Vector2f size, int scale)
{
	scale_ = scale;
	position_ = position;
	rectSize_ = size;

	// Rectangle
	InitRectangle();

	// Physics
	InitDynamicBody(world);
	addRectangularFixtureToBody(rectSize_);

	// No Gravity - Friction
	body_->SetLinearDamping(0.4f);
	body_->SetAngularDamping(0.4f);
}


void EntityBox::InitRectangle()
{
	rectangle_.setSize(rectSize_);
	rectangle_.setOrigin(rectSize_.x / 2, rectSize_.y / 2);
	rectangle_.setPosition(position_.x, position_.y);
	rectangle_.setFillColor(activeColour_);
}

void EntityBox::Render(sf::RenderWindow &Window)
{
	rectangle_.setPosition(body_->GetPosition().x * scale_, body_->GetPosition().y * scale_);
	rectangle_.setRotation(body_->GetAngle() * 180 / b2_pi);

	entityActive = body_->IsAwake();

	if (entityActive || networkEntityActive)
		rectangle_.setFillColor(activeColour_);
	else
		rectangle_.setFillColor(sleepColour_);

	Window.draw(rectangle_);

	// Velocity/Friction cut off point
	float cutOff = 0.03f;
	if ((body_->GetLinearVelocity().x < cutOff && body_->GetLinearVelocity().x > -cutOff)
		|| (body_->GetLinearVelocity().y < cutOff && body_->GetLinearVelocity().y > -cutOff))
	{
		body_->SetLinearVelocity(b2Vec2(0,0));
		body_->SetAngularVelocity(0);
	}
}