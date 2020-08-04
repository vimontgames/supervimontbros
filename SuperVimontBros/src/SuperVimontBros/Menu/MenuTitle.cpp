#include "Precomp.h"
#include "MenuTitle.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "PostProcess/PostProcess.h"
#include "Controller/Controller.h"
#include "MenuPlayers.h"
#include "MenuCredits.h"

using namespace sf;

//--------------------------------------------------------------------------
MenuTitle::MenuTitle()
{
	m_subMenu[(uint)SubMenu::Players] = new MenuPlayers();
	m_subMenu[(uint)SubMenu::Credits] = new MenuCredits();
}

//--------------------------------------------------------------------------
MenuTitle::~MenuTitle()
{
	for (uint i = 0; i < COUNT_OF(m_subMenu); ++i)
		SAFE_DELETE(m_subMenu[i]);
}

//--------------------------------------------------------------------------
void MenuTitle::init(bool _reinit)
{
	m_title.loadFromFile("SuperVimontBros/data/img/supervimontbros.psd");
}

//--------------------------------------------------------------------------
void MenuTitle::setTitleMenu(SubMenu _menu)
{
	auto & game = SuperVimontBros::get();
	game.deleteGameStateData(GameState::Title);

	switch (_menu)
	{
		case SubMenu::Main:
			if (m_selected)
			{
				m_selected = nullptr;
				onEnterMenu();
				//m_titleMenu = SubMenu::Players;
			}
			break;

		default:
			if (m_selected != m_subMenu[(uint)_menu])
			{
				m_selected = m_subMenu[(uint)_menu];
				m_selected->onEnterMenu();
				//m_titleMenu = _menu;
			}
			break;
	}
}

//--------------------------------------------------------------------------
void MenuTitle::update(float _dt, GameStateData & _data)
{
	auto & game = SuperVimontBros::get();

	const uint maxPlayers = game.getMaxControllers();
	for (uint c = 0; c < maxPlayers; ++c)
	{
		auto & pad = Controller::getController(c);
		auto & info = game.m_selectedPlayerType[c];

		if (info.keyDelay.getElapsedTime().asMilliseconds() < 200)
			continue;

		for (uint b = 0; b < Controller::getButtonCount(); ++b)
		{
			if (pad.isButtonJustPressed(b))
				onMenuAction(c, b);
		}

		const auto xAxis = pad.getXAxis();
		if (xAxis < -title_joyDeadZone)
			onMenuLeft(c);
		else if (xAxis > title_joyDeadZone)
			onMenuRight(c);

		const auto yAxis = pad.getYAxis();
		if (yAxis > title_joyDeadZone)
			onMenuDown(c);
		else if (yAxis < -title_joyDeadZone)
			onMenuUp(c);
	}

	if (m_selected)
		m_selected->update(_dt, _data);
	else
	{

	}
	
}

//--------------------------------------------------------------------------
void MenuTitle::drawTitleLogo(RenderTexture & _dst)
{
	auto & game = SuperVimontBros::get();

	RectangleShape title;
				   title.setTexture(&m_title);

	auto size = (Vector2f)m_title.getSize();

	title.setPosition({ game.m_screenSize.x / 2.0f - size.x / 2.0f, game.m_screenSize.y * 1.0f / 4.0f - size.y / 2.0f });
	title.setFillColor({ 255,255,255,255 });
	title.setSize(size);

	_dst.draw(title);

	Text text;
		 text.setFont(game.getFont(GameFont::Dlx8));
		 text.setCharacterSize((uint)title_fontSize);
		 text.setFillColor(title_disabledColor);

	// Version
	{
		const char * version = "1.01";
		const uint len = (uint)strlen(version);
		const uint offset = (uint)((len & 1) ? title_fontSize * len * 0.5f : title_fontSize * (len - 0.5f) * 0.5f);

		Vector2f pos = title.getPosition() + Vector2f{ title.getLocalBounds().width, 0 * title.getLocalBounds().height };

		pos.x = (float)(int(pos.x) - int(pos.x) % (int)title_fontSize);
		pos.y = (float)(int(pos.y) - int(pos.y) % (int)title_fontSize);

		text.setPosition(pos);
		text.setString(version);
		text.setFillColor(title_disabledColor);
		_dst.draw(text);
	}

	// VimontGames
	{
		const char * copyright = "VimontGames 2019-2020";
		const uint len = (uint)strlen(copyright);
		const uint offset = (uint)((len & 1) ? title_fontSize * len * 0.5f : title_fontSize * (len - 0.5f) * 0.5f);

		text.setPosition({ game.m_screenSize.x * 2.0f / 4.0f - offset, game.m_screenSize.y - title_fontSize * 4 });
		text.setString(copyright);
		text.setFillColor(title_disabledColor);
		_dst.draw(text);
	}

	// Jahtari
	{
		const char * copyright = "Music by JAHTARI";
		const uint len = (uint)strlen(copyright);
		const uint offset = (uint)((len & 1) ? title_fontSize * len * 0.5f : title_fontSize * (len - 0.5f) * 0.5f);

		Vector2f pos = { game.m_screenSize.x * 2.0f / 4.0f - offset, game.m_screenSize.y - title_fontSize * 2 };
		text.setPosition(pos);
		text.setString("Music by");
		text.setFillColor(title_disabledColor);
		_dst.draw(text);

		text.setPosition(text.getPosition() + Vector2f(strlen("Music by ") * title_fontSize, 0.0f));
		text.setString("JAH");
		text.setFillColor({ 207,0,1,(255) });
		_dst.draw(text);

		text.setPosition(text.getPosition() + Vector2f(strlen("JAH") * title_fontSize, 0.0f));
		text.setString("TA");
		text.setFillColor({ 253,203,52,(255) });
		_dst.draw(text);

		text.setPosition(text.getPosition() + Vector2f(strlen("TA") * title_fontSize, 0.0f));
		text.setString("RI");
		text.setFillColor({ 0,153,1,(255) });
		_dst.draw(text);
	}
}

