#include "GameScreen.h"



GameScreen::GameScreen()
{
}


GameScreen::~GameScreen()
{
}

void GameScreen::LoadContent(Network* network, sf::Clock* clock)
{
	network_ = network;
	clock_ = clock;

	// Prepare the world
	InitPhysicalWorld();

	// Init Walls
	sf::Vector2f wallTopBottom = sf::Vector2f(ScreenWidth, 10);
	sf::Vector2f wallLeftRight = sf::Vector2f(10, ScreenHeight);

	CreateWall(ScreenWidth / 2, 0, wallTopBottom); // Top
	CreateWall(ScreenWidth / 2, ScreenHeight, wallTopBottom); // Bottom
	CreateWall(0, ScreenHeight / 2, wallLeftRight); // Left
	CreateWall(ScreenWidth, ScreenHeight / 2, wallLeftRight); // Right

	networkMethod = network_->GetNetworkMethod();

	int cubeMethod = network_->GetCube();
	switch (cubeMethod)
	{
	case 1:
		rows = 20;
		columns = 15;
		gridOffset = 50;
		break;
	case 2:
		rows = 39;
		columns = 29;
		gridOffset = 25;
		break;
	case 3:
		int rows = 51;
		int columns = 38;
		int gridOffset = 19;
		break;
	}

	if (networkMethod == NetworkSynchronisationMethod::State)
	{
	/*	rows = 20;
		columns = 15;
		gridOffset = 50;*/
	}

	// Grid
	if (networkMethod != NetworkSynchronisationMethod::ClientSidePredictionAndServerReconciliation)
	{
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < columns; j++)
			{

				EntityBox* box = new EntityBox();
				box->init(world, sf::Vector2i((i * gridOffset) + 40, (j * gridOffset) + 40), boxSize, scale);
				boxes.push_back(box);
			}
		}
	}

	// Set Boxes Asleep (Snapshot - For non host players)
	for (auto b : boxes)
		b->SetAsleep();

	// Init Player and Boxes
	player.init(world, sf::Vector2i(200, 400), playerSize, scale);

	host = network_->GetHost();

	NetworkPlayer* networkPlayer = new NetworkPlayer();
	bool temp = !host;
	networkPlayer->SetFix(temp);
	networkPlayer->init(world, sf::Vector2i(800, 400), networkPlayersSize, scale);
	networkPlayers.push_back(networkPlayer);

	//networkMethod = network_->GetNetworkMethod();

	int stateTempIndex = 0;
	switch (networkMethod) {
	case NetworkSynchronisationMethod::SnapShot:
		dynamic_cast<NetworkSnapshot *>(network_)->SetClock(&packetClock);
		snapshotQueue = dynamic_cast<NetworkSnapshot *>(network_)->getSnapshotQueue();
		ClientPendingInputsQueue = dynamic_cast<NetworkSnapshot *>(network_)->getClientInputQueue();
		dynamic_cast<NetworkSnapshot *>(network_)->SetInitialPacket(initialStatePacket());
		break;
	case NetworkSynchronisationMethod::State:
		dynamic_cast<NetworkState *>(network_)->SetClock(&packetClock);
		stateJitterBuffer = dynamic_cast<NetworkState *>(network_)->getStateJitterBuffer();
		stateJitterBuffer->init(100); // 32
		dynamic_cast<NetworkState *>(network_)->SetClock(&packetClock);
		dynamic_cast<NetworkState *>(network_)->initPacketVector(boxes.size() + 1); // + players

		player.SetStatePriority(100000);

		dt = 1.0f / 120.0f;

		// Set priority vector
		stateTempIndex = 0;
		for (auto b : boxes)
			statePriority.push_back(PriorityIndex(stateTempIndex++, b->GetStatePriorityPointer()));
		
		break;
	case NetworkSynchronisationMethod::ClientSidePredictionAndServerReconciliation:
		statePRQueue = dynamic_cast<NetworkPredictionAndReconciliation *>(network_)->GetStatePRQueue();
		// Input Buffer
		
		// Tickrate
		dt = 1.0f / 60.0f;

		//if(!host)
			//dt = 1.0f / 30.0f;

		// Football
		EntityBox * box = new EntityBox();
		box->init(world, sf::Vector2i(100, 100), sf::Vector2f(100, 100), scale);
		boxes.push_back(box);

		EntityBox * box1 = new EntityBox();
		box1->init(world, sf::Vector2i(210, 210), sf::Vector2f(100, 100), scale);
		boxes.push_back(box1);

		EntityBox * box2 = new EntityBox();
		box2->init(world, sf::Vector2i(420, 420), sf::Vector2f(100, 100), scale);
		boxes.push_back(box2);

		EntityBox * box3 = new EntityBox();
		box3->init(world, sf::Vector2i(530, 530), sf::Vector2f(100, 100), scale);
		boxes.push_back(box3);

		break;
	}

	// Debug Text
	InitDebugText();

	// Debug Graph's
	debugDataInGraph.Init(sf::Vector2i(700, 120), "In");
	debugDataOutGraph.Init(sf::Vector2i(700, 220), "Out");
}

