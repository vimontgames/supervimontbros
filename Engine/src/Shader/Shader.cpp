#include "Precomp.h"
#include "Shader.h"

std::vector<ShaderInfo> Shader::s_shaders;
ShaderID Shader::s_defaultShaderID = invalidShaderID;

//--------------------------------------------------------------------------
void Shader::init()
{
	s_defaultShaderID = add("Engine/data/shader/error_vs.fx", "Engine/data/shader/error_ps.fx", true);
}

//--------------------------------------------------------------------------
void Shader::deinit()
{
	for (auto & info : s_shaders)
	{
		SAFE_DELETE(info.shader);
		SAFE_FREE(info.vs);
		SAFE_FREE(info.ps);
	}
	s_shaders.clear();
}

//--------------------------------------------------------------------------
uint Shader::update()
{
	assert(invalidShaderID != s_defaultShaderID);
	auto * defaultShader = get(s_defaultShaderID);

	for (uint i = 0; i < s_shaders.size(); ++i)
	{
		auto & info = s_shaders[i];
		const bool isDefaultShader = (i == s_defaultShaderID);
		if (isDefaultShader || info.shader != defaultShader)
		{
			SAFE_DELETE(info.shader);
		}
		info.shader = compile(info.vs, info.ps, isDefaultShader);
	}

	return 0;
}

//--------------------------------------------------------------------------
ShaderID Shader::find(const char * _vs, const char * _ps)
{
	for (uint i = 0; i < s_shaders.size(); ++i)
	{
		auto & info = s_shaders[i];
		if (!_strcmpi(info.vs, _vs) && !_strcmpi(info.ps, _ps))
		{
			return i;
		}
	}

	return invalidShaderID;
}

//--------------------------------------------------------------------------
sf::Shader * Shader::compile(const char * _vs, const char * _ps, bool _default)
{
	sf::String header, vs, ps;

	fileToString(header, "Engine/data/shader/hlsl2glsl.h");
	fileToString(vs, _vs);
	fileToString(ps, _ps);

	if (_default)
	{
		vs = header + vs;
		ps = header + ps;
	}
	else
	{
		sf::String fx;
		fileToString(fx, "Engine/data/shader/fx.h");

		vs = header + fx + vs;
		ps = header + fx + ps;
	}

	sf::Shader * shader = new sf::Shader();
	
	if (shader->loadFromMemory(vs, ps))
	{
		return shader;
	}
	else
	{
		SAFE_DELETE(shader);
		dump("VS", _vs, vs.toAnsiString().c_str());
		dump("PS", _ps, ps.toAnsiString().c_str());

		assert(s_defaultShaderID != invalidShaderID);
		return s_shaders[s_defaultShaderID].shader;
	}
}

//--------------------------------------------------------------------------
ShaderID Shader::add(const char * _vs, const char * _ps, bool _default)
{
	ShaderID id = find(_vs, _ps);
	if (invalidShaderID != id)
	{
		return id;
	}

	ShaderInfo info;
			   info.vs = _strdup(_vs);
			   info.ps = _strdup(_ps);
			   info.shader = compile(_vs, _ps, _default);

	s_shaders.push_back(info);

	return (ShaderID)(s_shaders.size()-1);
}

//--------------------------------------------------------------------------
bool Shader::fileToString(sf::String & _string, const char * _filepath)
{
	FILE * fp = nullptr;

	errno_t err = fopen_s(&fp, _filepath, "r");
	if (0 != err)
	{
		debugPrint("Failed to open file \"%s\"\n", _filepath);
		_string = "";
		return false;
	}
	else
	{
		fseek(fp, 0, SEEK_END);
		uint size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char temp[4096];
		size_t read = fread_s(&temp[0], sizeof(temp), 1, size, fp);
		temp[read] = '\0';
		fclose(fp);

		_string = sf::String((char*)&temp[0]);
		return true;
	}
}

//--------------------------------------------------------------------------
void Shader::dump(const char * _type, const char * _path, const char * _src)
{
	debugPrint("\n%s = \"%s\"\n\n", _type, _path);

	const char * c = _src;
	char temp[1024];
	uint line = 0;

	sprintf_s(temp, "%s(%u): ", _path, line++);
	size_t index = strlen(temp);

	while (*c != '\0')
	{
		temp[index++] = *c;

		if (*c == '\n')
		{
			temp[index++] = '\0';
			debugPrint(temp);

			sprintf_s(temp, "%s(%u): ", _path, line++);
			index = strlen(temp);
		}

		++c;
	}
	temp[index++] = '\n';
	temp[index++] = '\0';
	debugPrint(temp);
}