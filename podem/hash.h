/************************************************************************/
/*                                                                      */
/*     RAMBAS --- Redundancy-Addition-and-Removal Based Framework       */
/*                version 2.2.1 (special version for ECE255)            */
/*                                                                      */
/*     Author       : Ric Chung-Yang Huang                              */
/*     Last updated : May 6, 1998                                       */
/*                                                                      */
/*                University of California, Santa Barbara               */
/*                                                                      */
/************************************************************************/

//
// class KEY must overload the operator "==" and define Hash_function();
//
#ifndef HASH_H
#define HASH_H

#include <string>     // for GNU compilier, for others --> string.h
#include <vector>      // string must be included before stl.h
#include <list>      // string must be included before stl.h
#include <iostream>
#include <stdlib.h>
using namespace std;

class Str_hash_function
{
    public:
	int operator() (const string id, const int size)
	{
	    unsigned base_key, tmp_key, ch;
	    base_key = tmp_key = ch = 0;
	    for (unsigned i = 0; i < id.size(); i++) {
		tmp_key = tmp_key << 8;
		tmp_key += id[i];
		if (i % 4 == 3) {
		    base_key += tmp_key;
		    tmp_key = 0;
		}
	    }
	    base_key += tmp_key;
	    return int(base_key % size);
	}
};

template <class VALUE>
class Pred
{
    public:
	virtual bool operator() (const VALUE v) const
	{
	    return 1;
	}
};

template <class VALUE>
class Exec
{
    public:
	virtual void operator() (VALUE v) = 0;
};

template <class KEY, class VALUE, class HASH_FUNCTION>
class Cache
{
    int _size;
    vector<pair<KEY, VALUE> > bucket;
    public:
    Cache(int s = 8191) : bucket(s) { _size = s; }
    ~Cache() {}
    int size() { return _size; }
    void write(const KEY k, const VALUE v)
    {
	HASH_FUNCTION f;
	bucket[f(k, _size)] = make_pair(k, v);
    }
    VALUE read(KEY k)
    {
	HASH_FUNCTION f; int b = f(k, _size);
	if (bucket[b].first == k) return (bucket[b].second);
	return VALUE(0);
    }
    VALUE operator[] (KEY k)
    {
	HASH_FUNCTION f; int b = f(k, _size);
	if (bucket[b].first == k) return (bucket[b].second);
	return VALUE(0);
    }
    VALUE& operator[] (int b) { return (bucket[b].second); }
    void clean() { bucket.erase(bucket.begin(), bucket.end()); }
};

template <class KEY, class VALUE, class HASH_FUNCTION>
class Hash
{
    int _size;
    unsigned num_of_nodes;
    vector<list<pair<KEY, VALUE> > > bucket;
    public:
    Hash(int s = 509)
    {
	_size = s; num_of_nodes = 0;
	bucket.reserve(_size);
	for (int i = 0; i < _size; i++) { list<pair<KEY, VALUE> > l; bucket.push_back(l); }
    }
    ~Hash() { bucket.erase(bucket.begin(), bucket.end()); }
    int size() { return _size; }
    unsigned nodes() { return num_of_nodes; }
    int bucket_num(KEY k);  // return -1 if not in hash
    int test_insert(KEY k); // return bucket_num if not in hash;
    // else return -1;
    int insert(const KEY k, const VALUE v);
    void forced_insert(const int b, const KEY k, const VALUE v);
    int is_member(KEY k);
    VALUE pop_value(KEY k);
    VALUE get_value(KEY k);
    VALUE get_value(KEY k, int b);
    int remove(KEY k);
    unsigned remove_if(Pred<VALUE> *pred);
    unsigned remove_if_else(Pred<VALUE> *pred, Exec<VALUE> *e);
    void clean();
    VALUE operator[] (KEY k);
    // get a "copy" of bucket[b]; not bucket[b] itself!!
    list<pair<KEY, VALUE> >& operator[] (int b) { return bucket[b]; }
    void for_each_exec(Exec<VALUE> *e);
    void for_each_if_else(Pred<VALUE> *, Exec<VALUE> *, Exec<VALUE> *);
};

template <class KEY, class VALUE, class HASH_FUNCTION>
    int
