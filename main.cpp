#include "handle.h"
#include <iostream>
#include <cassert>

using namespace DaniM::MemoryManagement;
using namespace std;

struct Foo
{
public:
	Foo() : foo(0) {}
	Foo(int f) : foo(f) {}
	int foo;
};

struct HF
{
public:
	int f;
	HANDLE h;
};

typedef System<Foo,HANDLE> FooSystem;

int main()
{
	FooSystem system;
	vector<HF> created;
	vector<size_t> reuse;
	cout << "Creating and deleting test\n";
	for( size_t i = 0; i != 10; ++i )
	{
		Foo f( i );
		Foo* pf = 0;

		HANDLE hnd = system.Create( f );
		assert( hnd != 0 && "Error when creating the handle\n" );

		pf = system.Acquire( hnd );
		HF hf;

		assert( pf && system.Acquire( hnd, f ) && "Error when acquiring the created handle\n" );
		system.Set( hnd, Foo( i ) );

		hf.f = i;
		hf.h = hnd;

		created.push_back( hf );
	}

	// delete the created ones
	while( created.size() > 5 )
	{
		size_t at = created.size() >> 1;
		HANDLE h = created[at].h;
		system.Free( h );
		created.erase( created.begin() + at );
		reuse.push_back( h.idx );
		for( vector<HF>::iterator it = created.begin(); it != created.end(); ++it )
		{
			Foo f;
			Foo* pf = system.Acquire( it->h );
			assert( pf && system.Acquire( (*it).h, f ) && "Error when acquiring the handle\n" );
			assert( pf->foo == f.foo && f.foo == (*it).f && "Corrupted data error\n" );
		}	
	}

	cout << "Reuse test\n";
	for( vector<size_t>::reverse_iterator it = reuse.rbegin(); it != reuse.rend(); ++it )
	{
		HANDLE hnd = system.Create();
		assert( hnd != 0 && "Error when creating the reused handle\n" );
		assert( hnd.idx == (*it) && "Error when reusing the handle\n" );


		Foo f;
		Foo* pf = system.Acquire( hnd );
		HF hf;

		assert( pf && system.Acquire( hnd, f ) && "Error when acquiring the reused handle\n" );
		system.Set( hnd, Foo( *it ) );

		hf.f = *it;
		hf.h = hnd;

		created.push_back( hf );


		for( vector<HF>::iterator it = created.begin(); it != created.end(); ++it )
		{
			Foo f;
			Foo* pf = system.Acquire( it->h );
			assert( pf && system.Acquire( (*it).h, f ) && "Error when acquiring the handle\n" );
			assert( pf->foo == f.foo && f.foo == (*it).f && "Corrupted data error\n" );
		}
	}


	cout << "IDs overflow test\n";
	HANDLE hnd = system.Create();
	while( hnd.id != 1 )
	{
		assert( hnd != 0 && "Error when creating the handle\n" );
		system.Free( hnd );
		hnd = system.Create();
	}

	return 0;
}
