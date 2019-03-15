#pragma once

#include "Network.h"

struct InputData
{
public:
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	bool space = false;
};

class NetworkLockStep : public Network
{
public:
	NetworkLockStep();
	~NetworkLockStep();

	void TCP_Consumer() override;
	void UDP_Consumer() override;

private:
	std::vector<InputData> inputs;
};