Hash<KEY, VALUE, HASH_FUNCTION>::bucket_num(KEY k)
{
    HASH_FUNCTION f;
    int b = f(k, _size);
    if (bucket[b].empty())
	return ( -1);
    typename list<pair<KEY, VALUE> >::iterator li = bucket[b].begin();
    typename list<pair<KEY, VALUE> >::iterator last = bucket[b].end();
    for (; li != last; li++)
	if ((*li).first == k)
	    return b;
    return ( -1);
}

template <class KEY, class VALUE, class HASH_FUNCTION>
    int
Hash<KEY, VALUE, HASH_FUNCTION>::test_insert(KEY k)
{
    HASH_FUNCTION f;
    int b = f(k, _size);
    if (bucket[b].empty())
	return b;
    typename list<pair<KEY, VALUE> >::iterator li = bucket[b].begin();
    typename list<pair<KEY, VALUE> >::iterator last = bucket[b].end();
    for (; li != last; li++)
	if ((*li).first == k)
	    return ( -1);
    return b;
}

template <class KEY, class VALUE, class HASH_FUNCTION>
    int
Hash<KEY, VALUE, HASH_FUNCTION>::insert(const KEY k, const VALUE v)
{
    HASH_FUNCTION f;
    int b = f(k, _size);
    if (bucket[b].empty()) {
	bucket[b].push_back(make_pair(k, v));
	num_of_nodes++;
	return 1;
    }
    typename list<pair<KEY, VALUE> >::iterator li = bucket[b].begin();
    typename list<pair<KEY, VALUE> >::iterator last = bucket[b].end();
    for (; li != last; li++)
	if ((*li).first == k) {
	    //cerr << "Warning : data already exist!!" << endl;
	    return 0;
	}
    pair<KEY, VALUE> p = make_pair(k, v);
    bucket[b].push_back(p);
    num_of_nodes++;
    return 1;
}

template <class KEY, class VALUE, class HASH_FUNCTION>
    void Hash<KEY, VALUE, HASH_FUNCTION>::forced_insert
(const int b, const KEY k, const VALUE v)
{
    pair<KEY, VALUE> p = make_pair(k, v);
    bucket[b].push_back(p);
    num_of_nodes++;
}

template <class KEY, class VALUE, class HASH_FUNCTION>
    int
Hash<KEY, VALUE, HASH_FUNCTION>::is_member(KEY k)
{
    HASH_FUNCTION f;
    int b = f(k, _size);
    if (bucket[b].empty())
	return 0;
    typename list<pair<KEY, VALUE> >::iterator li = bucket[b].begin();
    typename list<pair<KEY, VALUE> >::iterator last = bucket[b].end();
    for (; li != last; li++)
	if ((*li).first == k)
	    return 1;
    return 0;
}

template <class KEY, class VALUE, class HASH_FUNCTION>
    VALUE
Hash<KEY, VALUE, HASH_FUNCTION>::pop_value(KEY k)
{
    HASH_FUNCTION f;
    int b = f(k, _size);
    if (bucket[b].empty())
	return VALUE(0);
    typename list<pair<KEY, VALUE> >::iterator li = bucket[b].begin();
    typename list<pair<KEY, VALUE> >::iterator last = bucket[b].end();
    for (; li != last; li++)
	if ((*li).first == k) {
	    VALUE v = (*li).second;
	    bucket[b].erase(li);
	    num_of_nodes--;
	    return v;
	}
    return VALUE(0);
}

template <class KEY, class VALUE, class HASH_FUNCTION>
    VALUE
Hash<KEY, VALUE, HASH_FUNCTION>::get_value(KEY k)
{
    HASH_FUNCTION f;
    int b = f(k, _size);
    if (bucket[b].empty())
	return VALUE(0);
    typename list<pair<KEY, VALUE> >::iterator li = bucket[b].begin();
    typename list<pair<KEY, VALUE> >::iterator last = bucket[b].end();
    for (; li != last; li++)
	if ((*li).first == k)
	    return (*li).second;
    return VALUE(0);
}

template <class KEY, class VALUE, class HASH_FUNCTION>
    VALUE
Hash<KEY, VALUE, HASH_FUNCTION>::get_value(KEY k, int b)
{
    if (b >= _size || b < 0) {
	cerr << "Error : bucket number out of range!!" << endl;
	exit( -1);
    }
    if (bucket[b].empty())
	return VALUE(0);
    typename list<pair<KEY, VALUE> >::iterator li = bucket[b].begin();
    typename list<pair<KEY, VALUE> >::iterator last = bucket[b].end();
    for (; li != last; li++)
	if ((*li).first == k)
	    return (*li).second;
    return VALUE(0);
}

