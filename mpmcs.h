#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <unistd.h>
#include <atomic>

using namespace std;

template <typename TT, uint32_t n, uint32_t m>
class ringBufM {
private:
	vector<TT>        ring[n];
	atomic<uint32_t>  rptr[n], wptr[n];
	atomic<uint32_t>  count[n];
	atomic<uint32_t>  size[n];
	atomic<uint32_t>  tokens[n];

public:
	ringBufM() {
		uint32_t i = 0;
		for (i = 0; i < n; i++) {
			ring[i].resize(m);
			size[i] = m;
			count[i] = 0;
			rptr[i] = 0;
			wptr[i] = 0;
			tokens[i] = -1;
			cout << "Queue[" << i << "] Size: " << m << endl;
		}
	}

	bool update_size(uint32_t s, uint8_t idx, uint32_t token);
	bool update_token(uint8_t idx, uint32_t token);
	int enqueue(TT o, uint8_t idx, uint32_t token);
	int dequeue(TT* p, uint8_t idx, uint32_t token);
	uint32_t get_size(uint8_t idx, uint32_t token) {
		return size[idx];
	}
	bool ringIsFull(uint8_t idx, uint32_t token) {
		return (count[idx] == size);
	}
	bool ringIsEmpty(uint8_t idx, uint32_t token) {
		return (count[idx] == 0);
	}
};

template <typename TT, uint32_t n, uint32_t m>
bool ringBufM<TT, n, m>::update_token(uint8_t idx, uint32_t token) {
	tokens[idx] = token;
	return true;
}

template <typename TT, uint32_t n, uint32_t m>
bool ringBufM<TT, n, m>::update_size(uint32_t s, uint8_t idx, uint32_t token) {
	if (tokens[idx] != token)
		return false;
	size[idx] = s;
	ring[idx].resize(size);
	return true;
}

template <typename TT, uint32_t n, uint32_t m>
int ringBufM<TT, n, m>::enqueue(TT o, uint8_t idx, uint32_t token)
{
	if (tokens[idx] != token)
		return -1;
	if ((wptr[idx] + 1) % size[idx] == rptr[idx])
		return 0;
	ring[idx][wptr[idx]] = o;
	wptr[idx] = (wptr[idx] + 1) % size[idx];
	count[idx]++;
	return 1;
}

template <typename TT, uint32_t n, uint32_t m>
int ringBufM<TT, n, m>::dequeue(TT* o, uint8_t idx, uint32_t token)
{
	if (tokens[idx] != token)
		return -1;
	if (rptr[idx] == wptr[idx])
		return 0;
	*o = ring[idx][rptr[idx]];
	rptr[idx] = (rptr[idx] + 1) % size[idx];
	count[idx]--;
	return 1;
}

