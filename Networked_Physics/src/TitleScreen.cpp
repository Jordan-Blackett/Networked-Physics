#include "TitleScreen.h"



TitleScreen::TitleScreen()
{
}


TitleScreen::~TitleScreen()
{
}

void TitleScreen::LoadContent(Network* network, sf::Clock* clock)
{
	network_ = network;
	clock_ = clock;

	std::string ipAddressInput;
	std::string methodInput;
	int hostinput;
	int cubeInput;
	bool host;

	std::cout << "Connect - IP Address " << std::endl;
	std::cin >> ipAddressInput;

	if (ipAddressInput == "1")
	{
		ipAddressInput = "192.168.1.33";
		//methodInput = "2";
		host = true;
	}
	else if (ipAddressInput == "2")
	{
		ipAddressInput = "192.168.1.33";
		//methodInput = "2";
		host = false;
	}

	std::cout << "Host? 1 - y 2 - n" << std::endl;
	std::cin >> hostinput;

	if (hostinput == 1)
		host = true;
	else
		host = false;

	std::cout << "1 - Lockstep (Removed) | 2 - Snapshot | 3 - State | 4 - Client Side Prediction And Server Reconciliation" << std::endl;
	std::cin >> methodInput;
	
	std::cout << "Number of cubes 1 - Small  | 2 - Medium | 3 - Large" << std::endl;
	std::cin >> cubeInput;

	switch (std::stoi(methodInput))
	{
	case 1:
		network_ = new NetworkLockStep();
		network_->SetNetworkMethod(NetworkSynchronisationMethod::LockStep);
		break;
	case 2:
		network_ = new NetworkSnapshot();
		network_->SetNetworkMethod(NetworkSynchronisationMethod::SnapShot);
		break;
	case 3:
		network_ = new NetworkState();
		network_->SetNetworkMethod(NetworkSynchronisationMethod::State);
		break;
	case 4:
		network_ = new NetworkPredictionAndReconciliation();
		network_->SetNetworkMethod(NetworkSynchronisationMethod::ClientSidePredictionAndServerReconciliation);
		break;
	}

	network_->SetHost(host);
	network_->SetCube(cubeInput);
	network_->init(ipAddressInput);

	ScreenManager::GetInstance()->AddScreen(new GameScreen, network_, clock_);
}

void TitleScreen::UnloadContent()
{
}

void TitleScreen::Update(sf::RenderWindow &Window)
{
}

void TitleScreen::Render(sf::RenderWindow &Window)
{
}

