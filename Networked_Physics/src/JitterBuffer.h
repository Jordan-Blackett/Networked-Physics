#pragma once

#include <SFML\Graphics.hpp>
#include <queue>

struct StatePacket;

class JitterBuffer
{
public:
	JitterBuffer();
	~JitterBuffer();

	void init(float delayBufferTime);
	void InsertPacket(StatePacket* statePacket);
	StatePacket* RetrievePacket();

	bool time();

	float getDelta() { 
		float currentTime = clock_.getElapsedTime().asMilliseconds();
		float delta = (currentTime - lastUpdatedDelta_);
		lastUpdatedDelta_ = currentTime;
		return delta; }

	int getSize() { return statePackets_.size(); };


	float AaptivePlayOutDelay(float ri, float ti);
	void SetServerTimeDelta(float clientTime, float serverTime);

	float GetClientTime() {
		return clock_.getElapsedTime().asMilliseconds();
	}

private:
	std::deque<StatePacket*> statePackets_;

	float delayBufferTime_;
	
	bool firstPacket{ false };
	int lastestUsedSeq;

	sf::Clock clock_;
	float lastUpdatedDelta_;
	float localTimeDifference;

	std::deque<float> delayRiTi;
	int delayExampleSize = 15;

	float ServerTimeDelta;
	float vi = 0;
};

