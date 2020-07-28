#pragma once

struct Message
{
	Message(const char * _txt, int _fontSize, sf::Color _color, int _x, int _y, sf::Time _time)
	{
		txt = _txt;
		fontSize = _fontSize;
		time = _time;
		color = _color;
		x = _x;
		y = _y;
	}

	const char * txt = nullptr;
	sf::Time time;
	int fontSize;
	sf::Color color;
	int x;
	int y;
	sf::Clock timer;
};

class MessageList
{
public:
	void print(int _milliseconds, int _fontSize, sf::Color _color, int _x, int _y, const char * _format, ...);

	void update();
	void draw(sf::RenderTexture & _surface);
	void clear();

private:
	std::vector<Message> m_messages;
};