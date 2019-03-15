#include "NetworkState.h"



NetworkState::NetworkState()
{
}


NetworkState::~NetworkState()
{
}

void NetworkState::DeltaCompression(StatePacket* Packet)
{
	// Delta Compression
	for (auto c : clients)
	{
		// Delta Packet
		StatePacket deltaStatePacket;
		StatePacket* baselineSnapshotPacket = new StatePacket();

		// Baseline
		deltaStatePacket.sequence = packetIndex;

		// Box data
		for (int i = 0; i < Packet->objects.size(); i++)
		{
			ObjectStateState* deltaCubeState = new ObjectStateState();
			
			// Baseline Packet
			int netID = Packet->objects[i]->indexID;

			bool delta = false;
			int packetIdx;
			int packetPosition;

			// Check if client ACK'd packet
			std::vector<statePacketIndex> v = stateBaseSequences[netID];
			if (!stateBaseSequences[netID].empty())
			{
				for (int i = 0; i < v.size(); i++) {
					int index = c.second->ackSequence - v[i].statePacketIndex1;
					std::bitset<32> bit(c.second->ackBits);
					if (index < 32 && index >= 0) // TODO: clear packet index larger then 32 from base. 
					{
						if (bit.test(index))
						{
							delta = true;
							packetIdx = v[i].statePacketIndex1;
							packetPosition = v[i].statePacketPosition1;
							//break;
						}
					}
				}
			}

			if (delta)
			{
				baselineSnapshotPacket = baselineStatePacketMap[packetIdx];

				deltaCubeState->absoluteValue = false;
				deltaCubeState->baseSequence = packetIdx;

				// Delta Values
				deltaCubeState->indexID = Packet->objects[i]->indexID;
				deltaCubeState->position = Packet->objects[i]->position - baselineSnapshotPacket->objects[packetPosition]->position;
				deltaCubeState->angle = Packet->objects[i]->angle;// -baselineSnapshotPacket->objects[packetPosition]->angle;
				deltaCubeState->linear_velocity = Packet->objects[i]->linear_velocity;
				deltaCubeState->angular_velocity = Packet->objects[i]->angular_velocity;
			}
			else
			{
				deltaCubeState->absoluteValue = true;

				// Absolute value
				deltaCubeState->indexID = Packet->objects[i]->indexID;
				deltaCubeState->position = Packet->objects[i]->position;
				deltaCubeState->angle = Packet->objects[i]->angle;
				deltaCubeState->linear_velocity = Packet->objects[i]->linear_velocity;
				deltaCubeState->angular_velocity = Packet->objects[i]->angular_velocity;
			}

			// Add packet ID and position in the packet
			statePacketIndex temp;
			temp.statePacketIndex1 = packetIndex;
			temp.statePacketPosition1 = i;
			stateBaseSequences[netID].push_back(temp);
			
			// Add cube
			deltaStatePacket.objects.push_back(deltaCubeState);
		}

		// Send Packet
		SendStatePacket(deltaStatePacket, c.first);

		// Add packet to baseline map
		baselineStatePacketMap.insert(std::pair<int, StatePacket*>(packetIndex, Packet));
	}
	packetIndex++;
}

void NetworkState::SendStatePacket(StatePacket Packet, int netID)
{
	// Create Message
	ProtobufMessage::UDPMessage* serverMessage = new ProtobufMessage::UDPMessage;
	ProtobufMessage::StateMessage* stateMessage = new ProtobufMessage::StateMessage;

	stateMessage->set_netid(netID);

	// Packet sequence index
	stateMessage->set_sequence(Packet.sequence);

	// Populate Inputs
	for (auto inputs : Packet.inputs)
	{
		ProtobufMessage::StateMessage_Input* inputState = stateMessage->add_inputs();
		inputState->set_left(inputs->left);
		inputState->set_right(inputs->right);
		inputState->set_up(inputs->up);
		inputState->set_down(inputs->down);
		inputState->set_space(inputs->space);
	}

	// Populate Object State
	for (auto object : Packet.objects)
	{
		ProtobufMessage::StateMessage_Objectstate* objectState = stateMessage->add_objectstate();
		objectState->set_index(object->indexID);
		objectState->set_absolutevalue(object->absoluteValue);
		objectState->set_basesequence(object->baseSequence);
		objectState->set_positionx(object->position.x);
		objectState->set_positiony(object->position.y);
		objectState->set_angle(object->angle);
		objectState->set_linear_velocityx(object->linear_velocity.x);
		objectState->set_linear_velocityy(object->linear_velocity.y);
		objectState->set_angular_velocity(object->angular_velocity);
	}

	// Timpstamp
	stateMessage->set_timestamp(clockPacket->getElapsedTime().asMilliseconds());

	// Send
	serverMessage->set_allocated_statemsg(stateMessage);
	UDP_Send(serverMessage);
}

void NetworkState::ClearDelta(int baseline)
{
	// Clear stored packets

	if (baselineStatePacketMap.size() > 32)
	{
		// Clear older packets then base - 32
		std::map<int, StatePacket*>::iterator iterUpper;
		iterUpper = baselineStatePacketMap.end();//baselineStatePacketMap.lower_bound(lowestBaseSequence);
		std::advance(iterUpper, -124); // Element before lowerest -33
		int lowestBaseSequence = iterUpper->first;
		baselineStatePacketMap.erase(baselineStatePacketMap.begin(), iterUpper);

		// Clear per object index
		std::map<int, int>::iterator it;
		for (int i = 0; i < clientStateBaseSequences.size(); i++)
		{
			it = clientStateBaseSequences[i].lower_bound(lowestBaseSequence); // lower_bound = >=
			// Make sure it isnt the first element as --it will return nul
			if (clientStateBaseSequences[i].begin() != it)
			{
				clientStateBaseSequences[i].erase(clientStateBaseSequences[i].begin(), --it);
			}
		}
	}


	// TODO: clear host
}

