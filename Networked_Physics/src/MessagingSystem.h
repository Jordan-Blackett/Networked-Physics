#pragma once

#include <iostream>
#include <functional>
#include <queue>
#include <vector>

class Message
{
public: 
	Message(const std::string event)
	{
		messageEvent = event;
	}

	std::string getEvent()
	{
		return messageEvent;
	}

private: 
	std::string messageEvent;

};

class MessageManager
{
public:
	MessageManager();
	~MessageManager();

	void addReceiver(std::function<void(Message)> messageReceiver)
	{
		receivers.push_back(messageReceiver);
	}

	void sendMessage(Message message)
	{
		messages.push(message);
	}

	void notify()
	{
		while (!messages.empty())
		{
			for (auto iter = receivers.begin(); iter != receivers.end(); iter++)
			{
				(*iter)(messages.front());
			}
			messages.pop();
		}
	}


private:
	std::vector<std::function<void(Message)>> receivers;
	std::queue<Message> messages;
};

class MessageNode
{
public:
	MessageNode();

	void MessageNodeInit(MessageManager *messagingManger)
	{
		this->messageManager = messagingManger;
		this->messageManager->addReceiver(this->getNotifyFunc());
	}

	virtual void update() {};

	void send(Message message)
	{
		messageManager->sendMessage(message);
	}

private:
	MessageManager* messageManager;

	std::function<void(Message)> getNotifyFunc()
	{
		auto messageListener = [=](Message message) -> void {
			this->onNotify(message);
		};
		return messageListener;
	}

	virtual void onNotify(Message message)
	{
		// Do something here. Your choice. You could do something like this.
		// std::cout << "Siopao! Siopao! Siopao! (Someone forgot to implement onNotify().)" << std::endl;
	}
};
