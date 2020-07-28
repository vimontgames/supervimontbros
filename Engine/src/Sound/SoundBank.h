#pragma once

#include <SFML/Audio.hpp>

struct SoundBankEntry
{
	sf::String path;
	sf::SoundBuffer buffer;
};

struct SoundInstanceEntry
{
	sf::Sound * m_instance = nullptr;
	bool m_isAvailable = false;
};

class SoundBank
{
public:
	~SoundBank();

	static SoundBank & get() { return s_instance; }

	int addSound(const char * _path);

	const SoundBankEntry & getSound(int _index) const;

	sf::Sound * getSoundInstance();
	void releaseSoundInstance(sf::Sound *& _soundInstance);

private:
	static SoundBank s_instance;
	std::vector<SoundBankEntry>		m_entries;
	std::vector<SoundInstanceEntry>	m_soundInstances;
};