#pragma once

#include <SFML\Network.hpp>
#include <thread>
#include <iostream>

#include <queue>
#include "MutexQueue.h"

#include "src\Proto\ProtoMessage.pb.h"

#include "MessagingSystem.h"

enum class NetworkSynchronisationMethod
{
	LockStep,
	SnapShot,
	State,
	ClientSidePredictionAndServerReconciliation
};

struct client
{
public:
	int netID;
	int sequence = 0;
	int baseSequence = 0;

	// State Delta
	int ackSequence;
	int32_t ackBits;
	std::vector<int> stateBaseSequences;
};

class Network
{
public:
	Network();
	~Network();

	void init(std::string ip);

	// TCP
	void TCP_Connect();
	void TCP_Receive();

	void TCP_Send(ProtobufMessage::TCPMessage* msg);
	virtual void TCP_Consumer() = 0;
	std::vector<std::string> splitServerMessage(std::string msg, std::string delimiter);

	// UDP
	void UDP_Receive();

	void UDP_Send(ProtobufMessage::UDPMessage* msg);
	virtual void UDP_Consumer() = 0;

	void ping();

	void Disconnect();

	bool isConnected() { return connected; }

	float GetInPacketSize() { return packetInSize; }
	float GetOutPacketSize() { return packetOutSize; }

	void SetHost(bool host) { serverHost = host; }
	bool GetHost() { return serverHost; }

	void SetCube(int value) { cubes = value; }
	int GetCube() { return cubes; }


	void SetNetworkMethod(NetworkSynchronisationMethod method) { networkMethod = method; }
	NetworkSynchronisationMethod GetNetworkMethod() { return networkMethod; }

protected:
	MutexQueue<ProtobufMessage::TCPMessage*> tcp_queue;
	MutexQueue<ProtobufMessage::UDPMessage*> udp_queue;

	std::map<int, client*> clients;

	bool serverHost;

	int packetIndex{ 1 };
	int serverSequence{ 0 };
private:
	sf::TcpSocket tcp_socket;
	sf::UdpSocket udp_socket;

	std::thread tcp_receive;
	std::thread udp_receive;

	sf::IpAddress ipAddress = "192.168.1.123"; // sf::IpAddress::getLocalAddress();

	unsigned short TCP_port = 8080;
	unsigned short UDP_port = 8080;

	bool connected{ false };
	bool endThreads{ false };

	float packetInSize;
	float packetOutSize;

	sf::Clock clockIn;
	sf::Clock clockOut;

	NetworkSynchronisationMethod networkMethod{ NetworkSynchronisationMethod::LockStep };

	int cubes;
};