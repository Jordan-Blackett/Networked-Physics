#include "InputBuffer.h"



InputBuffer::InputBuffer()
{
}


InputBuffer::~InputBuffer()
{
}

void InputBuffer::InsertClient(int clientID)
{
	// Insert
	std::queue<InputStateData*> temp2;
	inputbuffer_.insert(std::pair<int, std::queue<InputStateData*>>(clientID, temp2));
}

void InputBuffer::InsertInput(InputStateData* inputState, int clientID)
{
	// Order packets
	// check old packets etc
	//convert client local frameID to serverFrameID

	// Add input
	inputbuffer_[clientID].push(inputState);
}

std::vector<InputStateData*> InputBuffer::GetInput()
{
	std::vector<InputStateData*> inputsFrame;
	for (auto i : inputbuffer_)
	{
		if (!i.second.empty()) //i.second.size() > 4
		{
			inputsFrame.push_back(i.second.front());
			//std::cout << "Size " << i.second.size() << std::endl;
			//i.second.pop();
			inputbuffer_[i.first].pop();
		}
		else {
			// dummyInput
			inputsFrame.push_back(new InputStateData());
		}
	}

	return inputsFrame;
}

void InputBuffer::CheckThrottle()
{
	//send slow down
}