void GameScreen::UnloadContent()
{
}

void GameScreen::Update(sf::RenderWindow &Window)
{
	//if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
	//{
	//	sf::Vector2i mousePosition = sf::Mouse::getPosition(Window);
	//	EntityBox* box = new EntityBox();
	//	box->init(world, mousePosition, boxSize, scale);
	//	boxes.push_back(box);
	//}

	// Timestep - DeltaTime
	double newTime = simulationClock_.getElapsedTime().asSeconds();
	double frameTime = newTime - simulationTimeLastUpdated_;
	simulationTimeLastUpdated_ = newTime;

	if (frameTime > 0.25f)
		frameTime = 0.25f; // Avoid "Spiral of death"

	accumulator += frameTime;

	while (accumulator >= dt)
	{
	//std::cout << accumulator << "//" << dt << std::endl;
		switch (networkMethod) {
		case NetworkSynchronisationMethod::SnapShot:
		{
			playerInputSnapshot = player.SnapshotInput();
			//networkPlayers[0]->Input(playerInputSnapshot);

			if (!host) {
				// Input
				//playerInputSnapshot = player.SnapshotInput();

				if (!playerInputSnapshot->none)
				{
					playerInputSnapshot->sequence = inputSequence;
					inputSequence++;

					// Send Packet
					dynamic_cast<NetworkSnapshot *>(network_)->SendClientInput(playerInputSnapshot);			
				}
			}
		}
		break;
		case NetworkSynchronisationMethod::State:
		{
			playerInput = player.StateInput();

			bool isButtonPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::E);

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::E) && !wasButtonPressed && !host)
			{
				desync = !desync;
				std::cout << "Desync" << desync << std::endl;
			}

			wasButtonPressed = isButtonPressed;
		}
		break;
		case NetworkSynchronisationMethod::ClientSidePredictionAndServerReconciliation:
			if (host)
			{
				// Host Player Input
				playerInput = player.StateInput();

				// Pull from input buffer - std::vector<InputStateData*> inputstemp
				inputstemp = dynamic_cast<NetworkPredictionAndReconciliation *>(network_)->GetInputBufferData();
				
				// Apply Inputs
				int tempPlayers = 0;
				for (auto input : inputstemp)
				{
					//std::cout << input->frameID << std::endl;
					networkPlayers[tempPlayers]->StateInput(input);
					tempPlayers++;
				}

				// Run Simulation
				world->Step(1 / 60.0f, 8, 3);

				// Store Frame Data
				//frameData.push_back(CaptureFrame());

				inputstemp.push_back(playerInput);

				// Send State
				SendState(inputstemp);
			}
			else
			{
				// New state packet
				Reconciliation();

				// Network player interp
				

				// Store Input
				playerInput = player.StateInput();

				// Run Simulation
				world->Step(1 / 60.0f, 8, 3);

				// Store Frame Data
				int frameIndex = frameNum;
				frameNum++;
				frameData.insert(std::pair<int, FrameData*>(frameIndex, CaptureFrame()));
				
				// Send Input + Frame #
				dynamic_cast<NetworkPredictionAndReconciliation *>(network_)->SendInputPRPacket(playerInput, frameIndex);
			}
		break;
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && host)
		{
			b2Vec2 center = player.GetBody()->GetPosition();
			FindAllCubesAndApplyBlastImpulse(center, 5, -2.f);
		} 
		else if(sf::Mouse::isButtonPressed(sf::Mouse::Right) && host)
		{
			b2Vec2 center = player.GetBody()->GetPosition();
			FindAllCubesAndApplyBlastImpulse(center, 5, 2.f);
		}

		if (networkMethod == NetworkSynchronisationMethod::SnapShot)
		{
			if (!ClientPendingInputsQueue->empty())
			{
				clientInputs* input = ClientPendingInputsQueue->front();
				ClientPendingInputsQueue->pop();

				networkPlayers[0]->Input(input);
			}
		}
		
		if ((host || networkMethod == NetworkSynchronisationMethod::State) && networkMethod != NetworkSynchronisationMethod::ClientSidePredictionAndServerReconciliation)
		{
			world->Step(1 / 30.0f, 8, 3);
		}

		accumulator -= dt;
	}
	
	// Network - Update
	switch (networkMethod) {
	case NetworkSynchronisationMethod::SnapShot:
	{
		if (!host)
		{
			// Interpolation Buffer
			int buffer_ratio = 3; // 2 snapshotQueue->size() >= buffer_ratio
			if (snapshotQueue->size() >= buffer_ratio && !initialized)
			//if (!snapshotQueue->empty() && !initialized)
			{
				last_snapshotState = snapshotQueue->front();
				snapshotQueue->pop();
				initialized = true;
				packetClockBuffer.restart();
				//packetClock.restart();
			}
			else if (initialized)
			{
				// Purge buffer of expired frames //while
				if (snapshotQueue->size() > 1)
				{
					//while (d > snapshotQueue->front()->time && snapshotQueue->size() > 1) // && !snapshotQueue->empty()
					if (d > snapshotQueue->front()->time && !snapshotQueue->empty())
					{
						last_snapshotState = snapshotQueue->front();
						snapshotQueue->pop();
					}
				}
			
				// Delta Time
				float currentTime = packetClockBuffer.getElapsedTime().asMilliseconds();
				float delta = (currentTime - lastBufferUpdateTime);
				lastBufferUpdateTime = currentTime;

				if (!snapshotQueue->empty())
				{
					SnapshotPacket* snapshotState = snapshotQueue->front();

					float deltaTimePacket = snapshotState->time - last_snapshotState->time;
					float alpha = (d - last_snapshotState->time) / (deltaTimePacket);

					// clamp
					//float alphaClamped = alpha;
					float alphaClamped = std::max(0.0f, std::min(alpha, 1.0f));
					//std::cout << alpha << std::endl;

					// Player
					b2Vec2 position(snapshotState->objects[0]->position.x, snapshotState->objects[0]->position.y);
					b2Vec2 last_position(last_snapshotState->objects[0]->position.x, last_snapshotState->objects[0]->position.y);

					float linearInterpolationX = lerp(last_position.x, position.x, alphaClamped); // 1.0 - alphaClamped
					float linearInterpolationY = lerp(last_position.y, position.y, alphaClamped);
					b2Vec2 linearInterpolationPosition = b2Vec2(linearInterpolationX, linearInterpolationY);

					float linearInterpolationAngle = slerp(last_snapshotState->objects[0]->angle, snapshotState->objects[0]->angle, alphaClamped);

					player.GetBody()->SetTransform(linearInterpolationPosition, linearInterpolationAngle);
						
					// Network Players Data
					int temp = 1;
					for (auto players : networkPlayers)
					{
						b2Vec2 position(snapshotState->objects[temp]->position.x, snapshotState->objects[temp]->position.y);
						b2Vec2 last_position(last_snapshotState->objects[temp]->position.x, last_snapshotState->objects[temp]->position.y);

						float linearInterpolationX = lerp(last_position.x, position.x, alphaClamped); // 1.0 - alphaClamped
						float linearInterpolationY = lerp(last_position.y, position.y, alphaClamped);
						b2Vec2 linearInterpolationPosition = b2Vec2(linearInterpolationX, linearInterpolationY);

						float linearInterpolationAngle = slerp(last_snapshotState->objects[temp]->angle, snapshotState->objects[temp]->angle, alphaClamped);

						players->GetBody()->SetTransform(linearInterpolationPosition, linearInterpolationAngle);
						temp++;
					}

					// Box Data
					for (auto b : boxes)
					{
						// Interacting
						b->SetInteracting(snapshotState->objects[temp]->interacting);

						b2Vec2 position(snapshotState->objects[temp]->position.x, snapshotState->objects[temp]->position.y);
						b2Vec2 last_position(last_snapshotState->objects[temp]->position.x, last_snapshotState->objects[temp]->position.y);

						// Hermite interpolation of position - REMOVED
						//float hermiteInterpolationX = hermiteLerp(alpha, last_position.x, position.x, last_snapshotState->objects[temp]->linear_velocity.x * delta, snapshotState->objects[temp]->linear_velocity.x * delta);
						//float hermiteInterpolationY = hermiteLerp(alpha, last_position.y, position.y, last_snapshotState->objects[temp]->linear_velocity.y * delta, snapshotState->objects[temp]->linear_velocity.y * delta);
						//b2Vec2 hermiteInterpolationPosition = b2Vec2(hermiteInterpolationX, hermiteInterpolationY)

						// Linear interpolation of position
						float linearInterpolationX = lerp(last_position.x, position.x, alphaClamped); // 1.0 - alphaClamped
						float linearInterpolationY = lerp(last_position.y, position.y, alphaClamped);
						b2Vec2 linearInterpolationPosition = b2Vec2(linearInterpolationX, linearInterpolationY);

						// Spherical interpolation of rotation
						float linearInterpolationAngle = slerp(last_snapshotState->objects[temp]->angle, snapshotState->objects[temp]->angle, alphaClamped);

						b->GetBody()->SetTransform(linearInterpolationPosition, linearInterpolationAngle);
						temp++;
					}
				}
				d += delta;
			}
		}
		else {
			//if (!ClientPendingInputsQueue->empty())
			//{
			//	clientInputs* input = ClientPendingInputsQueue->front();
			//	ClientPendingInputsQueue->pop();

			//	networkPlayers[0]->Input(input);
			//}
		}
	}
	break;
	case NetworkSynchronisationMethod::State:
	{
		// Priority Accumulator
		if (host)
		{
			for (auto b : boxes)
				if (!b->IsSleep())
					b->UpdateStatePriority(1);
				else
					b->UpdateStatePriority(100);
		} 
		else
		{	
			// Time delay
			timeRequestAccumulator += frameTime;
			if (timeRequestAccumulator >= timeRequestDT)
			{
				// Send time request packet
				dynamic_cast<NetworkState *>(network_)->SendTimeRequest();
				timeRequestAccumulator = 0;
			}

			// Jitter Buffer
			if (stateJitterBuffer->time())
			{
				// Desync
				if (desync)
				{
					StatePacket* statePacket = stateJitterBuffer->RetrievePacket();

					// Packetloss
					//sequenceVS

					//positionError = 0; 
					angleError = 0;

				}
				else
				{
					StatePacket* statePacket = stateJitterBuffer->RetrievePacket();

					bool packetLoss = false;
					if (statePacket->sequence == sequenceVS + 1)
					{
					}
					else
					{
						// Packet Loss
						packetLoss = true;
					}

					sequenceVS = statePacket->sequence;

				/*	positionError *= smoothingValueSmall;
					angleError *= smoothingValueSmall;*/

		/*			if (positionError < 5)
						positionError = 0;*/

					//if (angleError < 5)
					//	angleError = 0;

					// Smoothing Error Offset
					//statePacket->objects[0]->position.x
					
					sf::Vector2f errorOffset;
					if (packetLoss)
					{
						errorOffset.x = player.GetBody()->GetTransform().p.x - statePacket->objects[0]->position.x;
						errorOffset.y = player.GetBody()->GetTransform().p.y - statePacket->objects[0]->position.y;
						errorOffset.x = VisualSmoothing(errorOffset.x);
						errorOffset.y = VisualSmoothing(errorOffset.y);
						player.SetStateErrorPosition(errorOffset);
					}
					else
					{
						errorOffset = player.GetStateErrorPosition();
						errorOffset.x = VisualSmoothing(errorOffset.x);
						errorOffset.y = VisualSmoothing(errorOffset.y);
						player.SetStateErrorPosition(errorOffset);
					}


					// Player Data
					player.GetBody()->SetTransform(b2Vec2(statePacket->objects[0]->position.x + errorOffset.x, statePacket->objects[0]->position.y + errorOffset.y), statePacket->objects[0]->angle + angleError);
					player.GetBody()->SetLinearVelocity(b2Vec2(statePacket->objects[0]->linear_velocity.x, statePacket->objects[0]->linear_velocity.y));
					player.GetBody()->SetAngularVelocity(statePacket->objects[0]->angular_velocity);

					// Box Data
					for (auto object : statePacket->objects)
					{
						int objectID = object->indexID;

						if (objectID < boxes.size()) // Box
						{
							// 
							if (packetLoss)
							{
								errorOffset.x = boxes[objectID]->GetBody()->GetTransform().p.x - object->position.x;
								errorOffset.y = boxes[objectID]->GetBody()->GetTransform().p.y - object->position.y;
								errorOffset.x = VisualSmoothing(errorOffset.x);
								errorOffset.y = VisualSmoothing(errorOffset.y);
								boxes[objectID]->SetStateErrorPosition(errorOffset);
							}
							else
							{
								errorOffset = boxes[objectID]->GetStateErrorPosition();
								errorOffset.x = VisualSmoothing(errorOffset.x);
								errorOffset.y = VisualSmoothing(errorOffset.y);
								boxes[objectID]->SetStateErrorPosition(errorOffset);
							}


							boxes[objectID]->GetBody()->SetTransform(b2Vec2(object->position.x + errorOffset.x, object->position.y + errorOffset.y) + positionError, object->angle + angleError);
							boxes[objectID]->GetBody()->SetLinearVelocity(b2Vec2(object->linear_velocity.x, object->linear_velocity.y));
							boxes[objectID]->GetBody()->SetAngularVelocity(object->angular_velocity);
						}
					}

					// Inputs
					if (!statePacket->inputs.empty())
						player.NetworkInput(*statePacket->inputs.front());
				}
			}
		}	
	}
	break;
	case NetworkSynchronisationMethod::ClientSidePredictionAndServerReconciliation:
	{
	}
	break;
	}
	
	// Network - Send
	ticktime_ = sf::milliseconds(16.6);
	timeLastUpdated_ = clock_->getElapsedTime();
	if (timeLastUpdated_ >= ticktime_ && host)
	{
		switch (networkMethod) {
		case NetworkSynchronisationMethod::LockStep:
			break;
		case NetworkSynchronisationMethod::SnapShot:
		{	
			// Snapshot
			SnapshotPacket* snapshotPacket = new SnapshotPacket();

			// Players
			ObjectState* cubeState = new ObjectState();
			cubeState->position.x = player.GetBody()->GetPosition().x;
			cubeState->position.y = player.GetBody()->GetPosition().y;
			cubeState->angle = player.GetBody()->GetAngle();
			snapshotPacket->objects.push_back(cubeState);

			// Network Players
			for (auto nplayers : networkPlayers)
			{
				ObjectState* cubeState = new ObjectState();
				cubeState->position.x = nplayers->GetBody()->GetPosition().x;
				cubeState->position.y = nplayers->GetBody()->GetPosition().y;
				cubeState->angle = nplayers->GetBody()->GetAngle();
				snapshotPacket->objects.push_back(cubeState);
			}
		
			// Box Data
			for (auto b : boxes)
			{
				ObjectState* cubeState = new ObjectState();

				if (b->IsSleep())
					cubeState->interacting = true;
				else
					cubeState->interacting = false;

				cubeState->position.x = b->GetBody()->GetPosition().x;
				cubeState->position.y = b->GetBody()->GetPosition().y;
				//cubeState->linear_velocity.x = b->GetBody()->GetLinearVelocity().x;
				//cubeState->linear_velocity.y = b->GetBody()->GetLinearVelocity().y;
				cubeState->angle = b->GetBody()->GetAngle();

				snapshotPacket->objects.push_back(cubeState);
			}

			// Send Packet
			dynamic_cast<NetworkSnapshot *>(network_)->DeltaCompression(snapshotPacket);
		}
		break;
		case NetworkSynchronisationMethod::State:
		{
			// State Packet
			StatePacket* statePacket = new StatePacket();

			// Inputs
			//if (playerInput != nullptr)
			//{
				statePacket->inputs.push_back(playerInput);
				//playerInput = new InputStateData();
			//}

			// Sorting Priority
			std::sort(statePriority.begin(), statePriority.end(), [](PriorityIndex a, PriorityIndex b) { return *a.priority > *b.priority; }); //[](float* a, float* b) { return *a > *b; }
			//std::cout << "TOP" << std::endl;
			//for (auto p : statePriority)
			//	std::cout << *p.priority << std::endl;

			// Player Data
			ObjectStateState* cubeState = new ObjectStateState();
			cubeState->indexID = boxes.size();
			cubeState->position.x = player.GetBody()->GetPosition().x;
			cubeState->position.y = player.GetBody()->GetPosition().y;
			cubeState->angle = player.GetBody()->GetAngle();
			cubeState->linear_velocity.x = player.GetBody()->GetLinearVelocity().x;
			cubeState->linear_velocity.x = player.GetBody()->GetLinearVelocity().y;
			cubeState->angular_velocity = player.GetBody()->GetAngularVelocity();

			statePacket->objects.push_back(cubeState);

			// Hightest Priority
			int nObjects = 64;
			if (boxes.size() < 64)
			{
				nObjects = boxes.size();
			}

			for (int i = 0; i < nObjects; i++)
			{ 
				// Reset Priority
				*statePriority[i].priority = 0;

				// Box Data
				ObjectStateState* cubeState = new ObjectStateState();
				int objectID = statePriority[i].objectID;
				cubeState->indexID = objectID;
				cubeState->position.x = boxes[objectID]->GetBody()->GetPosition().x;
				cubeState->position.y = boxes[objectID]->GetBody()->GetPosition().y;
				cubeState->angle = boxes[objectID]->GetBody()->GetAngle();
				cubeState->linear_velocity.x = boxes[objectID]->GetBody()->GetLinearVelocity().x;
				cubeState->linear_velocity.y = boxes[objectID]->GetBody()->GetLinearVelocity().y;
				cubeState->angular_velocity = boxes[objectID]->GetBody()->GetAngularVelocity();

				statePacket->objects.push_back(cubeState);
			}

			// Send Packet
			dynamic_cast<NetworkState *>(network_)->DeltaCompression(statePacket);
		}
		break;
		case NetworkSynchronisationMethod::ClientSidePredictionAndServerReconciliation:
			//SendState(inputstemp);
		break;
		}
		debugDataOutGraph.Update(network_->GetOutPacketSize());
		timeLastUpdated_ = clock_->restart();
	}

	debugDataInGraph.Update(network_->GetInPacketSize());

	network_->TCP_Consumer();
	network_->UDP_Consumer();
}