//--------------------------------------------------------------------------
void MenuTitle::draw(RenderTexture & _dst)
{
	auto & game = SuperVimontBros::get();

	drawTitleLogo(_dst);

	if (m_selected)
		m_selected->draw(_dst);
	else
	{
		Text text;
		text.setFont(game.getFont(GameFont::Dlx8));
		text.setCharacterSize((uint)title_fontSize);
		text.setFillColor(title_disabledColor);

		float Y = 0;

		Y += title_fontSize * 4;

		const char * menu[] =
		{
			nullptr,

			"JOUER",
			"CREDITS"
		};

		for (uint i = 1; i < (uint)SubMenu::Count; ++i)
		{
			const char * msg = menu[i];

			const uint len = (uint)strlen(msg);
			const uint offset = (uint)((len & 1) ? title_fontSize * len * 0.5f : title_fontSize * (len - 0.5f) * 0.5f);

			text.setPosition({ game.m_screenSize.x * 2.0f / 4.0f - offset, game.m_screenSize.y / 2.0f + Y });
			text.setString(msg);

			if (i == (uint)m_titleMenu)
				text.setFillColor(title_enabledColor);
			else
				text.setFillColor(title_disabledColor);

			_dst.draw(text);

			Y += title_fontSize * 2;
		}

		Y += title_fontSize * 2;
	}
}

//--------------------------------------------------------------------------
void MenuTitle::onMenuUp(uint _player)
{
	auto & game = SuperVimontBros::get();
	auto & info = game.m_selectedPlayerType[_player];

	if (m_selected)
		m_selected->onMenuLeft(_player);
	else
	{
		if (m_titleMenu != (SubMenu)1)
			m_titleMenu = (SubMenu)((i32)m_titleMenu - 1);
	}

	game.setKeyPressed();
	info.keyDelay.restart();
}

//--------------------------------------------------------------------------
void MenuTitle::onMenuDown(uint _player)
{
	auto & game = SuperVimontBros::get();
	auto & info = game.m_selectedPlayerType[_player];

	if (m_selected)
		m_selected->onMenuLeft(_player);
	else
	{
		if (m_titleMenu != (SubMenu)((i32)SubMenu::Count - 1))
			m_titleMenu = (SubMenu)((i32)m_titleMenu + 1);
	}

	game.setKeyPressed();
	info.keyDelay.restart();
}

//--------------------------------------------------------------------------
void MenuTitle::onMenuLeft(uint _player)
{
	auto & game = SuperVimontBros::get();
	auto & info = game.m_selectedPlayerType[_player];

	if (m_selected)
		m_selected->onMenuLeft(_player);
	else
	{

	}

	game.setKeyPressed();
	info.keyDelay.restart();
}

//--------------------------------------------------------------------------
void MenuTitle::onMenuRight(uint _player)
{
	auto & game = SuperVimontBros::get();
	auto & info = game.m_selectedPlayerType[_player];

	if (m_selected)
		m_selected->onMenuRight(_player);
	else
	{

	}

	game.setKeyPressed();
	info.keyDelay.restart();
}

//--------------------------------------------------------------------------
void MenuTitle::onMenuAction(uint _player, unsigned int _button)
{
	auto & game = SuperVimontBros::get();
	auto & info = game.m_selectedPlayerType[_player];

	if (m_selected)
		m_selected->onMenuAction(_player, _button);
	else
	{
		if (_button == 1 || _button == 2)
			setTitleMenu(m_titleMenu);
	}

	game.setKeyPressed();
	info.keyDelay.restart();
}
