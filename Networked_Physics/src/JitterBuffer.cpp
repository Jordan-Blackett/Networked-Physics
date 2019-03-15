#include "JitterBuffer.h"
#include "NetworkState.h"


JitterBuffer::JitterBuffer()
{
}


JitterBuffer::~JitterBuffer()
{
}

void JitterBuffer::init(float delayBufferTime)
{
	delayBufferTime_ = delayBufferTime;
	clock_.restart();
}

void JitterBuffer::InsertPacket(StatePacket* statePacket)
{
	//double newTime = simulationClock_.getElapsedTime().asSeconds();
	//double dt = newTime - bufferTimeLastUpdated_;
	//simulationTimeLastUpdated_ = newTime;

	// First Packet
	if (!firstPacket)
	{
		firstPacket = true;
		localTimeDifference = (statePacket->time - clock_.getElapsedTime().asMilliseconds());
	}

	// Old packet - ignore
	if (statePacket->sequence <= lastestUsedSeq)
	{
		std::cout << statePacket->sequence << " - IGNORED " << std::endl;
	}
	else
	{
		// Add to the delay examples
		float ri = statePacket->timeReceived;// + ServerTimeDelta;
		float ti = statePacket->time - ServerTimeDelta; // Server timestamp
		delayRiTi.push_back(ri - ti);
		//std::cout << "Lac" << ri - ti << std::endl;
		if (delayRiTi.size() > delayExampleSize)
			delayRiTi.pop_front();

		// Playout Time - Fixed
		float playoutTime = AaptivePlayOutDelay(ri, ti);
		// Convert it back to client time
		//std::cout << "Delay" << playoutTime << std::endl;
		
		// Fixed Playout
		//float playoutTime = (statePacket->time - ServerTimeDelta) + delayBufferTime_;

		// Before playout time
		float temp = clock_.getElapsedTime().asMilliseconds();
		if (playoutTime > clock_.getElapsedTime().asMilliseconds())
		{
			statePacket->time = playoutTime;

			if (!statePackets_.empty())
			{
				// Check if packet is out of order
				if (statePacket->sequence < statePackets_.back()->sequence)
				{
					// Find the packets correct place - (-1 = index -1 = Already checked)
					for (int index = statePackets_.size()-1; index < 0; index--)
					{
						if (statePacket->sequence > statePackets_[index]->sequence)
						{
							//std::cout << statePacket->sequence << " - INSERTED " << std::endl;
							// Insert out of order packet
							std::deque<StatePacket*>::iterator it = statePackets_.begin();
							statePackets_.insert(it + index, statePacket);
							break;
						}
					}
				}
				else
				{
					//std::cout << statePacket->sequence << " - PUSHED " << std::endl;
					statePackets_.push_back(statePacket);
				}
			}
			else
			{
				//std::cout << statePacket->sequence << " - PUSHED " << std::endl;
				statePackets_.push_back(statePacket);
			}
		}
		else {
			std::cout << statePacket->sequence << " - MISSED PLAYOUT TIME " << std::endl;
		}
	}
}

StatePacket* JitterBuffer::RetrievePacket()
{
	// Add to the delay examples
	//float ri = statePackets_.front()->timeReceived;
	//float ti = statePackets_.front()->time;
	//delayRiTi.push_back(ri - ti);
	//if (delayRiTi.size() <= delayExampleSize)
	//	delayRiTi.pop_front();

	StatePacket* statePacket = statePackets_.front();
	statePackets_.pop_front();
	lastestUsedSeq = statePacket->sequence;

	return statePacket;
}

bool JitterBuffer::time()
{
	if (!statePackets_.empty())
	{
		//float playoutTime = AaptivePlayOutDelay(statePackets_.front()->time, statePackets_.front()->timeReceived);
		// Currect time > Playout time 
		if (clock_.getElapsedTime().asMilliseconds() >= statePackets_.front()->time) // statePackets_.front()->time
			return true;
		else
			return false;
	}
	return false;
}

float JitterBuffer::AaptivePlayOutDelay(float ri, float ti)
{
	float di; // the estimate of the average network delay after receiving the i’th packet
	float u = 0.1; // Fixed constant
	//float ti; // the timestamp for the i’th packet
	//float ri; // the time that packet ‘i’ is received by the receiver	

	// Last 15 packets delays
	float sum = 0;
	for (int i = 0; i < delayRiTi.size(); ++i) {
		sum += delayRiTi[i];
	}
	
	// Average
	di = sum / delayRiTi.size();

	// Average network delay - 
	//estimate of the average network delay upon reception of the ith packet
	di = (1 - u)*di-1 + u*(ri - ti);

	// Average deviation of delay
	//denote an estimate of the average deviation of the delay from the estimated average delay
	float vi1 = vi;
	vi = (1 - u)*vi1 - 1 + u * std::abs(ri - ti - di);
	
	int k = 4; //4 // Positive constant
	float pi = ti + di + (k * std::abs(vi)); //(timestamp + est. delay + scaled deviation) ti +
	//std::cout << "delay =" << pi - clock_.getElapsedTime().asMilliseconds() << std::endl;

	return pi;
}

void JitterBuffer::SetServerTimeDelta(float clientTime, float serverTime)
{
	float currentTime = clock_.getElapsedTime().asMilliseconds();
	float latencyDelta = (currentTime - clientTime) / 2; //(clientTime - currentTime) / 2;
	float serverclientTimeDelta = serverTime - currentTime;
	ServerTimeDelta = serverclientTimeDelta + latencyDelta; // +5
}