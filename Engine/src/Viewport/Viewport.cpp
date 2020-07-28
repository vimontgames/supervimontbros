#include "Precomp.h"
#include "Viewport/Viewport.h"
#include "Visual/Visual.h"
#include "Tiles/Tiles.h"
#include "PostProcess/PostProcess.h"
#include "RenderTarget/RenderTarget.h"

#ifndef ENABLE_INL
#include "Viewport.inl"
#endif

using namespace sf;

//--------------------------------------------------------------------------
Viewport::Viewport(const char * _name)
{
	m_name = _strdup(_name);
	m_postProcess = new PostProcess();
}

//--------------------------------------------------------------------------
Viewport::~Viewport()
{
	SAFE_FREE(m_name); 
	SAFE_DELETE(m_postProcess);
}

//--------------------------------------------------------------------------
void Viewport::init(u32 _width, u32 _height, i32 _offsetX, i32 _offsetY)
{
	m_screenPos = { _offsetX, _offsetY };
	m_camOffset = { _width / 2.0f, _height / 2.0f };

	//m_surface.create(_width, _height);
	//m_view = m_surface.getDefaultView();
	m_view.setSize((float)_width, (float)_height);
	m_view.setCenter(m_camOffset);

	m_size = { _width, _height };

	m_postProcess->init();

	reset();
}

//--------------------------------------------------------------------------
void Viewport::reset()
{
	m_camOffset = (Vector2f)m_size * 0.5f;
	m_view.setCenter(m_camOffset);
}

//--------------------------------------------------------------------------
void Viewport::update(bool _addVisuals, const ObjectList<Visual*> & _visuals, bool _addDecals, const ObjectList<Visual*> _decals)
{
	// Reset list of visuals to sort front-to-back
	m_visualsToSort.clear();
	m_decalsToSort.clear();

	const auto center = getCameraOffset();
	const auto size = getSize();

	AABB viewportAABB((Vector2f)center - (Vector2f)size/2.0f, { 0, 0, (int)size.x, (int)size.y });

	if (_addVisuals)
	{
		for (u32 i = 0; i < _visuals.size(); ++i)
		{
			auto * visual = _visuals[i];
			if (visual->m_visibilityAABB.intersects(viewportAABB))
				m_visualsToSort.push_back(visual);
		}
	}

	if (_addDecals)
	{
		for (u32 i = 0; i < _decals.size(); ++i)
		{
			auto * decal = _decals[i];
			if (decal->m_visibilityAABB.intersects(viewportAABB))
				m_decalsToSort.push_back(decal);
		}
	}
}

//--------------------------------------------------------------------------
void Viewport::begin()
{
	const auto center = getCameraOffset();
	const auto size = getSize();

	m_view.setCenter(center);
	m_view.setSize((Vector2f)size);

	m_surface = RenderTargetManager::get(RenderTargetDesc(m_size.x, m_size.y));
	m_surface->setView(m_view);
	m_surface->clear(Color(255, 0, 255, 255));
}

//--------------------------------------------------------------------------
void Viewport::end()
{
	RenderTargetManager::release(m_surface);
}

//--------------------------------------------------------------------------
bool Viewport::isVisible(const Visual * _visual) const
{
	if (_visual->isImageSet())
		return isVisible(_visual->getPosition(), _visual->m_tileSet->m_tileInfos.get(_visual->m_coords));
	else
		return false;
}

//--------------------------------------------------------------------------
bool Viewport::isVisible(sf::Vector2f _point, const TileInfo & _tileInfo) const
{
	float x = (float)_tileInfo.m_box.left;
	float y = (float)_tileInfo.m_box.top;

	float X = x + _tileInfo.m_box.width;
	float Y = y + _tileInfo.m_box.height + _tileInfo.m_box.height / 2 + _tileInfo.m_shadowOffset;

	return isVisible(_point, x, y, X, Y);
}

//--------------------------------------------------------------------------
bool Viewport::isVisible(sf::Vector2f _point, float x, float y, float X, float Y) const
{
	return (_point.x + X) > getMinScreenCorner().x && (_point.y + Y) > getMinScreenCorner().y && (_point.x + x) < getMaxScreenCorner().x && (_point.y + y) < getMaxScreenCorner().y;
}

