#include "Entity.h"



Entity::Entity()
{
}


Entity::~Entity()
{
}

void Entity::InitDynamicBody(b2World* world)
{
	// Add a dynamic body to world
	b2BodyDef BodyDef;
	BodyDef.type = b2_dynamicBody;
	BodyDef.position = b2Vec2(position_.x / scale_, position_.y / scale_);
	body_ = world->CreateBody(&BodyDef);
}

void Entity::addRectangularFixtureToBody(sf::Vector2f size)
{
	b2PolygonShape shape;
	shape.SetAsBox((size.x / 2) / scale_, (size.y / 2) / scale_);
	createFixture(&shape);
}

void Entity::addCircularFixtureToBody(float radius)
{
	b2CircleShape shape;
	shape.m_radius = radius * scale_;
	createFixture(&shape);
}

//enum
//{
//	kFilterCategoryLevel = 0x01,
//	kFilterCategorySolidObject = 0x02,
//	kFilterCategoryNonSolidObject = 0x04
//};

void Entity::createFixture(b2Shape* shape)
{
	b2FixtureDef fixtureDef;
	fixtureDef.shape = shape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.7f;

	if (networkPlayerFix)
	{
		short CATEGORY_NETWORKPLAYER = 0x0002;
		//fixtureDef.restitution = 0.1f;
		fixtureDef.filter.categoryBits = CATEGORY_NETWORKPLAYER; // Category of the body
		fixtureDef.filter.maskBits = 0x0000; // Category that this body can collide with
	}
	else
	{
		short CATEGORY_NETWORKPLAYER1 = 0x0001;
		fixtureDef.filter.categoryBits = CATEGORY_NETWORKPLAYER1; // Category of the body
		fixtureDef.filter.maskBits = 0xffff; // Category that this body can collide with
	}

	body_->CreateFixture(&fixtureDef);
}

void Entity::SetFitter()
{
}

void Entity::Update()
{
}

void Entity::Render(sf::RenderWindow &Window)
{
}