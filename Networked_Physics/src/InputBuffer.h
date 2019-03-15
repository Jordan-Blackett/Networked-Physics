#pragma once

#include <map>
#include <vector>

#include "NetworkState.h"

struct InputStateData;

struct frameInput
{
	int frameNum;
	InputStateData input;
};

class InputBuffer
{
public:
	InputBuffer();
	~InputBuffer();

	void InsertClient(int clientID);
	void InsertInput(InputStateData* inputState, int clientID);

	std::vector<InputStateData*> GetInput();
	void CheckThrottle();

private:
	std::map<int, std::queue<InputStateData*>> inputbuffer_;
	int currentFrame = 0;
};

