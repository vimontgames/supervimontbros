#include "Precomp.h"
#include "MessageList.h"
#include "Game/Game.h"

using namespace sf;

//--------------------------------------------------------------------------
void MessageList::clear()
{
	for (u32 i = 0; i < m_messages.size(); ++i)
	{
		Message & msg = m_messages[i];
		free((void*)msg.txt);
	}
	m_messages.clear();
}

//--------------------------------------------------------------------------
void MessageList::print(int _milliseconds, int _fontSize, sf::Color _color, int _x, int _y, const char * _format, ...)
{
	char buffer[256];
	va_list args;
	va_start(args, _format);
	vsnprintf(buffer, sizeof(buffer), _format, args);
	perror(buffer);
	va_end(args);

	m_messages.push_back(Message(_strdup(buffer), _fontSize, _color, _x, _y, sf::milliseconds(_milliseconds)));

}
//--------------------------------------------------------------------------
void MessageList::update()
{
	auto messages = std::move(m_messages);

	for (u32 i = 0; i < messages.size(); ++i)
	{
		Message & msg = messages[i];

		if (msg.time.asMilliseconds() < 0 || msg.timer.getElapsedTime() < msg.time)
		{
			m_messages.push_back(msg);
		}
		else
		{
			free((void*)msg.txt);
		}
	}
}

//--------------------------------------------------------------------------
void MessageList::draw(sf::RenderTexture & _surface)
{
	Game & game = Game::get();

	float Y = 0;

	for (u32 i = 0; i < m_messages.size(); ++i)
	{
		const Message & msg = m_messages[i];
		
		int size = msg.fontSize;
		u32 opacity = 255;

		const float title_fontSize = msg.timer.getElapsedTime().asSeconds();
		const float t = msg.time.asSeconds();

		if (title_fontSize < 1.0f)
		{
			size *= title_fontSize;
			opacity *= title_fontSize;
		}
		else if (t > 0 && title_fontSize > msg.time.asSeconds() - 1.0f)
		{
			opacity = 255 - 255 * (title_fontSize - (t - 1.0f));
		}

		Text debugText;
			 debugText.setString(msg.txt);
			 debugText.setFont(game.getFont(GameFont::Dlx8));
			 debugText.setCharacterSize(size);
			 debugText.setPosition(msg.x, msg.y);
			 debugText.setFillColor({ msg.color.r, msg.color.g, msg.color.b, (u8)((msg.color.a * (u8)opacity) / 255) });
			 debugText.setOrigin({ strlen(msg.txt)*0.5f*size, size * 0.5f });

		Y += msg.fontSize;

		_surface.draw(debugText);
	}
}