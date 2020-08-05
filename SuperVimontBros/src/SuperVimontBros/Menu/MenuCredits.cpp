#include "Precomp.h"
#include "MenuCredits.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "MenuTitle.h"

using namespace sf;

//--------------------------------------------------------------------------
void MenuCredits::onEnterMenu()
{
	m_timeStart = 0.0f;
}

//--------------------------------------------------------------------------
void MenuCredits::update(float _dt, GameStateData & _data)
{
	auto & game = SuperVimontBros::get();

	m_timeStart += _dt;

	if (m_timeStart >= 50000)
		game.m_menuTitle->setTitleMenu(MenuTitle::SubMenu::Main);
}

//--------------------------------------------------------------------------
void MenuCredits::draw(sf::RenderTexture & _dst)
{
	_dst.clear(Color(0, 0, 0, 128));

	auto & game = SuperVimontBros::get();

	float offset = -m_timeStart * 0.0025f;
	int Y = (int)((offset+22) * title_fontSize);

	Text text;
		 text.setFont(game.getFont(GameFont::Dlx8));
		 text.setCharacterSize((uint)title_fontSize);
		 text.setFillColor(title_disabledColor);

	const char * lines[] =
	{
		"SUPER VIMONT BROS",
		"",
		"",
		"",
		"Lead Game Designer / Art Director",
		"ROMEO VIMONT",
		"",
		"",
		"Principal Associate Lead Game Designer",
		"PABLO VIMONT",
		"",
		"",
		"Chief technical Officer",
		"BENOIT VIMONT",
		"",
		"",
		"Associate Programming International Consultant",
		"BENJAMIN PRUNEVIELLE",
		"",
		"",
		"Chief Workplace Manager",
		"MARIE VIALEFONT",
		"",
		"",
		"",
		"using SFML",
		"distributed under the zlib / png license",
		"",
		"",
		"music by Jahtari",
		"distributed under Creative Commons BY-NC-ND 2.0 licence",
		"",
		"",
		"",
		"Special Thanks",
		"NINI",
		"YVAN",
		"",
		"PATOCHE",
		"",
		""
	};

	for (uint i = 0; i < COUNT_OF(lines); ++i)
	{
		const uint len = strlen(lines[i]);
		const uint offset = (len & 1) ? title_fontSize * len * 0.5f : title_fontSize * (len - 0.5f) * 0.5f;

		text.setPosition({ game.m_screenSize.x * 2.0f / 4.0f - offset, game.m_screenSize.y / 2.0f + Y });
		text.setString(lines[i]);
		text.setFillColor(title_enabledColor);

		_dst.draw(text);

		Y += title_fontSize * 2;
	}
}

//--------------------------------------------------------------------------
void MenuCredits::onMenuAction(uint _player, unsigned int _button)
{
	auto & game = SuperVimontBros::get();
	auto & info = game.m_selectedPlayerType[_player];

	if (!info.enabled)
	{
		info.enabled = true;
	}
	else if (_button == 1 || _button == 2)
	{

	}
	else if (_button == 3)
		game.m_menuTitle->setTitleMenu(MenuTitle::SubMenu::Main);
}