void NetworkState::SendACK()
{

}

void NetworkState::TCP_Consumer()
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
		}
	}
}

void NetworkState::UDP_Consumer()
{
	while (!udp_queue.isEmpty()) {
		auto serverMessage = udp_queue.pop();

		switch (serverMessage->msg_case())
		{
		// Server Message
		case 3:
		{
			// Snapshot
			StatePacket* statePacket = new StatePacket();
			statePacket->sequence = serverMessage->statemsg().sequence();

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
			int objectSize = serverMessage->statemsg().objectstate_size();
			for (int i = 0; i < objectSize; i++)
			{
				const ProtobufMessage::StateMessage_Objectstate objectState = serverMessage->statemsg().objectstate(i);

				// Cube
				ObjectStateState* cubeState = new ObjectStateState();

				if (objectState.absolutevalue())
				{
					cubeState->indexID = objectState.index();
					cubeState->position.x = objectState.positionx();
					cubeState->position.y = objectState.positiony();
					cubeState->angle = objectState.angle();
					cubeState->linear_velocity.x = objectState.linear_velocityx();
					cubeState->linear_velocity.y = objectState.linear_velocityy();
					cubeState->angular_velocity = objectState.angular_velocity();
				}
				else
				{
					int deltaSequence = objectState.basesequence();
					StatePacket* deltaStatePacket = baselineStatePacketMap[deltaSequence];
					
					cubeState->indexID = objectState.index();
					int baseIndex = clientStateBaseSequences[cubeState->indexID][deltaSequence];

					cubeState->position.x = deltaStatePacket->objects[baseIndex]->position.x + objectState.positionx();
					cubeState->position.y = deltaStatePacket->objects[baseIndex]->position.y + objectState.positiony();
					cubeState->angle = objectState.angle();
					cubeState->linear_velocity.x = objectState.linear_velocityx();
					cubeState->linear_velocity.y = objectState.linear_velocityy();
					cubeState->angular_velocity = objectState.angular_velocity();
				}

				clientStateBaseSequences[cubeState->indexID].insert(std::pair<int,int> (statePacket->sequence, i));

				statePacket->objects.push_back(cubeState);
			}
			statePacket->time = serverMessage->statemsg().timestamp();
			statePacket->timeReceived = stateJitterBuffer.GetClientTime();
			stateJitterBuffer.InsertPacket(statePacket);

			// Add packet to baseline map
			baselineStatePacketMap.insert(std::pair<int, StatePacket*>(serverMessage->statemsg().sequence(), statePacket));
			ClearDelta(serverMessage->statemsg().sequence());

			// Bit field
			int diffence = serverMessage->statemsg().sequence() - baseACK; // Differnce between last base packet and current
			baseACK = serverMessage->statemsg().sequence();
			bitACK >> diffence; // Bit shift depending on differnce
			bitACK.set(0, true); // Set 0 bit

			// Send ACK
			ProtobufMessage::UDPMessage* ACKMessage = new ProtobufMessage::UDPMessage;
			ProtobufMessage::StateACKPacket* ackMsg = new ProtobufMessage::StateACKPacket;
			ackMsg->set_netid(serverMessage->statemsg().netid());
			ackMsg->set_acksequence(baseACK);
			ackMsg->set_ackbits((int)(bitACK.to_ulong()));
			ACKMessage->set_allocated_stateack(ackMsg);
			UDP_Send(ACKMessage);
		}
		break;
		// Receive ACK
		case 6:
		{
			int clientID = serverMessage->stateack().netid();
			clients[clientID]->ackSequence = serverMessage->stateack().acksequence();
			clients[clientID]->ackBits = serverMessage->stateack().ackbits();
			//ClearDelta(clients[clientID]->ackSequence);
		}
		break;
		// Time Request
		case 9: 
		{
			if (serverHost)
			{
				// Set Server Time
				ProtobufMessage::ServerTimeRequest* timeRequestMessage = new ProtobufMessage::ServerTimeRequest;
				timeRequestMessage->set_clienttime(serverMessage->servertimerequestmsg().clienttime());
				//std::cout << timeRequestMessage->clienttime() << std::endl;
				timeRequestMessage->set_servertime(clockPacket->getElapsedTime().asMilliseconds());
				serverMessage->set_allocated_servertimerequestmsg(timeRequestMessage);
				// Send back
				UDP_Send(serverMessage);
			}
			else
			{
				// Set JitterBuffer Server Delta
				stateJitterBuffer.SetServerTimeDelta(serverMessage->servertimerequestmsg().clienttime(), serverMessage->servertimerequestmsg().servertime());
			}
		}
		break;

		
		}
	}
}

void NetworkState::SendTimeRequest()
{
	ProtobufMessage::UDPMessage* serverMessage = new ProtobufMessage::UDPMessage;
	ProtobufMessage::ServerTimeRequest* timeRequestMessage = new ProtobufMessage::ServerTimeRequest;
	timeRequestMessage->set_clienttime(stateJitterBuffer.GetClientTime());
	serverMessage->set_allocated_servertimerequestmsg(timeRequestMessage);
	UDP_Send(serverMessage);
}
