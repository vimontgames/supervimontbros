#include "Precomp.h"
#include "SoundBank.h"

using namespace sf;

SoundBank SoundBank::s_instance;

//--------------------------------------------------------------------------
SoundBank::~SoundBank()
{
	for (auto & entry : m_soundInstances)
	{
		SAFE_DELETE(entry.m_instance);
	}
	m_soundInstances.clear();
}

//--------------------------------------------------------------------------
sf::Sound * SoundBank::getSoundInstance()
{
	for (auto & entry : m_soundInstances)
	{
		if (entry.m_isAvailable)
		{
			entry.m_isAvailable = false;
			return entry.m_instance;
		}
	}

	m_soundInstances.push_back({new sf::Sound(), false});
	return m_soundInstances[m_soundInstances.size() - 1].m_instance;
}

//--------------------------------------------------------------------------
void SoundBank::releaseSoundInstance(sf::Sound *& _soundInstance)
{
	for (auto & entry : m_soundInstances)
	{
		if (entry.m_instance == _soundInstance)
		{
			_soundInstance->stop();
			_soundInstance = nullptr;

			entry.m_isAvailable = true;

			return;
		}
	}
}

//--------------------------------------------------------------------------
int SoundBank::addSound(const char * _path)
{
	uint size = (uint)m_entries.size();

	for (uint i = 0; i < size; ++i)
	{
		const auto & e = m_entries[i];

		if (!strcmp(e.path.toAnsiString().c_str(), _path))
		{
			return i;
		}
	}

	SoundBankEntry entry;
	entry.path = _path;

	bool result = entry.buffer.loadFromFile(_path); // ie. "SuperVimontBros/data/sound/Grrr.wav"
	
	if (result)
	{
		m_entries.push_back(std::move(entry));
		return size;
	}
	else
	{
		return -1;
	}
}

//--------------------------------------------------------------------------
const SoundBankEntry & SoundBank::getSound(int _index) const
{
	return m_entries[_index];
}