void GameScreen::UIUpdate(sf::RenderWindow &Window)
{
	// Connected Status
	if (network_->isConnected())
		debugConnectedStatusText.setString("Connected");

	float data = network_->GetOutPacketSize();
	std::stringstream stream;
	stream << std::fixed << std::setprecision(0) << data;
	std::string s = stream.str();

	// Packet Size
	debugPacketSizeText.setString("" + s + "kb/s");
	
	// Active Boxes
	int sleepCount = 0;
	for (auto b : boxes)
		if (b->IsSleep())
			sleepCount++;

	debugNumOfActiveBoxesText.setString("Active: " + std::to_string(sleepCount));
}

void GameScreen::Render(sf::RenderWindow & Window)
{
	for (auto w : walls)
		Window.draw(w->rectangle);

	for (auto b : boxes)
		b->Render(Window);

	for (auto players : networkPlayers)
		players->Interp(clock_->getElapsedTime());

	for (auto players : networkPlayers)
		players->Render(Window);

	player.Render(Window);

	Window.draw(debugConnectedStatusText);
	Window.draw(debugPacketSizeText);
	Window.draw(debugNumOfBoxesText);
	Window.draw(debugNumOfActiveBoxesText);
	Window.draw(debugSynchronisationMethodText);
	Window.draw(debugHostText);

	debugDataInGraph.Render(Window);
	debugDataOutGraph.Render(Window);
}

