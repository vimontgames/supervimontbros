#include "Precomp.h"
#include "RenderTarget.h"

using namespace sf;

std::vector<RenderTargetEntry> RenderTargetManager::s_renderTextures;

//--------------------------------------------------------------------------
RenderTargetEntry::RenderTargetEntry(const RenderTargetDesc & _desc) :
	desc(_desc),
	available(false)
{
	surface = new sf::RenderTexture();
	surface->create(_desc.x, _desc.y);
}

//--------------------------------------------------------------------------
void RenderTargetManager::init()
{

}

//--------------------------------------------------------------------------
void RenderTargetManager::deinit()
{
	for (auto & entry : s_renderTextures)
	{
		SAFE_DELETE(entry.surface);
	}
	s_renderTextures.clear();
}

//--------------------------------------------------------------------------
sf::RenderTexture * RenderTargetManager::get(const RenderTargetDesc & _desc)
{
	for (auto & entry : s_renderTextures)
	{
		if (entry.available && entry.desc == _desc)
		{
			entry.available = false;
			entry.surface->setSmooth(false);
			entry.surface->setRepeated(false);
			return entry.surface;
		}
	}

	s_renderTextures.emplace_back(_desc);
	return s_renderTextures.back().surface;
}

//--------------------------------------------------------------------------
void RenderTargetManager::release(sf::RenderTexture *& _renderTexture)
{
	for (auto & entry : s_renderTextures)
	{
		if (entry.surface == _renderTexture)
		{
			entry.available = true;
			break;
		}
	}
	
	_renderTexture = nullptr;
}