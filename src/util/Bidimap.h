#ifndef BIDIMAP_H
#define BIDIMAP_H
#include <algorithm>

namespace Darling
{

template<typename T, size_t N> class Bidimap
{
public:
	Bidimap(T aToB[N], T bToA[N])
		: m_aToB(aToB), m_bToA(bToA)
	{
		std::fill(&aToB[0], &aToB[N], 0);
		std::fill(&bToA[0], &bToA[N], 0);
	}
	void put(T a, T b)
	{
		m_aToB[a] = b;
		m_bToA[b] = a;
	}
private:
	T *m_aToB, *m_bToA;
};

}


#endif

