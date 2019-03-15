#include "Network.h"



Network::Network()
{
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;
}


Network::~Network()
{
	// Delete all global objects allocated by libprotobuf.
	google::protobuf::ShutdownProtobufLibrary();
}

void Network::init(std::string ip)
{
	ipAddress = ip;
	std::cout << "Network Init - IP: " << ipAddress << " Port: " << TCP_port << std::endl;

	//messageNode.MessageNodeInit(messageManager);

	//tcp_socket.setBlocking(false);
	//udp_socket.setBlocking(false);

	udp_receive = std::thread(&Network::UDP_Receive, this);
	udp_socket.bind(8005);
	TCP_Connect();
}

void Network::TCP_Connect()
{
	// TCP socket
	sf::Socket::Status status = tcp_socket.connect(ipAddress, TCP_port);
	if (status != sf::Socket::Done) {
		// Error
		std::cerr << "TCP: Connect Failed";
	}
	else 
	{
		connected = true;

		// Connect Successful
		std::cout << "TCP: Connect Successful" << std::endl;

		// Receive Thread
		tcp_receive = std::thread(&Network::TCP_Receive, this);
		//udp_receive = std::thread(&Network::UDP_Receive, this);
	}
}

void Network::TCP_Receive()
{
	std::cout << "TCP: Receive started" << std::endl;

	char buffer_in[255];

	while (!endThreads)
	{
		std::size_t received;

		// TCP socket receive
		if (tcp_socket.receive(buffer_in, 100, received) != sf::Socket::Done) {
			// Error
			std::cerr << "TCP: Received failed" << std::endl;
		}
		else
		{
			// Receive successful
			std::string s = buffer_in;

			ProtobufMessage::TCPMessage* serverMessage = new ProtobufMessage::TCPMessage;
			serverMessage->ParseFromArray(buffer_in, received);

			tcp_queue.push(serverMessage);
		}
	}
}

void Network::TCP_Send(ProtobufMessage::TCPMessage* msg)
{
	std::string* buffer_out = new std::string();
	msg->SerializeToString(buffer_out);

	// TCP socket send
	if (tcp_socket.send(buffer_out->c_str(), buffer_out->size()) != sf::Socket::Done) {
		// Error
		std::cerr << "TCP: Sending failed" << std::endl;
	}
	else
	{
		//std::cout << msg->DebugString() << std::endl;
	}
}

std::vector<std::string> Network::splitServerMessage(std::string msg, std::string delimiter)
{
	std::vector<std::string> splitString;
	int startIndex = 0;
	int endIndex = 0;

	while ((endIndex = msg.find(delimiter, startIndex)) < msg.size())
	{
		std::string val = msg.substr(startIndex, endIndex - startIndex);
		splitString.push_back(val);
		startIndex = endIndex + delimiter.size();
	}

	if (startIndex < msg.size())
	{
		std::string val = msg.substr(startIndex);
		splitString.push_back(val);
	}

	return splitString;
}

void Network::UDP_Receive()
{
	std::cout << "UDP: Receive started" << std::endl;
	
	char buffer_in[100000];
	//udp_socket.bind(8005); // set listen port

	while (!endThreads)
	{
		std::size_t received;
		sf::IpAddress sender;
		unsigned short port;

		// UDP socket receive
		if (udp_socket.receive(buffer_in, 50000, received, sender, port) != sf::Socket::Done) {
			// Error
			std::cerr << "UDP: Receive failed." << std::endl;
		}
		else
		{
			// Receive successful
			std::string s = buffer_in;

			if (s != "") {
				ProtobufMessage::UDPMessage* serverMessage = new ProtobufMessage::UDPMessage;
				serverMessage->ParseFromArray(buffer_in, received);

				float mi = clockIn.restart().asSeconds();
				float callspersec = 1000.f / (mi * 1000.f);
				packetInSize = (serverMessage->ByteSizeLong() / 1024.0f) * callspersec;

				//std::cout << serverMessage->DebugString() << std::endl;
				udp_queue.push(serverMessage);
			}
		}
	}
}

void Network::UDP_Send(ProtobufMessage::UDPMessage* msg)
{
	std::string* buffer_out = new std::string();
	msg->SerializeToString(buffer_out);

	// UDP socket send
	if (udp_socket.send(buffer_out->c_str(), buffer_out->size(), ipAddress, UDP_port) != sf::Socket::Done) {
		// Error
		std::cerr << "UDP: sending failed." << std::endl;
	} else {
		float mi = clockOut.restart().asSeconds();
		float callspersec = 1000.f / (mi * 1000.f);
		packetOutSize = (buffer_out->size() / 1024.0f) * callspersec;
	}
}

void Network::ping()
{
	// Ping

}