void GameScreen::InitPhysicalWorld()
{
	world = new b2World(b2Vec2(0.0f, gravity));
	world->SetAllowSleeping(true);
	world->SetContinuousPhysics(true);
}

void GameScreen::InitDebugText()
{
	if (!debugFont.loadFromFile("Resources/Fonts/closeness/Closeness.ttf")) {}

	debugConnectedStatusText.setFont(debugFont);
	debugConnectedStatusText.setCharacterSize(debugTextSize);
	debugConnectedStatusText.setFillColor(debugTextColor);
	debugConnectedStatusText.setPosition(sf::Vector2f(855, 10));
	if (network_->isConnected())
		debugConnectedStatusText.setString("Connected");
	else
		debugConnectedStatusText.setString("Disconnected");

	debugPacketSizeText.setFont(debugFont);
	debugPacketSizeText.setCharacterSize(debugTextSize + 25);
	debugPacketSizeText.setFillColor(debugTextColor);
	debugPacketSizeText.setPosition(sf::Vector2f(410, 10));

	debugNumOfBoxesText.setFont(debugFont);
	debugNumOfBoxesText.setCharacterSize(debugTextSize);
	debugNumOfBoxesText.setFillColor(debugTextColor);
	debugNumOfBoxesText.setPosition(sf::Vector2f(20, 10));
	debugNumOfBoxesText.setString("Boxes: " + std::to_string(boxes.size()));

	debugNumOfActiveBoxesText.setFont(debugFont);
	debugNumOfActiveBoxesText.setCharacterSize(debugTextSize);
	debugNumOfActiveBoxesText.setFillColor(debugTextColor);
	debugNumOfActiveBoxesText.setPosition(sf::Vector2f(20, 50));

	debugSynchronisationMethodText.setFont(debugFont);
	debugSynchronisationMethodText.setCharacterSize(debugTextSize);
	debugSynchronisationMethodText.setFillColor(debugTextColor);
	debugSynchronisationMethodText.setPosition(sf::Vector2f(720, 10));

	debugHostText.setFont(debugFont);
	debugHostText.setCharacterSize(debugTextSize);
	debugHostText.setFillColor(debugTextColor);
	debugHostText.setPosition(sf::Vector2f(710, 40));
	if(host)
		debugHostText.setString("[HOST]");
	else
		debugHostText.setString("[CLIENT]");

	switch (networkMethod) {
	case NetworkSynchronisationMethod::LockStep:
		debugSynchronisationMethodText.setString("LockStep - ");
		break;
	case NetworkSynchronisationMethod::SnapShot:
		debugSynchronisationMethodText.setString("SnapShot - ");
		break;
	case NetworkSynchronisationMethod::State:
		debugSynchronisationMethodText.setString("State - ");
		break;
	}
}

