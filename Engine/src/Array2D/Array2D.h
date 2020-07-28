#pragma once

template <class T> class Array2D
{
public:
	bool resize(const sf::Vector2u & _count)
	{
		if (_count != m_size)
		{
			if (nullptr != m_data)
			{
				free(m_data);
				m_data = nullptr;
			}

			const uint size = sizeof(T) * _count.x * _count.y;
			m_data = (T*)malloc(size);
			m_size = _count;
			
			clear();

			return true;
		}

		return false;
	}

	void clear()
	{
		const uint count = m_size.x * m_size.y;
		if (count)
		{
			if (__has_trivial_constructor(T))
			{
				memset(m_data, 0, count * sizeof(T));
			}
			else
			{
				for (uint i = 0; i < count; ++i)
					m_data[i] = T();
			}
		}
	}

	inline void set(const sf::Vector2u & _coords, const T & _val)
	{
		assert(_coords.x < m_size.x && _coords.y < m_size.y);
		m_data[_coords.y * m_size.x + _coords.x] = _val;
	}

	inline const T & get(const sf::Vector2u & _coords) const
	{
		assert(_coords.x < m_size.x && _coords.y < m_size.y);
		return m_data[_coords.y * m_size.x + _coords.x];
	}

	inline const T & get(uint _index) const
	{
		assert(_index < m_size.x * m_size.y);
		return m_data[_index];
	}

	inline T & get(uint _index)
	{
		assert(_index < m_size.x * m_size.y);
		return m_data[_index];
	}

	inline T & get(const sf::Vector2u & _coords)
	{
		assert(_coords.x < m_size.x && _coords.y < m_size.y);
		return m_data[_coords.y * m_size.x + _coords.x];
	}

	inline T & get(uint _x, uint _y)
	{
		assert(_x < m_size.x && _y < m_size.y);
		return m_data[_y * m_size.x + _x];
	}

	inline sf::Vector2u size() const
	{
		return m_size;
	}

	uint read(FILE * _fp, const char * _debugName, bool _resizeToFile = false)
	{
		uint read = 0;

		sf::Vector2u size;
		read += (uint)fread(&size, sizeof(size), 1, _fp);

		if (_resizeToFile || (size == m_size))
		{
			m_size = size;
			resize(m_size);
			read += (uint)fread(m_data, sizeof(T) * m_size.x * m_size.y, 1, _fp);
		}
		else
		{
			debugPrint("Dimensions of Array \"%s\" have changed from {%u,%u} to {%u,%u}\n", _debugName, size.x, size.y, m_size.x, m_size.y);

			Array2D<T> tempArray;
					   tempArray.resize(size);
			read += (uint)fread(tempArray.m_data, sizeof(T) * size.x * size.y, 1, _fp);

			const uint width = min(m_size.x, size.x);
			const uint height = min(m_size.y, size.y);

			clear();

			for (uint j = 0; j < m_size.y; ++j)
			{
				for (uint i = 0; i < m_size.x; ++i)
				{
					if (i < size.x && j < size.y)
					{
						set({ i, j }, tempArray.get({ i, j }));
					}
					else
					{
						T empty;
						set({ i,j }, empty);
					}
				}
			}
		}	

		return read;
	}

	uint write(FILE * _fp)
	{
		uint write = 0;

		write += (uint)fwrite(&m_size, sizeof(m_size), 1, _fp);
		write += (uint)fwrite(m_data, sizeof(T) * m_size.x * m_size.y, 1, _fp);

		return write;
	}

private:
	sf::Vector2u	m_size;
	T *				m_data = nullptr;
};
