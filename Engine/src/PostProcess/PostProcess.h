#pragma once

#include "Shader/Shader.h"

class PostProcess
{
public:
	void init();
	bool initShaders();
	bool initTextures();
	void update(float _dt);

	void draw(sf::RenderTexture & _src, sf::RenderTexture & _dst, bool _final);

	template <class T> class Param
	{
	public:
		Param(const T & _initValue, const char * _name);

		void set(const T & _value, float _duration = 0);
		inline const T & get() { return current; }
		void update(float _dt);
		void debugPrint(const T & _value);

	private:
		T current;
		T begin;
		T end;
		sf::Clock time;
		float duration;
		const char * name;
	};

private:
	ShaderID			m_shaderID;

public:
	Param<float>		m_saturation = Param(0.0f, "Saturation");
	Param<float>		m_blur = Param(0.0f, "Blur");
	Param<float>		m_pixelize = Param(0.0f, "Pixelize");
	Param<sf::Vector3f>	m_color = Param(sf::Vector3f(0.0f, 0.0f, 0.0f), "Color");

	sf::Texture			m_lut;
};