void GameScreen::CreateWall(float posX, float posY, sf::Vector2f size)
{
	// Wall
	WorldWall* wall = new WorldWall();
	walls.push_back(wall);

	// Body
	b2BodyDef BodyDef;
	BodyDef.position = b2Vec2(posX / scale, posY / scale);
	BodyDef.type = b2_staticBody;
	wall->body = world->CreateBody(&BodyDef);

	// Shape
	b2PolygonShape Shape;
	Shape.SetAsBox((size.x / 2) / scale, (size.y / 2) / scale);

	//FixtureDef
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &Shape; // Sets the shape
	fixtureDef.density = 0.f;  // Sets the density of the body
	wall->body->CreateFixture(&fixtureDef); // Apply the fixture definition

	// Rectangle
	wall->rectangle.setSize(size);
	wall->rectangle.setOrigin(size.x / 2, size.y / 2);
	wall->rectangle.setPosition(wall->body->GetPosition().x * scale, wall->body->GetPosition().y * scale);
	wall->rectangle.setFillColor(sf::Color::Black);
}

void GameScreen::FindAllCubesAndApplyBlastImpulse(b2Vec2 center, float blastRadius, float blastPower)
{
	// Find all bodies with fixtures in blast radius
	MyQueryCallback queryCallback;
	b2AABB aabb;
	aabb.lowerBound = center - b2Vec2(blastRadius, blastRadius);
	aabb.upperBound = center + b2Vec2(blastRadius, blastRadius);
	world->QueryAABB(&queryCallback, aabb);

	//check which of these bodies have their center of mass within the blast radius
	for (int i = 0; i < queryCallback.foundBodies.size(); i++) {
		b2Body* body = queryCallback.foundBodies[i];
		b2Vec2 bodyCom = body->GetWorldCenter();

		//ignore bodies outside the blast range
		if ((bodyCom - center).Length() >= blastRadius)
			continue;

		ApplyBlastImpulse(body, center, bodyCom, blastPower);
	}
}