template <class KEY, class VALUE, class HASH_FUNCTION>
    int
Hash<KEY, VALUE, HASH_FUNCTION>::remove(KEY k)
{
    HASH_FUNCTION f;
    int b = f(k, _size);
    if (bucket[b].empty())
	return 0;
    typename list<pair<KEY, VALUE> >::iterator li = bucket[b].begin();
    typename list<pair<KEY, VALUE> >::iterator last = bucket[b].end();
    for (; li != last; li++)
	if ((*li).first == k) {
	    bucket[b].erase(li);
	    num_of_nodes--;
	    return 1;
	}
    return 0;
}

template <class KEY, class VALUE, class HASH_FUNCTION>
    unsigned
Hash<KEY, VALUE, HASH_FUNCTION>::remove_if(Pred<VALUE> *pred)
{
    unsigned total_removed = 0;
    for (int b = 0; b < _size; b++) {
	if (bucket[b].empty()) continue;
	typename list<pair<KEY, VALUE> >::iterator li = bucket[b].begin();
	typename list<pair<KEY, VALUE> >::iterator last = bucket[b].end();
	while (li != last) {
	    typename list<pair<KEY, VALUE> >::iterator this_li = li++;
	    if ((*pred)((*this_li).second)) {
		bucket[b].erase(this_li);
		num_of_nodes--;
		total_removed++;
	    }
	}
    }
    return total_removed;
}

template <class KEY, class VALUE, class HASH_FUNCTION>
unsigned
    Hash<KEY, VALUE, HASH_FUNCTION>::remove_if_else
(Pred<VALUE> *pred, Exec<VALUE> *e)
{
    unsigned total_removed = 0;
    for (int b = 0; b < _size; b++) {
	if (bucket[b].empty()) continue;
	typename list<pair<KEY, VALUE> >::iterator li = bucket[b].begin();
	typename list<pair<KEY, VALUE> >::iterator last = bucket[b].end();
	while (li != last) {
	    typename list<pair<KEY, VALUE> >::iterator this_li = li++;
	    if ((*pred)((*this_li).second)) {
		bucket[b].erase(this_li);
		num_of_nodes--;
		total_removed++;
	    }
	    else (*e)((*this_li).second);
	}
    }
    return total_removed;
}

template <class KEY, class VALUE, class HASH_FUNCTION>
    void
Hash<KEY, VALUE, HASH_FUNCTION>::clean()
{
    num_of_nodes = 0;
    for (int b = 0; b < _size; b++)
	bucket[b].erase(bucket[b].begin(), bucket[b].end());
}

template <class KEY, class VALUE, class HASH_FUNCTION>
    VALUE
Hash<KEY, VALUE, HASH_FUNCTION>::operator[] (KEY k)
{
    HASH_FUNCTION f;
    int b = f(k, _size);
    if (bucket[b].empty())
	return VALUE(0);
    typename list<pair<KEY, VALUE> >::iterator li = bucket[b].begin();
    typename list<pair<KEY, VALUE> >::iterator last = bucket[b].end();
    for (; li != last; li++)
	if ((*li).first == k)
	    return (*li).second;
    return VALUE(0);
}

template <class KEY, class VALUE, class HASH_FUNCTION>
    void
Hash<KEY, VALUE, HASH_FUNCTION>::for_each_exec(Exec<VALUE> *e)
{
    for (int b = 0; b < _size; b++) {
	if (bucket[b].empty()) continue;
	typename list<pair<KEY, VALUE> >::iterator li = bucket[b].begin();
	typename list<pair<KEY, VALUE> >::iterator last = bucket[b].end();
	while (li != last)
	    (*e)((*li++).second);
    }
}

template <class KEY, class VALUE, class HASH_FUNCTION>
void
    Hash<KEY, VALUE, HASH_FUNCTION>::for_each_if_else
(Pred<VALUE> *pred, Exec<VALUE> *if_e, Exec<VALUE> *else_e)
{
    for (int b = 0; b < _size; b++) {
	if (bucket[b].empty()) continue;
	typename list<pair<KEY, VALUE> >::iterator li = bucket[b].begin();
	typename list<pair<KEY, VALUE> >::iterator last = bucket[b].end();
	while (li != last) {
	    if ((*pred)((*li).second))
		(*if_e)((*li++).second);
	    else (*else_e)((*li++).second);
	}
    }
}

#endif
