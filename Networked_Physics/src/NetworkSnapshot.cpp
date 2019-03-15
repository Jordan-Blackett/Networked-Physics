#include "NetworkSnapshot.h"



NetworkSnapshot::NetworkSnapshot()
{
}


NetworkSnapshot::~NetworkSnapshot()
{
}

void NetworkSnapshot::DeltaCompression(SnapshotPacket* Packet)
{
	// Delta Compression
	for(auto c : clients)
	{
		// Delta Packet
		SnapshotPacket deltaSnapshotPacket;
		SnapshotPacket* baselineSnapshotPacket = new SnapshotPacket(); // TODO: not this
		
		if(!baselineSnapshotMap.empty())
			 baselineSnapshotPacket = baselineSnapshotMap[c.second->baseSequence];

		// Baseline and current sequence
		deltaSnapshotPacket.sequence = packetIndex;
		deltaSnapshotPacket.deltapacketindex = c.second->baseSequence;

		// Delta from initial state
		if (c.second->baseSequence == 0)
		{
			deltaSnapshotPacket.initial = true;
			baselineSnapshotPacket = initialSnapshotPacket;
		}			

		// Box data
		for (int i = 0; i < Packet->objects.size(); i++)
		{
			ObjectState* deltaCubeState = new ObjectState();

			deltaCubeState->interacting = Packet->objects[i]->interacting;
			deltaCubeState->position = Packet->objects[i]->position - baselineSnapshotPacket->objects[i]->position;
			deltaCubeState->angle = Packet->objects[i]->angle - baselineSnapshotPacket->objects[i]->angle;

			deltaSnapshotPacket.objects.push_back(deltaCubeState);
		}

		// Send Packet
		SendSnapshot(deltaSnapshotPacket, c.first);

		// Add packet to baseline map
		baselineSnapshotMap.insert(std::pair<int, SnapshotPacket*>(packetIndex, Packet));
	}
	packetIndex++;
}

void NetworkSnapshot::SendSnapshot(SnapshotPacket Packet, int netID)
{
	// Create Message
	ProtobufMessage::UDPMessage* serverMessage = new ProtobufMessage::UDPMessage;
	ProtobufMessage::SnapShotMessage* snapshotMessage = new ProtobufMessage::SnapShotMessage;

	// Client ID
	snapshotMessage->set_netid(netID);

	// Packet sequence index
	snapshotMessage->set_sequence(Packet.sequence);

	// Delta baseline index
	snapshotMessage->set_deltapacketindex(Packet.deltapacketindex);
	snapshotMessage->set_initial(Packet.initial);

	// Populate Object State
	for (auto object : Packet.objects)
	{
		ProtobufMessage::SnapShotMessage_Objectstate* objectState = snapshotMessage->add_objectsnapshot();
		objectState->set_interacting(object->interacting);
		objectState->set_positionx(object->position.x);
		objectState->set_positiony(object->position.y);
		//objectState->set_linear_velocityx(object->linear_velocity.x);
		//objectState->set_linear_velocityy(object->linear_velocity.y);
		objectState->set_angle(object->angle);
	}

	// Send
	serverMessage->set_allocated_snapshotmsg(snapshotMessage);
	UDP_Send(serverMessage);
}

void NetworkSnapshot::ClearDelta()
{
	// Find early sequence
	int lowestBaseSequence = 0;
	for (auto c : clients)
	{
		if(lowestBaseSequence == 0)
			lowestBaseSequence = c.second->baseSequence;
		else if (c.second->baseSequence < lowestBaseSequence)
			lowestBaseSequence = c.second->baseSequence;
	}

	// Clear early base sequence that clients arn't using
	std::map<int, SnapshotPacket*>::iterator iterUpper;
	iterUpper = baselineSnapshotMap.lower_bound(lowestBaseSequence);
	baselineSnapshotMap.erase(baselineSnapshotMap.begin(), --iterUpper); // Element before lowerest
}

