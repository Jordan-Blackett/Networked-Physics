#pragma once

#include "Network.h"
#include "InputBuffer.h"

struct InputStateData;

struct ObjectStatePRState
{
public:
	sf::Vector2f position;
	sf::Vector2f linear_velocity;
	float angle;
	float angular_velocity;
};

struct StatePRPacket
{
public:
	int sequence;
	std::vector<InputStateData*> inputs;
	int numObjects;
	std::vector<ObjectStatePRState*> objects;
	float time;
};

struct FrameData
{
public:
	int frameNum;
	InputStateData* input;
	std::vector<ObjectStatePRState*> objects;
};

class NetworkPredictionAndReconciliation : public Network
{
public:
	NetworkPredictionAndReconciliation();
	~NetworkPredictionAndReconciliation();

	void SendStatePRPacket(StatePRPacket* Packet);
	void SendInputPRPacket(InputStateData* Packet, int frameIndex);

	void TCP_Consumer() override;
	void UDP_Consumer() override;

	std::vector<InputStateData*> GetInputBufferData() { return inputbuffer.GetInput(); }

	std::queue<StatePRPacket*>* GetStatePRQueue() { return &statePRPackets; }
private:
	std::queue<StatePRPacket*> statePRPackets;
	InputBuffer inputbuffer;
};

