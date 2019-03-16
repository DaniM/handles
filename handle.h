#ifndef __HANDLE_H__
#define __HANDLE_H__

#include <stdint.h>
#include <vector>
#include <algorithm>

namespace DaniM
{
namespace MemoryManagement
{

struct HANDLE
{
public:
	typedef uint32_t HANDLE_TYPE;
	typedef uint16_t ID;

	enum
	{
		MASK_SIZE = 16,
		ID_MASK = 0xFFFF0000
	};


	union
	{
		struct
		{
			uint16_t idx;
			uint16_t id;
		};
		uint32_t handle;
	};

	operator uint32_t() { return handle; }

	HANDLE() : handle( 0 ) {}
	HANDLE( uint32_t h ) : handle( h ) {}
};

struct LHANDLE
{
	typedef uint32_t HANDLE_TYPE;
	typedef uint16_t ID;

	enum
	{
		MASK_SIZE = 32,
		ID_MASK = 0xFFFFFFFF00000000
	};

	union
	{
		struct
		{
			uint32_t idx;
			uint32_t id;
		};
		uint64_t handle;
	};

	operator uint64_t() { return handle; }
	LHANDLE() : handle( 0 ) {}
	LHANDLE( uint64_t h ) : handle( h ) {}

};

template<typename T, typename H>
class System
{
public:
	typedef typename std::vector<T>::iterator Iterator;

	System() : handles(), free_handles(), data(), ids( 0 )
	{}

	H Create();
	H Create( const T& v );

	T* Acquire( H hnd );

	bool Acquire( H hnd, T& d ) const;
	void Set( H hdn, const T& v );

	void Free( H hnd );

	Iterator Begin() { return data.begin(); }
	Iterator End() { return data.begin() + ( last + 1 ); }

private:
	// non-copyable
	System( const System& s );
	System& operator=(const System& s);

	std::vector<H> handles;
	std::vector<size_t> free_handles;
	// in order to iterate faster over the whole data vector, we will keep an array of indexes mapping
	// the handles with the real position in the data
	std::vector<typename H::ID> map;
	std::vector<T> data;

	typename H::ID ids;
	typename H::ID last;
};

}
}

//
// implementation
//

namespace DM=DaniM::MemoryManagement;
using DM::HANDLE;
using std::swap;

template<typename T, typename H>
H DM::System<T,H>::Create()
{
	typename H::ID nextId = ids++;
	if( nextId == 0 )
	{
		// 0 is the null (invalid) handle
		ids = 1;
		nextId = 1;
	}


	if( free_handles.size() > 0 )
	{
		size_t idx = free_handles[free_handles.size()-1];
		free_handles.pop_back();
		last++;

		handles[idx].id = nextId;
		map[idx] = last;		
		data[last] = T();
		return handles[idx];	
	}
	else
	{
		if( handles.size() < ( 1 << H::MASK_SIZE ) )
		{
			size_t idx = handles.size();
			H hnd;
			hnd.id = nextId;
			hnd.idx = idx;
			handles.push_back( hnd );

			map.push_back( idx );
			data.push_back( T() );			

			last = data.size() - 1;	
	
			return handles[idx];
		}
		else
		{
			return 0;
		}
	}
}

template<typename T, typename H>
H DM::System<T,H>::Create( const T& v )
{
	typename H::ID nextId = ids++;
	if( nextId == 0 )
	{
		// 0 is the null (invalid) handle
		ids = 1;
		nextId = 1;
	}


	if( free_handles.size() > 0 )
	{
		size_t idx = free_handles[free_handles.size()-1];
		free_handles.pop_back();
		last++;

		handles[idx].id = nextId;
		map[idx] = last;		
		data[last] = v;


		return handles[idx];	
	}
	else
	{
		if( handles.size() < ( 1 << H::MASK_SIZE ) )
		{
			size_t idx = handles.size();
			H hnd;
			hnd.id = nextId;
			hnd.idx = idx;
			handles.push_back( hnd );

			map.push_back( idx );
			data.push_back( v );			
	
			last = data.size() - 1;	

			return handles[idx];
		}
		else
		{
			return 0;
		}
	}
}


template<typename T, typename H>
void DM::System<T,H>::Set( H hnd, const T& v )
{
	if( hnd.idx < handles.size() && hnd.id == handles[hnd.idx].id )
	{
		// valid handle, remove it
		size_t idx = map[hnd.idx];
		data[idx] = v;
	}
}


template<typename T, typename H>
void DM::System<T,H>::Free( H hnd )
{
	if( hnd.idx < handles.size() && hnd.id == handles[hnd.idx].id )
	{
		// valid handle, remove it
		handles[hnd.idx].id = 0;
		free_handles.push_back( hnd.idx );
		size_t idx = map[hnd.idx];

		// swap with the last one
		size_t m_idx;
		for( m_idx = 0; m_idx != map.size(); ++m_idx )
		{
			if( map[m_idx] == last && handles[m_idx].id != 0 )
			{
				break;
			}
		}


		map[ m_idx ] = idx;
		swap( data[idx], data[last] );

		last--;
	}
}

template<typename T, typename H>
T* DM::System<T,H>::Acquire( H hnd )
{
	if( hnd.idx < handles.size() && hnd.id == handles[hnd.idx].id )
	{
		size_t idx = map[ hnd.idx ];
		return &(*( data.begin() + idx ));
	}
	else
	{
		return 0;
	}
}

template<typename T, typename H>
bool DM::System<T,H>::Acquire( H hnd, T& d ) const
{
	if( hnd.idx < handles.size() && hnd.id == handles[hnd.idx].id )
	{
		size_t idx = map[ hnd.idx ];
		d = data[idx];
		return true;
	}
	else
	{
		return false;
	}
}

#endif