void NetworkSnapshot::SendClientInput(clientInputs* inputs)
{
	ProtobufMessage::UDPMessage* serverMessage = new ProtobufMessage::UDPMessage;
	ProtobufMessage::SnapShotInputMessage* inputMessage = new ProtobufMessage::SnapShotInputMessage;

	// Input
	inputMessage->set_sequence(inputs->sequence);
	inputMessage->set_up(inputs->up);
	inputMessage->set_down(inputs->down);
	inputMessage->set_left(inputs->left);
	inputMessage->set_right(inputs->right);

	serverMessage->set_allocated_snapshotinputmsg(inputMessage);
	UDP_Send(serverMessage);
}

void NetworkSnapshot::TCP_Consumer()
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

void NetworkSnapshot::UDP_Consumer()
{
	while (!udp_queue.isEmpty()) {
		auto serverMessage = udp_queue.pop();

		switch (serverMessage->msg_case())
		{
		// Server Message
		case 2:
		{
			if (serverSequence < serverMessage->snapshotmsg().sequence())
			{
				serverSequence = serverMessage->snapshotmsg().sequence();

				SnapshotPacket* deltaSnapshotPacket = new SnapshotPacket();
				if(serverMessage->snapshotmsg().initial())
				{ 
					deltaSnapshotPacket = initialSnapshotPacket;
				}
				else
				{
					int deltaSequence = serverMessage->snapshotmsg().deltapacketindex();
					deltaSnapshotPacket = baselineSnapshotMap[deltaSequence];

					// Clear delta client side TODO: Function
					std::map<int, SnapshotPacket*>::iterator iterUpper;
					iterUpper = baselineSnapshotMap.lower_bound(deltaSequence); // Not current ack sequence due to packetlose
					baselineSnapshotMap.erase(baselineSnapshotMap.begin(), --iterUpper); // Element before lowerest
				}

				// Snapshot
				SnapshotPacket* snapshotPacket = new SnapshotPacket();

				int size = serverMessage->snapshotmsg().objectsnapshot_size();
				for (int i = 0; i < size; i++)
				{
					const ProtobufMessage::SnapShotMessage_Objectstate objectState = serverMessage->snapshotmsg().objectsnapshot(i);

					// Cube
					ObjectState* cubeState = new ObjectState();

					cubeState->interacting = objectState.interacting();
					cubeState->position.x = deltaSnapshotPacket->objects[i]->position.x + objectState.positionx();
					cubeState->position.y = deltaSnapshotPacket->objects[i]->position.y + objectState.positiony();
					//cubeState->linear_velocity.x = objectState.linear_velocityx();
					//cubeState->linear_velocity.y = objectState.linear_velocityy();
					cubeState->angle = deltaSnapshotPacket->objects[i]->angle + objectState.angle();

					snapshotPacket->objects.push_back(cubeState);
				}
				snapshotPacket->time = clockPacket->getElapsedTime().asMilliseconds();
				baselineSnapshotMap.insert(std::pair<int, SnapshotPacket*>(serverSequence, snapshotPacket));
				snapshotQueue.push(snapshotPacket);
				
				// Send ACK
				ProtobufMessage::UDPMessage* ACKMessage = new ProtobufMessage::UDPMessage;
				ProtobufMessage::UDPStringMessage* stringMsg = new ProtobufMessage::UDPStringMessage;
				stringMsg->set_msg("snapshotack::" + std::to_string(serverMessage->snapshotmsg().netid()) + "::" + std::to_string(serverSequence));
				ACKMessage->set_allocated_udpstringmsg(stringMsg);
				UDP_Send(ACKMessage);
			}
		}
		break;
		case 4:
		{
			// Split tcp message
			std::vector<std::string> splitMessage = splitServerMessage(serverMessage->udpstringmsg().msg(), "::");

			// ACK
			if (splitMessage[0] == "snapshotack")
			{
				//std::cout << "base" + splitMessage[2] << std::endl;
				clients[std::stoi(splitMessage[1])]->baseSequence = std::stoi(splitMessage[2]);
				ClearDelta();
			}
		}
		break;
		case 5:
		{	
			// Client Inputs
			clientInputs* inputs = new clientInputs();
			inputs->up = serverMessage->snapshotinputmsg().up();
			inputs->down = serverMessage->snapshotinputmsg().down();
			inputs->left = serverMessage->snapshotinputmsg().left();
			inputs->right = serverMessage->snapshotinputmsg().right();

			clientInputQueue.push(inputs);
		}
		break;
		}
	}
}

