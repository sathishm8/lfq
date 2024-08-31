#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <unistd.h>
#include <atomic>

using namespace std;

template <typename TT, uint32_t n>
class ringBuf {
private:
	vector<TT>        ring;
	atomic<uint32_t>  rptr, wptr;
	atomic<uint32_t>  count;
	uint32_t          size;

public:
	ringBuf() : rptr(0), wptr(0), count(0), size(n) {
		ring.resize(n);
		cout << "SPSC Queue Size: " << n << endl;
	}

	void update_size(uint32_t);
	bool enqueue(TT);
	bool dequeue(TT*);
	uint32_t get_size(void) {
		return size;
	}
	bool ringIsFull(void) {
		return (count == size);
	}
	bool ringIsEmpty(void) {
		return (count == 0);
	}
};

template <typename TT, uint32_t n>
void ringBuf<TT, n>::update_size(uint32_t s) {
	size = s;
	ring.resize(size);
}

template <typename TT, uint32_t n>
bool ringBuf<TT, n>::enqueue(TT o)
{
	if ((wptr + 1) % size == rptr)
		return false;
	ring[wptr] = o;
	wptr = (wptr + 1) % size;
	count++;
	return true;
}

template <typename TT, uint32_t n>
bool ringBuf<TT, n>::dequeue(TT* o)
{
	if (rptr == wptr)
		return false;
	*o = ring[rptr];
	rptr = (rptr + 1) % size;
	count--;
	return true;
}

