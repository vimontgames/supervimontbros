#include "Precomp.h"
#include "MenuTitle.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "Entity/Player/Player.h"
#include "PostProcess/PostProcess.h"

using namespace sf;

//--------------------------------------------------------------------------
void MenuTitle::update(GameStateData & _data)
{
	auto & game = SuperVimontBros::get();

	const uint maxPlayers = game.getMaxControllers();

	if (_data.m_entities.size() == 0)
	{
		game.m_selectedPlayerType.clear();

		uint Y = (uint)(4 * title_fontSize);

		for (uint c = 0; c < 4; ++c)
		{
			PlayerType playerType = (PlayerType)min(c, (u32)PlayerType::Count - 1);

			Entity * icon = new Entity("PlayerIcon", game.m_spritesTile);
			Player::setupPlayerAnimations(icon, playerType);

			icon->setPosition(game.m_screenSize.x / 2.0f - 24, game.m_screenSize.y / 2.0f + Y + 5);
			icon->playAnimation(Animation::Icon);
			_data.m_entities.push_back(icon);

			if (c < maxPlayers)
				icon->setColor(title_disabledColor);
			else
				icon->setColor(title_unavailableColor);

			Y += (uint)(2 * title_fontSize);

			game.m_selectedPlayerType.push_back({ playerType, false, icon }); // default
		}
	}

	for (uint c = 0; c < maxPlayers; ++c)
	{
		auto & pad = Controller::getController(c);
		auto & info = game.m_selectedPlayerType[c];

		if (info.enabled)
			info.icon->setColor(title_enabledColor);
		else if (c < maxPlayers)
			info.icon->setColor(title_disabledColor);
		else
			info.icon->setColor(title_unavailableColor);

		if (info.keyDelay.getElapsedTime().asMilliseconds() < 200)
			continue;

		bool playerTypeChanged = false;

		const auto xAxis = pad.getXAxis();
		if (xAxis < -title_joyDeadZone)
		{
			if (!info.enabled)
			{
				info.enabled = true;
				info.keyDelay.restart();
				continue;
			}

			if ((int)info.playerType >= 1)
			{
				info.playerType = (PlayerType)((int)info.playerType - 1);
				playerTypeChanged = true;
			}
			else
			{
				info.playerType = (PlayerType)((int)PlayerType::Count - 1);
				playerTypeChanged = true;
			}
		}
		else if (xAxis > title_joyDeadZone)
		{
			if (!info.enabled)
			{
				info.enabled = true;
				info.keyDelay.restart();
				continue;
			}

			if ((int)info.playerType == (int)PlayerType::Count - 1)
			{
				info.playerType = (PlayerType)0;
				playerTypeChanged = true;
			}
			else
			{
				info.playerType = (PlayerType)((int)info.playerType + 1);
				playerTypeChanged = true;
			}
		}

		bool updated = false;

		if (playerTypeChanged)
		{
			Player::setupPlayerAnimations(info.icon, info.playerType);
			info.icon->playAnimation(Animation::Icon);
			updated = true;
		}

		const auto yAxis = pad.getYAxis();
		if (yAxis > title_joyDeadZone)
		{
			if (!info.enabled)
			{
				info.enabled = true;
			}

			menuDown();
			updated = true;
		}
		else if (yAxis < -title_joyDeadZone)
		{
			if (!info.enabled)
			{
				info.enabled = true;
			}

			menuUp();
			updated = true;
		}

		for (uint b = 0; b < Controller::getButtonCount(); ++b)
		{
			if (pad.isButtonJustPressed(b))
			{
				if (info.enabled == false)
				{
					info.enabled = true;
					updated = true;
				}
				else
				{
					menuButton(b);
					updated = true;
				}
			}
		}

		if (updated)
		{
			info.keyDelay.restart();
		}
	}
}

