#ifndef BPSLAB_STR_H
#define BPSLAB_STR_H

#ifndef wchar
#define wchar wchar_t
#endif
#ifndef UT
#define UT(x) ((wchar*)(L ## x))
#define UC(x) ((wchar)(L ## x))
#endif

#include <string>
typedef std::string str;
typedef std::basic_string<wchar> wstr;

str ws2s(const wstr& ws);

extern const unsigned char g_table_upcase[256];

template<class ch>
inline uint32_t measure(const ch* p)
{
	const ch* t = p;
	while (*t)
		++t;
	return t - p;
}

template<class ch>
inline bool compare(const ch *p1, uint32_t size1, const ch *p2, uint32_t size2, bool case_sensitive = true)
{
	if (size1 != size2)
		return false;
	if (case_sensitive)
	{
		for (const ch *end = p1 + size1; p1 < end; ++p1, ++p2)
			if (*p1 != *p2)
				return false;
	}
	else
	{
		for (const ch *end = p1 + size1; p1 < end; ++p1, ++p2)
			if (g_table_upcase[static_cast<unsigned char>(*p1)] != g_table_upcase[static_cast<unsigned char>(*p2)])
				return false;
	}
	return true;
}

template<class ch>
inline bool compare(const ch* p1, const ch* p2, bool case_sensitive = true)
{
	return compare(p1, measure(p1), p2, measure(p2), case_sensitive);
}

#endif // BPSLAB_STR_H