void GameScreen::ApplyBlastImpulse(b2Body* body, b2Vec2 blastCenter, b2Vec2 applyPoint, float blastPower) {
	b2Vec2 blastDir = applyPoint - blastCenter;
	float distance = blastDir.Normalize();
	//ignore bodies exactly at the blast point - blast direction is undefined
	if (distance == 0)
		return;
	float invDistance = 1 / distance;
	float impulseMag = blastPower * invDistance * invDistance;
	body->ApplyLinearImpulse(impulseMag * blastDir, applyPoint, true);
}

SnapshotPacket* GameScreen::initialStatePacket()
{
	// Snapshot
	SnapshotPacket* snapshotPacket = new SnapshotPacket();

	// Players
	ObjectState* cubeState = new ObjectState();
	cubeState->position.x = player.GetBody()->GetPosition().x;
	cubeState->position.y = player.GetBody()->GetPosition().y;
	cubeState->angle = player.GetBody()->GetAngle();
	snapshotPacket->objects.push_back(cubeState);

	// Network Players
	for (auto nplayers : networkPlayers)
	{
		ObjectState* cubeState = new ObjectState();
		cubeState->position.x = nplayers->GetBody()->GetPosition().x;
		cubeState->position.y = nplayers->GetBody()->GetPosition().y;
		cubeState->angle = nplayers->GetBody()->GetAngle();
		snapshotPacket->objects.push_back(cubeState);
	}

	// Box Data
	for (auto b : boxes)
	{
		ObjectState* cubeState = new ObjectState();

		if (b->IsSleep())
			cubeState->interacting = true;
		else
			cubeState->interacting = false;

		cubeState->position.x = b->GetBody()->GetPosition().x;
		cubeState->position.y = b->GetBody()->GetPosition().y;
		//cubeState->linear_velocity.x = b->GetBody()->GetLinearVelocity().x;
		//cubeState->linear_velocity.y = b->GetBody()->GetLinearVelocity().y;
		cubeState->angle = b->GetBody()->GetAngle();

		snapshotPacket->objects.push_back(cubeState);
	}

	return snapshotPacket;
}

