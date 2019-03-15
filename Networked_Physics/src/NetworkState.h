#pragma once

#include "Network.h"
#include "JitterBuffer.h"
#include <bitset>

struct InputStateData
{
public:
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	bool space = false;
	bool leftclick = false;
	bool rightclick = false;
	int frameID;
};

struct ObjectStateState
{
public:
	int indexID;
	bool absoluteValue;
	int baseSequence;
	sf::Vector2f position;
	sf::Vector2f linear_velocity;
	float angle;
	float angular_velocity;
};

struct StatePacket
{
public:
	int sequence;
	std::vector<InputStateData*> inputs;
	int numObjects;
	std::vector<ObjectStateState*> objects;
	float time;
	float timeReceived;
};

struct PriorityIndex
{
	int objectID;
	float* priority;
	PriorityIndex(int id, float* priority) : objectID(id), priority(priority) {}
};

class BitField
{
	char i;
};

struct statePacketIndex
{
	int statePacketIndex1;
	int statePacketPosition1;
};

class NetworkState : public Network
{
public:
	NetworkState();
	~NetworkState();

	void DeltaCompression(StatePacket* Packet);
	void SendStatePacket(StatePacket Packet, int netID);
	void ClearDelta(int baseline);
	void SendACK();

	void initPacketVector(int size)
	{
		for (int i = 0; i < size; i++)
		{
			std::vector<statePacketIndex> temp;
			stateBaseSequences.push_back(temp); //std::vector<int>()
			
			std::map<int, int> temp2;
			clientStateBaseSequences.push_back(temp2);
		}
	}

	void TCP_Consumer() override;
	void UDP_Consumer() override;

	JitterBuffer* getStateJitterBuffer() { return &stateJitterBuffer; }
	void SetClock(sf::Clock* clock) { clockPacket = clock; }

	void SendTimeRequest();

private:
	JitterBuffer stateJitterBuffer;
	sf::Clock* clockPacket{ nullptr };

	std::map<int, StatePacket*> baselineStatePacketMap;
	//SnapshotPacket* initialSnapshotPacket;

	std::vector<std::vector<statePacketIndex>> stateBaseSequences;
	std::vector<std::map<int, int>> clientStateBaseSequences;

	int baseACK;
	std::bitset<32> bitACK;
};