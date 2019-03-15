#include "NetworkLockStep.h"



NetworkLockStep::NetworkLockStep()
{
}


NetworkLockStep::~NetworkLockStep()
{
}

void NetworkLockStep::TCP_Consumer()
{
	if (!tcp_queue.isEmpty()) {
		auto serverMessage = tcp_queue.pop();

	}
}

void NetworkLockStep::UDP_Consumer()
{
	while (!udp_queue.isEmpty()) {
		auto serverMessage = udp_queue.pop();
		
		// LockStep Inputs
		if (serverMessage->has_lockstepmsg()) {
			int size = serverMessage->lockstepmsg().inputs_size();
			for (int i = 0; i < size; i++) {
				const ProtobufMessage::LockStepMessage_Input inputProto = serverMessage->lockstepmsg().inputs(i);
				InputData inputData;
				inputData.left = inputProto.left();
				inputData.right = inputProto.right();
				inputData.up = inputProto.up();
				inputData.down = inputProto.down();
				inputData.space = inputProto.space();

				inputs.push_back(inputData);
			}
		}
	}
}