float GameScreen::lerp(float a, float b, float s)
{
	return (a * (1.0 - s)) + (b * s);
}

// Hermite interpolation
float GameScreen::hermiteLerp(float t, float pos1, float pos2, float vel1, float vel2) {
	float t2 = pow(t, 2);
	float t3 = pow(t, 3);
	float a = 1 - 3 * t2 + 2 * t3;
	float b = t2 * (3 - 2 * t);
	float c = t * pow(t - 1, 2);
	float d = t2 * (t - 1);
	return a * pos1 + b * pos2 + c * vel1 + d * vel2;
}

// Spherical linear interpolation of two angles
float GameScreen::slerp(float v1, float v2, float alpha)
{
	const double pi = 3.141592;
	float angleDiff = v2 - v1;
	angleDiff = std::fmod(angleDiff, 2 * pi);
	angleDiff = std::fmod(angleDiff + 3 * pi, 2 * pi) - pi;
	return v1 + alpha * angleDiff;
}

FrameData* GameScreen::CaptureFrame()
{
	FrameData* frame = new FrameData();
	//frame->frameNum = frameNum;
	//frameNum++;

	// Inputs
	frame->input = playerInput;

	// Player Data
	ObjectStatePRState* cubeState = new ObjectStatePRState();
	cubeState->position.x = player.GetBody()->GetPosition().x;
	cubeState->position.y = player.GetBody()->GetPosition().y;
	cubeState->angle = player.GetBody()->GetAngle();
	cubeState->linear_velocity.x = player.GetBody()->GetLinearVelocity().x;
	cubeState->linear_velocity.x = player.GetBody()->GetLinearVelocity().y;
	cubeState->angular_velocity = player.GetBody()->GetAngularVelocity();

	frame->objects.push_back(cubeState);
	
	// Box
	for (auto b : boxes)
	{
		// Box Data
		ObjectStatePRState* cubeState = new ObjectStatePRState();
		cubeState->position.x = b->GetBody()->GetPosition().x;
		cubeState->position.y = b->GetBody()->GetPosition().y;
		cubeState->angle = b->GetBody()->GetAngle();
		cubeState->linear_velocity.x = b->GetBody()->GetLinearVelocity().x;
		cubeState->linear_velocity.y = b->GetBody()->GetLinearVelocity().y;
		cubeState->angular_velocity = b->GetBody()->GetAngularVelocity();

		frame->objects.push_back(cubeState);
	}

	return frame;
}

bool GameScreen::CompareEntitiesPositions(b2Vec2 pos1, b2Vec2 pos2, float threshold)
{

	return true;
}

void GameScreen::SendState(std::vector<InputStateData*> packetInputs)
{
	StatePRPacket* state = new StatePRPacket();

	// Frame Number
	//state->sequence = frameNum;
	//frameNum++;

	// Inputs
	state->inputs = packetInputs;
	
	// Players
	ObjectStatePRState* cubeState = new ObjectStatePRState();
	cubeState->position.x = player.GetBody()->GetPosition().x;
	cubeState->position.y = player.GetBody()->GetPosition().y;
	cubeState->angle = player.GetBody()->GetAngle();
	cubeState->linear_velocity.x = player.GetBody()->GetLinearVelocity().x;
	cubeState->linear_velocity.y = player.GetBody()->GetLinearVelocity().y;
	cubeState->angular_velocity = player.GetBody()->GetAngularVelocity();
	state->objects.push_back(cubeState);

	// Network Players
	for (auto nplayers : networkPlayers)
	{
		ObjectStatePRState* cubeState = new ObjectStatePRState();
		cubeState->position.x = nplayers->GetBody()->GetPosition().x;
		cubeState->position.y = nplayers->GetBody()->GetPosition().y;
		cubeState->angle = nplayers->GetBody()->GetAngle();
		cubeState->linear_velocity.x = nplayers->GetBody()->GetLinearVelocity().x;
		cubeState->linear_velocity.y = nplayers->GetBody()->GetLinearVelocity().y;
		cubeState->angular_velocity = nplayers->GetBody()->GetAngularVelocity();
		state->objects.push_back(cubeState);
	}

	// Box Data
	for (auto b : boxes)
	{
		ObjectStatePRState* cubeState = new ObjectStatePRState();

		cubeState->position.x = b->GetBody()->GetPosition().x;
		cubeState->position.y = b->GetBody()->GetPosition().y;
		cubeState->angle = b->GetBody()->GetAngle();
		cubeState->linear_velocity.x = b->GetBody()->GetLinearVelocity().x;
		cubeState->linear_velocity.y = b->GetBody()->GetLinearVelocity().y;
		cubeState->angular_velocity = b->GetBody()->GetAngularVelocity();

		state->objects.push_back(cubeState);
	}

	// Send
	dynamic_cast<NetworkPredictionAndReconciliation *>(network_)->SendStatePRPacket(state);
}

