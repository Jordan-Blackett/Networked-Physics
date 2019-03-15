#include "DebugGraph.h"



DebugGraph::DebugGraph()
{
}


DebugGraph::~DebugGraph()
{
}

void DebugGraph::Init(sf::Vector2i position, std::string text)
{
	position_ = position;

	int tempOffset = 0;
	for (int i = 0; i < (sizeof(debugBars_) / sizeof(*debugBars_)); i++)
	{
		debugBars_[i].setSize(sf::Vector2f(3, 5));
		debugBars_[i].setPosition(position_.x + tempOffset, position_.y);
		debugBars_[i].setFillColor(barColour_);
		tempOffset += barSpaceOffset_;
	}

	lastBarIndex_ = numberOfBars_ - 1;

	text_ = text;

	if (!debugFont.loadFromFile("Resources/Fonts/closeness/Closeness.ttf")) {}

	debugText.setFont(debugFont);
	debugText.setCharacterSize(debugTextSize);
	debugText.setFillColor(debugTextColor);
	debugText.setPosition(sf::Vector2f(position_.x, position_.y - 25));
	debugText.setString(text_ + "0kb/s");
}

void DebugGraph::Update(float data)
{
	// New Data - Once per second
	//int randTemp = (rand() % 10) + 1;
	//debugBars_[lastBarIndex_].setSize(sf::Vector2f(debugBars_[lastBarIndex_].getSize().x, 25 + randTemp));
	//debugBars_[lastBarIndex_].setPosition(position_.x, position_.y);

	// TODO: packet loss = red bar

	//
	//sf::Time ticktime_ = sf::milliseconds(1000);
	//if (timeLastUpdated_ > ticktime_)
	//{
		//std::cout << clock.getElapsedTime().asSeconds() << std::endl;
	
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << data;
		std::string s = stream.str();
		debugText.setString(text_ + ": " + s + "kb/s");

		// Scroll
		for (int i = 0; i < (sizeof(debugBars_) / sizeof(*debugBars_)); i++)
		{
			// Bar reached the end
			if (debugBars_[i].getPosition().x > position_.x + (barSpaceOffset_ * numberOfBars_))
			{
				float percentage = 100 / (maxDataSize / data);
				debugBars_[i].setSize(sf::Vector2f(debugBars_[i].getSize().x, percentage));
				debugBars_[i].setPosition(position_.x, position_.y);
			}
			else
			{
				debugBars_[i].setPosition(debugBars_[i].getPosition().x + scrollSpeed_, position_.y);
			}
		}

		// Last Bar Index
		lastBarIndex_--;
		if (lastBarIndex_ < 0)
			lastBarIndex_ = numberOfBars_ - 1;


		//timeLastUpdated_ = clock.restart();
	//}
	//timeLastUpdated_ = clock.getElapsedTime();

}

void DebugGraph::Render(sf::RenderWindow &Window)
{
	Window.draw(debugText);
	for (int i = 0; i < (sizeof(debugBars_)/ sizeof(*debugBars_)); i++)
	{
		Window.draw(debugBars_[i]);
	}
}