//--------------------------------------------------------------------------
void MenuTitle::draw(RenderTexture & _dst)
{
	auto & game = SuperVimontBros::get();

	RectangleShape title;
	title.setTexture(&m_title);

	auto size = (Vector2f)m_title.getSize();

	title.setPosition({ game.m_screenSize.x / 2.0f - size.x / 2.0f, game.m_screenSize.y * 1.0f / 4.0f - size.y / 2.0f });
	title.setFillColor({ 255,255,255,255 });
	title.setSize(size);

	_dst.draw(title);

	float Y = 0;

	Text text;
	text.setFont(game.getFont(GameFont::Dlx8));
	text.setCharacterSize((uint)title_fontSize);
	text.setFillColor(title_disabledColor);

	const char * menu[] =
	{
		"JOUEURS"
	};

	uint numPlayerReady = 0;
	for (const auto & info : game.m_selectedPlayerType)
	{
		if (info.enabled)
			numPlayerReady++;
	}

	for (uint i = 0; i < COUNT_OF(menu); ++i)
	{
		const char * msg = menu[i];
		char tmp[32];
		if (i == 0)
		{
			const uint maxPlayers = game.getMaxControllers();
			sprintf_s(tmp, "%u/%u PLAYER%s", numPlayerReady, maxPlayers, maxPlayers > 1 ? "S" : "");
			msg = tmp;
		}

		const uint len = strlen(msg);
		const uint offset = (len & 1) ? title_fontSize * len * 0.5f : title_fontSize * (len - 0.5f) * 0.5f;

		text.setPosition({ game.m_screenSize.x * 2.0f / 4.0f - offset, game.m_screenSize.y / 2.0f + Y });
		text.setString(msg);

		if (numPlayerReady || i != 0)
			text.setFillColor(title_enabledColor);
		else
			text.setFillColor(title_disabledColor);

		_dst.draw(text);

		Y += title_fontSize * 2;
	}

	Y += title_fontSize * 2;

	const uint maxPlayers = game.getMaxControllers();
	for (uint c = 0; c < 4; ++c)
	{
		const auto & info = game.m_selectedPlayerType[c];
		const auto & playerInfo = PlayerTypeInfo::get(info.playerType);

		char temp[64];
		sprintf_s(temp, "J%u  %s", c + 1, playerInfo.name);

		const uint len = 12;// strlen(temp);
		const uint offset = (len & 1) ? title_fontSize * len * 0.5f : title_fontSize * (len - 0.5f) * 0.5f;

		text.setPosition({ game.m_screenSize.x * 2.0f / 4.0f - offset, game.m_screenSize.y / 2.0f + Y });
		text.setString(temp);

		if (info.enabled)
			text.setFillColor(title_enabledColor);
		else if (c < maxPlayers)
			text.setFillColor(title_disabledColor);
		else
			text.setFillColor(title_unavailableColor);

		_dst.draw(text);

		Y += title_fontSize * 2;
	}

	// Version
	{
		const char * version = "1.0";
		const uint len = strlen(version);
		const uint offset = (len & 1) ? title_fontSize * len * 0.5f : title_fontSize * (len - 0.5f) * 0.5f;

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
		const uint len = strlen(copyright);
		const uint offset = (len & 1) ? title_fontSize * len * 0.5f : title_fontSize * (len - 0.5f) * 0.5f;

		text.setPosition({ game.m_screenSize.x * 2.0f / 4.0f - offset, game.m_screenSize.y - title_fontSize * 4 });
		text.setString(copyright);
		text.setFillColor(title_disabledColor);
		_dst.draw(text);
	}

	// Jahtari
	{
		const char * copyright = "Music by JAHTARI";
		const uint len = strlen(copyright);
		const uint offset = (len & 1) ? title_fontSize * len * 0.5f : title_fontSize * (len - 0.5f) * 0.5f;

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
void MenuTitle::menuUp()
{
	auto & game = SuperVimontBros::get();

	if (m_titleMenu == (TitleMenu)0)
	{
		//m_titleMenu = (TitleMenu)((i32)TitleMenu::Count - 1);
	}
	else
	{
		m_titleMenu = (TitleMenu)((i32)m_titleMenu - 1);
	}
	game.setKeyPressed();
}

//--------------------------------------------------------------------------
void MenuTitle::menuDown()
{
	auto & game = SuperVimontBros::get();

	if (m_titleMenu == (TitleMenu)((i32)TitleMenu::Count - 1))
	{
		//m_titleMenu = (TitleMenu)0;
	}
	else
	{
		m_titleMenu = (TitleMenu)((i32)m_titleMenu + 1);
	}
	game.setKeyPressed();
}

//--------------------------------------------------------------------------
void MenuTitle::menuButton(unsigned int _button)
{
	auto & game = SuperVimontBros::get();

	if (game.m_gameState == GameState::Title)
	{
		switch (m_titleMenu)
		{
		case TitleMenu::Play:
		{
			if (game.createPlayers())
			{
				game.m_gameState = GameState::Play;
				game.m_editorMode = EditorMode::None;

				const float delay = 1.5f;

				// Intro
				game.m_postProcess->m_color.set({ 1.0f, 1.0f, 1.0f }, delay);
				game.m_postProcess->m_blur.set(0.0f, delay);
				game.m_postProcess->m_pixelize.set(0, delay);
				game.m_postProcess->m_saturation.set(1.0f, delay);
			}
		}
		break;
		}
	}
}