void GameScreen::Reconciliation()
{
	if (!statePRQueue->empty())
	{
		StatePRPacket* state = statePRQueue->front();
		statePRQueue->pop();

		// Compare Frame - Every entity is within threshold
		bool difference = true;
		float threshold = 10;
		int temp = 0;
		//for (auto obj : state->objects)
		//{
		//	if (difference)
		//	{
		//		break;
		//	}

		//	b2Vec2 packetPosition(state->objects[temp]->position.x, state->objects[temp]->position.y);
		//	// Player Compare
		//	if (temp == 0)
		//	{
		//		if (CompareEntitiesPositions(player.GetBody()->GetTransform().p, packetPosition, threshold))
		//		{

		//		}
		//	}
		//	// Network Players Compare
		//	else if (temp > 0 && temp <= networkPlayers.size())
		//	{
		//		if (CompareEntitiesPositions(networkPlayers[temp - 1]->GetBody()->GetTransform().p, packetPosition, threshold))
		//		{

		//		}
		//	}
		//	// Box Compare
		//	else
		//	{
		//		int offset = networkPlayers.size() + 1;
		//		if (CompareEntitiesPositions(boxes[temp - offset]->GetBody()->GetTransform().p, packetPosition, threshold))
		//		{

		//		}
		//	}

		//	temp++;
		//}

		if (difference)
		{
			int frameID = state->sequence;
			int maxFrameID = frameData.rbegin()->first;

			// Set Frame - TODO: its for checking back in time
			//frameData[frameID] = packet;

			//b2Vec2 packetPosition;
			//float angle;
			//b2Vec2 packetVelocity;
			//float angleVelocity;

			// Set Entities to frame
			int temp = 0;
			for (auto obj : state->objects)
			{
				b2Vec2 packetPosition(state->objects[temp]->position.x, state->objects[temp]->position.y);
				float angle = state->objects[temp]->angle;
				b2Vec2 packetVelocity(state->objects[temp]->linear_velocity.x, state->objects[temp]->linear_velocity.y);
				float angleVelocity = state->objects[temp]->angular_velocity;
				// Network Player
				if (temp < networkPlayers.size())
				{
					networkPlayers[temp]->GetBody()->SetTransform(packetPosition, angle);

					networkPlayers[temp]->SetInterpValue(sf::Vector2f(packetPosition.x, packetPosition.y));
					//networkPlayers[temp]->GetBody()->SetLinearVelocity(packetVelocity);
					//networkPlayers[temp]->GetBody()->SetAngularVelocity(angleVelocity);
				}
				// Player
				else if (temp == 1)
				{
					player.GetBody()->SetTransform(packetPosition, angle);
					player.GetBody()->SetLinearVelocity(packetVelocity);
					//std::cout << packetVelocity.x << std::endl;
					player.GetBody()->SetAngularVelocity(angleVelocity);
				}
				// Boxes
				else
				{
					int offset = networkPlayers.size() + 1;
					boxes[temp - offset]->GetBody()->SetTransform(packetPosition, angle);
					boxes[temp - offset]->GetBody()->SetLinearVelocity(packetVelocity);
					boxes[temp - offset]->GetBody()->SetAngularVelocity(angleVelocity);
				}

				temp++;
			}

			//std::cout << maxFrameID << " - " << frameID << " = " << maxFrameID - frameID << std::endl;
			//networkPlayers.

			// Rerun though all frames since
			for (int i = maxFrameID - frameID; i > 0; i--)
			{
				int index = maxFrameID - (i-1);
				player.NetworkInput(*frameData[index]->input);
				world->Step(1 / 60.0f, 8, 3);
			}

			// Delete Frame
			//frameData.erase(1,2);
		}
	}
}

float GameScreen::VisualSmoothing(float offsetValue)
{
	float offset = offsetValue;
	if (std::abs(offsetValue) > 15)
		offset *= smoothingValueLarge;
	else
		offset *= smoothingValueSmall;

	if (std::abs(offsetValue) < 0.35)
		offset = 0;

	return offset;
}
