#include "NetworkPredictionAndReconciliation.h"

#include "NetworkState.h"

NetworkPredictionAndReconciliation::NetworkPredictionAndReconciliation()
{
}


NetworkPredictionAndReconciliation::~NetworkPredictionAndReconciliation()
{
}

void NetworkPredictionAndReconciliation::SendStatePRPacket(StatePRPacket* Packet)
{
	int temp = 0;
	for (auto c : clients)
	{
		// Create Message
		ProtobufMessage::UDPMessage* serverMessage = new ProtobufMessage::UDPMessage;
		ProtobufMessage::PredictionReconciliationMessage* stateMessage = new ProtobufMessage::PredictionReconciliationMessage;

		// Packet sequence index
		stateMessage->set_netid(c.first);
		stateMessage->set_sequence(Packet->inputs[temp]->frameID);
		//std::cout << "TEST 3 " << stateMessage->sequence() << std::endl;
		//temp++;

		// Populate Inputs
		for (auto inputs : Packet->inputs)
		{
			ProtobufMessage::PredictionReconciliationMessage_Input* inputState = stateMessage->add_inputs();
			inputState->set_left(inputs->left);
			inputState->set_right(inputs->right);
			inputState->set_up(inputs->up);
			inputState->set_down(inputs->down);
			inputState->set_space(inputs->space);
		}

		// Populate Object State
		for (auto object : Packet->objects)
		{
			ProtobufMessage::PredictionReconciliationMessage_Objectstate* objectState = stateMessage->add_objectstate();
			objectState->set_positionx(object->position.x);
			objectState->set_positiony(object->position.y);
			objectState->set_angle(object->angle);
			objectState->set_linear_velocityx(object->linear_velocity.x);
			objectState->set_linear_velocityy(object->linear_velocity.y);
			objectState->set_angular_velocity(object->angular_velocity);
		}

		// Send
		serverMessage->set_allocated_predictionreconciliationmsg(stateMessage);
		UDP_Send(serverMessage);
	}
}

void NetworkPredictionAndReconciliation::SendInputPRPacket(InputStateData* Packet, int frameIndex)
{
	// Create Message
	ProtobufMessage::UDPMessage* serverMessage = new ProtobufMessage::UDPMessage;
	ProtobufMessage::PredictionReconciliationInputMessage* inputMessage = new ProtobufMessage::PredictionReconciliationInputMessage;

	// Packet sequence index
	inputMessage->set_sequence(frameIndex);

	// Populate Input + TODO: prev inputs
	ProtobufMessage::PredictionReconciliationInputMessage_Input* inputState = inputMessage->add_inputs();
	inputState->set_left(Packet->left);
	inputState->set_right(Packet->right);
	inputState->set_up(Packet->up);
	inputState->set_down(Packet->down);
	inputState->set_space(Packet->space);

	// Send
	serverMessage->set_allocated_predictionreconciliationinputmsg(inputMessage);
	UDP_Send(serverMessage);
}

void NetworkPredictionAndReconciliation::TCP_Consumer()
{
	if (!tcp_queue.isEmpty())
	{
		auto serverMessage = tcp_queue.pop();

		// Split tcp message
		std::vector<std::string> splitMessage = splitServerMessage(serverMessage->msg(), "::");

		// New client
		if (splitMessage[0] == "newclient")
		{
			std::cout << "[ New player joined ]" << std::endl;
			clients.insert(std::make_pair(std::stoi(splitMessage[1]), new client()));
			inputbuffer.InsertClient(std::stoi(splitMessage[1]));
		}
	}
}

void NetworkPredictionAndReconciliation::UDP_Consumer()
{
	while (!udp_queue.isEmpty()) {
		auto serverMessage = udp_queue.pop();

		switch (serverMessage->msg_case())
		{

		case 7:
		{
			// State
			StatePRPacket* statePacket = new StatePRPacket();
			statePacket->sequence = serverMessage->predictionreconciliationmsg().sequence();

			// Inputs
			int inputsSize = serverMessage->statemsg().inputs_size();
			for (int i = 0; i < inputsSize; i++)
			{
				const ProtobufMessage::StateMessage_Input inputStatePacket = serverMessage->statemsg().inputs(i);

				// Cube
				InputStateData* inputState = new InputStateData();

				inputState->left = inputStatePacket.left();
				inputState->right = inputStatePacket.right();
				inputState->up = inputStatePacket.up();
				inputState->down = inputStatePacket.down();
				inputState->space = inputStatePacket.space();

				statePacket->inputs.push_back(inputState);
			}

			// Object Data
			int objectSize = serverMessage->predictionreconciliationmsg().objectstate_size();
			for (int i = 0; i < objectSize; i++)
			{
				const ProtobufMessage::PredictionReconciliationMessage_Objectstate objectState = serverMessage->predictionreconciliationmsg().objectstate(i);

				// Cube
				ObjectStatePRState* cubeState = new ObjectStatePRState();

				cubeState->position.x = objectState.positionx();
				cubeState->position.y = objectState.positiony();
				cubeState->angle = objectState.angle();
				cubeState->linear_velocity.x = objectState.linear_velocityx();
				cubeState->linear_velocity.y = objectState.linear_velocityy();
				cubeState->angular_velocity = objectState.angular_velocity();
				
				statePacket->objects.push_back(cubeState);
			}

			statePRPackets.push(statePacket);
		}
		break;
		case 8:
		{
			// Client ID
			int clientID = serverMessage->predictionreconciliationinputmsg().netid();

			// Client Input
			const ProtobufMessage::PredictionReconciliationInputMessage_Input inputStatePacket = serverMessage->predictionreconciliationinputmsg().inputs(0);

			// Cube
			InputStateData* inputState = new InputStateData();

			inputState->left = inputStatePacket.left();
			inputState->right = inputStatePacket.right();
			inputState->up = inputStatePacket.up();
			inputState->down = inputStatePacket.down();
			inputState->space = inputStatePacket.space();
			inputState->frameID = serverMessage->predictionreconciliationinputmsg().sequence();
			//std::cout << "TEST 1" << serverMessage->predictionreconciliationinputmsg().sequence() << std::endl;

			inputbuffer.InsertInput(inputState, clientID);
		}
		break;
		}
	}
}
