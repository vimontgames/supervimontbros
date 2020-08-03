#include "Precomp.h"
#include "MenuPlayers.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "PostProcess/PostProcess.h"
#include "Entity/Player/Player.h"
#include "Controller/Controller.h"

using namespace sf;

//--------------------------------------------------------------------------
void MenuPlayers::update(float _dt, GameStateData & _data)
{
	auto & game = SuperVimontBros::get();
	const uint maxPlayers = game.getMaxControllers();

	if (_data.m_entities.size() == 0)
	{
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

			game.m_selectedPlayerType[c] = { playerType, false, icon }; // default
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
	}
}

//--------------------------------------------------------------------------
void MenuPlayers::draw(sf::RenderTexture & _dst)
{
	auto & game = SuperVimontBros::get();

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
}

//--------------------------------------------------------------------------
void MenuPlayers::onMenuLeft(uint _player)
{
	auto & game = SuperVimontBros::get();
	auto & info = game.m_selectedPlayerType[_player];

	if (!info.enabled)
	{
		info.enabled = true;
		return;
	}

	if ((int)info.playerType >= 1)
		info.playerType = (PlayerType)((int)info.playerType - 1);
	else
		info.playerType = (PlayerType)((int)PlayerType::Count - 1);

	Player::setupPlayerAnimations(info.icon, info.playerType);
	info.icon->playAnimation(Animation::Icon);
}

//--------------------------------------------------------------------------
void MenuPlayers::onMenuRight(uint _player)
{
	auto & game = SuperVimontBros::get();
	auto & info = game.m_selectedPlayerType[_player];

	if (!info.enabled)
	{
		info.enabled = true;
		return;
	}

	if ((int)info.playerType == (int)PlayerType::Count - 1)
		info.playerType = (PlayerType)0;
	else
		info.playerType = (PlayerType)((int)info.playerType + 1);

	Player::setupPlayerAnimations(info.icon, info.playerType);
	info.icon->playAnimation(Animation::Icon);
}

//--------------------------------------------------------------------------
void MenuPlayers::onMenuAction(uint _player, unsigned int _button)
{
	auto & game = SuperVimontBros::get();
	auto & info = game.m_selectedPlayerType[_player];

	if (_button == 1 || _button == 2)
	{
		if (!info.enabled)
		{
			info.enabled = true;
		}
		else if (game.createPlayers())
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
	else if (_button == 3)
		game.m_menuTitle->setTitleMenu(MenuTitle::SubMenu::Main);
}