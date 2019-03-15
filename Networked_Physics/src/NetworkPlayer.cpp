#include "NetworkPlayer.h"

#include "NetworkSnapshot.h"
#include "NetworkState.h"

NetworkPlayer::NetworkPlayer()
{
}


NetworkPlayer::~NetworkPlayer()
{
}

void NetworkPlayer::init(b2World * world, sf::Vector2i position, sf::Vector2f size, int scale)
{
	scale_ = scale;
	position_ = position;
	rectSize_ = size;
	activeColour_ = sf::Color::Blue;

	// Rectangle
	InitRectangle();
	
	// Physics
	InitDynamicBody(world);
	addRectangularFixtureToBody(rectSize_);
}

void NetworkPlayer::Input(clientInputs* input)
{
	enum _moveState {
		MS_STOP,
		MS_UP,
		MS_DOWN,
		MS_LEFT,
		MS_RIGHT,
	};

	_moveState moveState;
	moveState = MS_STOP;

	if (input->up)
	{
		moveState = MS_UP;
	}
	else if (input->down)
	{
		moveState = MS_DOWN;
	}
	else if (input->left)
	{
		moveState = MS_LEFT;
	}
	else if (input->right)
	{
		moveState = MS_RIGHT;
	}

	b2Vec2 vel = body_->GetLinearVelocity();
	b2Vec2 desiredVel = b2Vec2(0, 0);

	switch (moveState)
	{
	case MS_UP: desiredVel.y = -speed_; break;
	case MS_DOWN: desiredVel.y = speed_; break;
	case MS_LEFT: desiredVel.x = -speed_; break;
	case MS_RIGHT: desiredVel.x = speed_; break;
	case MS_STOP: desiredVel = b2Vec2(0, 0); break;
	}

	b2Vec2 velChange = b2Vec2(desiredVel.x - vel.x, desiredVel.y - vel.y);

	b2Vec2 impulse = b2Vec2(body_->GetMass() * velChange.x, body_->GetMass() * velChange.y); //disregard time factor
	body_->ApplyLinearImpulse(b2Vec2(impulse.x, impulse.y), body_->GetWorldCenter(), true);
}

void NetworkPlayer::StateInput(InputStateData * input)
{
	enum _moveState {
		MS_STOP,
		MS_UP,
		MS_DOWN,
		MS_LEFT,
		MS_RIGHT,
	};

	_moveState moveState;
	moveState = MS_STOP;

	if (input->up)
	{
		moveState = MS_UP;
	}
	else if (input->down)
	{
		moveState = MS_DOWN;
	}
	else if (input->left)
	{
		moveState = MS_LEFT;
	}
	else if (input->right)
	{
		moveState = MS_RIGHT;
	}

	b2Vec2 vel = body_->GetLinearVelocity();
	b2Vec2 desiredVel = b2Vec2(0, 0);

	switch (moveState)
	{
	case MS_UP: desiredVel.y = -speed_; break;
	case MS_DOWN: desiredVel.y = speed_; break;
	case MS_LEFT: desiredVel.x = -speed_; break;
	case MS_RIGHT: desiredVel.x = speed_; break;
	case MS_STOP: desiredVel = b2Vec2(0, 0); break;
	}

	b2Vec2 velChange = b2Vec2(desiredVel.x - vel.x, desiredVel.y - vel.y);

	b2Vec2 impulse = b2Vec2(body_->GetMass() * velChange.x, body_->GetMass() * velChange.y); //disregard time factor
	body_->ApplyLinearImpulse(b2Vec2(impulse.x, impulse.y), body_->GetWorldCenter(), true);
}

void NetworkPlayer::Interp(sf::Time dt)
{
	float tickRate = 10; //16

	float timePast = dt.asMilliseconds() - lastTimeUpdated;
	float s = timePast / tickRate;

	position_.x = lerp(position_.y, interp_.y, s);
	position_.y = lerp(position_.y, interp_.y, s);

	lastTimeUpdated = dt.asMilliseconds();
}

float NetworkPlayer::lerp(float a, float b, float s)
{
	return (a * (1.0 - s)) + (b * s);
}
