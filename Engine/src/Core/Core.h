#pragma once

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef unsigned int uint;

typedef sf::Vector2<i8>	Vector2i8;
typedef sf::Vector2<u8> Vector2u8;

template <typename T> inline T max(T _a, T _b)
{
	return _a > _b ? _a : _b;
}

template <typename T> inline T min(T _a, T _b)
{
	return _a < _b ? _a : _b;
}

template <typename T> inline T clamp(T _val, T _min, T _max)
{
	return min(max(_val, _min), _max);
}

template <typename T> inline T saturate(T _a)
{
	return clamp(_a, (T)0, (T)1);
}

template <typename T> inline T lerp(T _a, T _b, float _blend)
{
	return saturate(1.0f - _blend) * _a + saturate(_blend) * _b;
}

template <> inline sf::Vector2f min(sf::Vector2f _a, sf::Vector2f _b)
{
	return sf::Vector2f(_a.x < _b.x ? _a.x : _b.x, _a.y < _b.y ? _a.y : _b.y);
}

template <typename T> float length(const T & _v)
{
	return sqrtf(_v.x * _v.x + _v.y * _v.y);
}

template <typename T> T normalize(const T & _v)
{
	return _v/length(_v);
}

template <typename T> T sign(T _val)
{
	return _val >= 0 ? (T)+1 : (T)-1;
}

template <typename T> inline void exchange(T & _a, T & _b)
{
	T tmp = _a;
	_a = _b;
	_b = tmp;
}

inline float frand()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

void debugPrint(const char * _format, ...);

#define SAFE_FREE(p) { if (p) {free(p); p = nullptr; }}
#define SAFE_DELETE(p) { if (p) { delete p; p = nullptr; }}
#define SAFE_RELEASE(p) { if (p) { p->release(); p = nullptr; }}
#define SAFE_INCREASE_REFCOUNT(p) { if (p) { p->increaseRefCount(); }}

#define COUNT_OF(a) (sizeof(a)/sizeof(a[0]))

#ifndef _DEBUG
#define ENABLE_INL
#define INLINE inline
#else
#define INLINE
#endif

#define ENUM_FLAGS_OPERATORS(ENUMTYPE) \
    inline ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((std::underlying_type<ENUMTYPE>::type)a) | ((std::underlying_type<ENUMTYPE>::type)b)); } \
    inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((std::underlying_type<ENUMTYPE>::type &)a) |= ((std::underlying_type<ENUMTYPE>::type)b)); } \
    inline ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((std::underlying_type<ENUMTYPE>::type)a) & ((std::underlying_type<ENUMTYPE>::type)b)); } \
    inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((std::underlying_type<ENUMTYPE>::type &)a) &= ((std::underlying_type<ENUMTYPE>::type)b)); } \
    inline ENUMTYPE operator ~ (ENUMTYPE a) { return ENUMTYPE(~((std::underlying_type<ENUMTYPE>::type)a)); } \
    inline ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((std::underlying_type<ENUMTYPE>::type)a) ^ ((std::underlying_type<ENUMTYPE>::type)b)); } \
    inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((std::underlying_type<ENUMTYPE>::type &)a) ^= ((std::underlying_type<ENUMTYPE>::type)b)); } \
    inline bool asBool (ENUMTYPE a) { return 0 != (std::underlying_type<ENUMTYPE>::type)a; }
