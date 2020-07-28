#include "Precomp.h"
#include "PostProcess.h"
#include "Viewport/Viewport.h"

using namespace sf;

template class PostProcess::Param<float>;
template class PostProcess::Param<Vector3f>;

//--------------------------------------------------------------------------
template <class T> PostProcess::Param<T>::Param(const T & _initValue, const char * _name) :
	name(_name),
	current(_initValue),
	begin(_initValue),
	end(_initValue),
	duration(0.0f)
{
	time.restart();
}

//--------------------------------------------------------------------------
template <> void PostProcess::Param<float>::debugPrint(const float & _value)
{
	::debugPrint("PostProcess \"%s\" changed to %.2f\n", name, _value);
}

//--------------------------------------------------------------------------
template <> void PostProcess::Param<Vector3f>::debugPrint(const Vector3f & _value)
{
	::debugPrint("PostProcess \"%s\" changed to {%.2f, %.2f, %.2f}\n", name, _value.x, _value.y, _value.z);
}

//--------------------------------------------------------------------------
template <class T> void PostProcess::Param<T>::set(const T & _value, float _duration)
{
	//debugPrint(_value);

	if (_duration > 0.0f)
	{
		begin = current;
		end = _value;
	}
	else
	{
		current = begin = end = _value;
	}

	duration = _duration;
	time.restart();
}

//--------------------------------------------------------------------------
template <class T> void PostProcess::Param<T>::update(float _dt)
{
	const float t = time.getElapsedTime().asMilliseconds() / 1000.0f;
	if (duration > 0.0f && t <= duration)
	{
		const float f = clamp(t / duration, 0.0f, 1.0f);
		current = lerp(begin, end, f);
	}
	else
	{
		current = end;
	}
}

//--------------------------------------------------------------------------
void PostProcess::init()
{
	m_saturation.set(1.0f);
	m_blur.set(0.0f);
	m_color.set(Vector3f(1.0f, 1.0f, 1.0f));

	initShaders();
	initTextures();
}

//--------------------------------------------------------------------------
bool PostProcess::initShaders()
{
	m_shaderID = ::Shader::add("Engine/data/shader/postprocess_vs.fx", "Engine/data/shader/postprocess_ps.fx");
	return nullptr != ::Shader::get(m_shaderID);
}

//--------------------------------------------------------------------------
bool PostProcess::initTextures()
{
	bool result = true;
	result |= m_lut.loadFromFile("SuperVimontBros/data/img/lut/default.psd" /*"Engine/data/img/lut/default.psd"*/);
	m_lut.setSmooth(true);
	return result;
}

//--------------------------------------------------------------------------
void PostProcess::update(float _dt)
{
	m_saturation.update(_dt);
	m_blur.update(_dt);
	m_color.update(_dt);
	m_pixelize.update(_dt);
}

//--------------------------------------------------------------------------
void PostProcess::draw(RenderTexture & _src, RenderTexture & _dst, bool _final)
{
	auto & srcTex = (Texture&)_src.getTexture();
		   srcTex.setSmooth(true);

	Sprite quad(srcTex);

	sf::Shader * shader = ::Shader::get(m_shaderID);

	shader->setUniform("texture", sf::Shader::CurrentTexture);
	shader->setUniform("saturation", m_saturation.get());
	shader->setUniform("blur", m_blur.get());
	shader->setUniform("pixelize", m_pixelize.get());
	shader->setUniform("color", m_color.get());
	shader->setUniform("screenSize", (Vector2f)_src.getSize());
	shader->setUniform("lut", m_lut);
	shader->setUniform("final", _final);

	RenderStates rs;
				 rs.shader = shader;
				 rs.blendMode = { BlendMode::Factor::One, BlendMode::Factor::Zero, BlendMode::Add };

	_dst.draw(quad, rs);
}