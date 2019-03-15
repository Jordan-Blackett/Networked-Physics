#pragma once

#include <Box2D/Box2D.h>
#include <vector>

//subclass b2QueryCallback
class MyQueryCallback : public b2QueryCallback {
public:
	std::vector<b2Body*> foundBodies;

	bool ReportFixture(b2Fixture* fixture) {
		foundBodies.push_back(fixture->GetBody());
		return true;//keep going to find all fixtures in the query area
	}
};

