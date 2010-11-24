#include "includes.h"

#ifndef MTC_OALLOC_H
#define MTC_OALLOC_H

template <typename T>
class OrderedAllocationVector
{
private:
	size_t m_size;
	vector<T*> m_v;
	map<size_t,T*> m_map;
	list<size_t> m_emptyindexes;
public:
	typedef typename map<size_t,T*>::iterator iterator;
	iterator begin(){ return m_map.begin();}
	iterator end(){ return m_map.end();}
	T * at ( size_t n );
	size_t add ( T * x );
	void remove ( size_t n);
	OrderedAllocationVector(){
		m_size = 0;
	}
};

template <class T>
T * OrderedAllocationVector<T>::at ( size_t n ){
	return (m_v.at(n));
}

template <class T>
size_t OrderedAllocationVector<T>::add ( T * x ){
	if(m_emptyindexes.size()==0)
	{
		m_v.push_back(x);
		m_map.insert(pair<size_t,T *>(m_size,x));
		return m_size++;
	}else{
		size_t i = m_emptyindexes.back();
		m_map.insert(pair<size_t,T *>(i,x));
		m_v.assign(i, x);
		m_emptyindexes.pop_back();
		return i;
	}
}

template <class T>
void OrderedAllocationVector<T>::remove ( size_t n ){
	if(m_v.size()>n)
	{
		m_map.erase(n);
		m_v.assign(n,NULL);
		m_emptyindexes.push_back(n);
	}
}



#endif