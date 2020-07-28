#pragma once

#include "Core/Core.h"

struct RenderTargetDesc
{
	RenderTargetDesc(u16 _x, u16 _y) :
		x(_x),
		y(_y)
	{

	}

	bool operator == (const RenderTargetDesc & _other)
	{
		return _other.x == x && _other.y == y;
	}

	u16 x;
	u16 y;
};

struct RenderTargetEntry
{
	RenderTargetEntry(const RenderTargetDesc & _desc);

	sf::RenderTexture * surface;
	RenderTargetDesc desc;
	bool available;
};

class RenderTargetManager
{
public:
	static void init();
	static void deinit();

	static sf::RenderTexture * get(const RenderTargetDesc & _desc);
	static void release(sf::RenderTexture *& _renderTexture);

private:
	static std::vector<RenderTargetEntry> s_renderTextures;
};