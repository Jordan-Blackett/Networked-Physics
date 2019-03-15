#pragma once

#include "Entity.h"

class EntityBox : public Entity
{
public:
	EntityBox();
	~EntityBox();

	virtual void init(b2World* world, sf::Vector2i position, sf::Vector2f size, int scale);
	void InitRectangle();

	void Render(sf::RenderWindow &Window) override;

protected:
	sf::Vector2f rectSize_;

private:
	sf::RectangleShape rectangle_;
};

