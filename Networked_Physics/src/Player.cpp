#include "Player.h"
#include "NetworkState.h"
#include "NetworkSnapshot.h"


Player::Player()
{
}


Player::~Player()
{
}

void Player::init(b2World * world, sf::Vector2i position, sf::Vector2f size, int scale)
{
	scale_ = scale;
	position_ = position;
	rectSize_ = size;
	activeColour_ = sf::Color::Magenta;

	// Rectangle
	InitRectangle();

	// Physics
	InitDynamicBody(world);
	addRectangularFixtureToBody(rectSize_);
}

clientInputs* Player::SnapshotInput()
{
	enum _moveState {
		MS_STOP,
		MS_UP,
		MS_DOWN,
		MS_LEFT,
		MS_RIGHT,
	};

	enum _inputState {
		IS_NONE,
		IS_LEFTCLICK,
		IS_RIGHTCLICK,
	};

	_moveState moveState;
	moveState = MS_STOP;

	_inputState inputState;
	inputState = IS_NONE;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		moveState = MS_UP;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		moveState = MS_DOWN;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		moveState = MS_LEFT;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		moveState = MS_RIGHT;
	}

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
	{
		inputState = IS_LEFTCLICK;
	}
	else if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
	{
		inputState = IS_RIGHTCLICK;
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

	// Input Packet
	clientInputs* playerInput = new clientInputs();

	switch (moveState)
	{
	case MS_UP: playerInput->up = true; break;
	case MS_DOWN: playerInput->down = true; break;
	case MS_LEFT: playerInput->left = true; break;
	case MS_RIGHT: playerInput->right = true; break;
	case MS_STOP: playerInput->none = true; break;
	}

	return playerInput;
}

InputStateData* Player::StateInput()
{
	enum _moveState {
		MS_STOP,
		MS_UP,
		MS_DOWN,
		MS_LEFT,
		MS_RIGHT,
	};

	enum _inputState {
		IS_NONE,
		IS_LEFTCLICK,
		IS_RIGHTCLICK,
	};

	_moveState moveState;
	moveState = MS_STOP;

	_inputState inputState;
	inputState = IS_NONE;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		moveState = MS_UP;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		moveState = MS_DOWN;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		moveState = MS_LEFT;
	} 
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		moveState = MS_RIGHT;
	}

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
	{
		inputState = IS_LEFTCLICK;
	}
	else if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
	{
		inputState = IS_RIGHTCLICK;
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

	// Input Packet
	InputStateData* playerInput = new InputStateData();

	switch (moveState)
	{
		case MS_UP: playerInput->up = true; break;
		case MS_DOWN: playerInput->down = true; break;
		case MS_LEFT: playerInput->left = true; break;
		case MS_RIGHT: playerInput->right = true; break;
	}

	//switch (inputState)
	//{
	//	case IS_LEFTCLICK: playerInput->up = true; break;
	//	case IS_RIGHTCLICK: playerInput->down = true; break;
	//}

	return playerInput;
}

void Player::NetworkInput(InputStateData & packetInput)
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

	if (packetInput.up)
	{
		moveState = MS_UP;
	}
	else if (packetInput.down)
	{
		moveState = MS_DOWN;
	}
	else if (packetInput.left)
	{
		moveState = MS_LEFT;
	}
	else if (packetInput.right)
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
