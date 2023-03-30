// (C)2008 S2 Games
// k0_stl.h
//
//=============================================================================
#ifndef __K0_STL_H__
#define __K0_STL_H__

//=============================================================================
// Headers
//=============================================================================
#include <algorithm>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <queue>
#ifdef __GNUC__
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ > 2) // gcc >= 4.3
// hash_map is depreciated under gcc-3.4, use tr1 equivalents
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#else
#include <ext/hash_map>
#include <ext/hash_set>
#endif
#else
#include <hash_map>
#include <hash_set>
#endif
#include <utility>
#include <sstream>
#include <limits>
#include <iostream>
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
using std::abs;
using std::min;
using std::max;
using std::vector;
using std::deque;
using std::list;
using std::map;
using std::set;
using std::multiset;
using std::insert_iterator;
using std::stack;
using std::queue;
#ifdef __GNUC__
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ > 2) // gcc >= 4.3
using std::tr1::unordered_map;
#define hash_map unordered_map
using std::tr1::unordered_set;
#define hash_set unordered_set
#else
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_set;
#endif
#else
using stdext::hash_map;
using stdext::hash_set;
#endif
using std::pair;
using std::ostringstream;
using std::numeric_limits;

using std::cout;
using std::cerr;
using std::wcout;
using std::wcerr;
using std::endl;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
// Common containers
typedef vector<byte>				yvector;
typedef yvector::iterator			yvector_it;
typedef yvector::const_iterator		yvector_cit;
typedef yvector::reverse_iterator	yvector_rit;

typedef vector<int>					ivector;
typedef ivector::iterator			ivector_it;
typedef ivector::const_iterator		ivector_cit;
typedef ivector::reverse_iterator	ivector_rit;

typedef vector<uint>				uivector;
typedef uivector::iterator			uivector_it;
typedef uivector::const_iterator	uivector_cit;
typedef uivector::reverse_iterator	uivector_rit;

typedef vector<float>				fvector;
typedef fvector::iterator			fvector_it;
typedef fvector::const_iterator		fvector_cit;
typedef fvector::reverse_iterator	fvector_rit;

typedef vector<size_t>				zvector;
typedef zvector::iterator			zvector_it;
typedef zvector::const_iterator		zvector_cit;
typedef zvector::reverse_iterator	zvector_rit;

typedef list<int>					ilist;
typedef ilist::iterator				ilist_it;
typedef ilist::const_iterator		ilist_cit;
typedef ilist::reverse_iterator		ilist_rit;

typedef list<uint>					uilist;
typedef uilist::iterator			uilist_it;
typedef uilist::const_iterator		uilist_cit;
typedef uilist::reverse_iterator	uilist_rit;

typedef list<float>					flist;
typedef flist::iterator				flist_it;
typedef flist::const_iterator		flist_cit;
typedef flist::reverse_iterator		flist_rit;

typedef list<size_t>				zlist;
typedef zlist::iterator				zlist_it;
typedef zlist::const_iterator		zlist_cit;
typedef zlist::reverse_iterator		zlist_rit;

typedef set<int>					iset;
typedef iset::iterator				iset_it;
typedef iset::const_iterator		iset_cit;
typedef iset::reverse_iterator		iset_rit;

typedef set<uint>					uiset;
typedef uiset::iterator				uiset_it;
typedef uiset::const_iterator		uiset_cit;
typedef uiset::reverse_iterator		uiset_rit;

typedef set<float>					fset;
typedef fset::iterator				fset_it;
typedef fset::const_iterator		fset_cit;
typedef fset::reverse_iterator		fset_rit;

typedef set<size_t>					zset;
typedef zset::iterator				zset_it;
typedef zset::const_iterator		zset_cit;
typedef zset::reverse_iterator		zset_rit;

typedef deque<int>					ideque;
typedef ideque::iterator			ideque_it;
typedef ideque::const_iterator		ideque_cit;
typedef ideque::reverse_iterator	ideque_rit;

typedef deque<uint>					uideque;
typedef uideque::iterator			uideque_it;
typedef uideque::const_iterator		uideque_cit;
typedef uideque::reverse_iterator	uideque_rit;

typedef deque<float>				fdeque;
typedef fdeque::iterator			fdeque_it;
typedef fdeque::const_iterator		fdeque_cit;
typedef fdeque::reverse_iterator	fdeque_rit;

typedef deque<size_t>				zdeque;
typedef zdeque::iterator			zdeque_it;
typedef zdeque::const_iterator		zdeque_cit;
typedef zdeque::reverse_iterator	zdeque_rit;

typedef map<int, int>				imapi;
typedef imapi::iterator				imapi_it;
typedef imapi::const_iterator		imapi_cit;
typedef imapi::reverse_iterator		imapi_rit;

#ifdef __GNUC__
#define STL_ERASE(container, it)	(container).erase(it++)
#else
#define STL_ERASE(container, it)	(it) = (container).erase(it)
#endif
//=============================================================================

#endif //__K2_STL_H__
