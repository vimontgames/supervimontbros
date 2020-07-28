#pragma once

class Object
{
public:
	Object(){}
	Object(const sf::String & _name);
	virtual ~Object();

	uint increaseRefCount();
	virtual void release();

	const sf::String & name() const { return m_name;  }

	static uint getObjectCount() { return s_count; }

private:
	static uint s_count;

	sf::String	m_name;
	u32			m_refCount = 1;
};

template <class T> class ObjectList
{
public:
	void add(T _object)
	{
		m_vector.push_back(_object);
		_object->increaseRefCount();
	}

	void remove(T _object)
	{
		for (uint i = 0; i < size(); ++i)
		{
			if (m_vector[i] == _object)
			{
				m_vector.erase(m_vector.begin() + i);
				break;
			}
		}
		_object->release();
	}
	
	void clear()
	{
		const size_t count = size();
		for (size_t i = 0; i < count; ++i)
		{
			SAFE_RELEASE(m_vector[i]);
		}
		m_vector.clear();
	}

	uint size() const
	{
		return (uint)m_vector.size();
	}

	T & operator [] (int _index)
	{
		return m_vector[_index];
	}

	const T & operator [] (int _index) const
	{
		return m_vector[_index];
	}

	void update(const float _dt)
	{
		const uint count = this->size();
		for (uint i = 0; i < count; ++i)
		{
			T  object = m_vector[i];
			object->update(_dt);
		}
	}

	const std::vector<T> & get() const
	{
		return m_vector;
	}

protected:
	std::vector<T> m_vector;
};