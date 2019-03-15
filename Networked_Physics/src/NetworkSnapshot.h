#pragma once

#include "Network.h"

struct clientInputs 
{
public:
	int sequence;
	bool none = false;
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
};

struct clientCommands {};

struct ObjectState
{
public:
	bool interacting;
	sf::Vector2f position;
	//sf::Vector2f linear_velocity;
	float angle;
};

struct SnapshotPacket
{
public:
	int sequence;
	int deltapacketindex;
	bool initial = false;
	std::vector<ObjectState*> objects;
	float time;
};

class NetworkSnapshot : public Network
{
public:
	NetworkSnapshot();
	~NetworkSnapshot();

	void DeltaCompression(SnapshotPacket* Packet);
	void SendSnapshot(SnapshotPacket Packet, int netID);
	void ClearDelta();

	void SendClientInput(clientInputs* inputs);

	void TCP_Consumer() override;
	void UDP_Consumer() override;

	std::queue<SnapshotPacket*>* getSnapshotQueue() { return &snapshotQueue; }
	std::queue<clientInputs*>* getClientInputQueue() { return &clientInputQueue; }

	void SetInitialPacket(SnapshotPacket* initialPacket) { initialSnapshotPacket = initialPacket; }
	void SetClock(sf::Clock* clock) { clockPacket = clock; }

private:
	std::queue<SnapshotPacket*> snapshotQueue;
	std::queue<clientInputs*> clientInputQueue;
	std::map<int, SnapshotPacket*> baselineSnapshotMap;
	SnapshotPacket* initialSnapshotPacket;
	sf::Clock* clockPacket{ nullptr };
};
