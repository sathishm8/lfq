#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <unistd.h>
#include <thread>
#include <atomic>

using namespace std;

template <typename TT, uint32_t n>
class ringBuf {
private:
	vector<TT>			ring;
	atomic<uint32_t>	rptr, wptr;
	atomic<uint32_t>	count;
	uint32_t			size;

public:
	ringBuf() : size(n), rptr(0),	wptr(0), count(0), ring(n) {
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

typedef std::chrono::high_resolution_clock Clock;

typedef int* Obj_P;
typedef int Obj_T;

int main(int argc, char* argv[])
{
	ringBuf<Obj_P, 1024> r;
	thread t[2];
	Obj_T d = 1;
	Obj_P p;

	auto pwork = [&r](Obj_P addr, int n) {
		int k = 0;
		auto start_time = Clock::now();
		do {
			if (!r.enqueue(addr))
				std::this_thread::sleep_for(2ns);
			else
				k++;
		} while (k < n);
		auto end_time = Clock::now();
		cout << "Producer work cycles = " << k << " " << addr << endl;
	    cout << "Queue Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time    - start_time).count() << " ms" << std::endl;
		std::this_thread::sleep_for(20ms);
	};

	auto cwork = [&r](Obj_P *p, int n) {
		int k = 0;
		auto start_time = Clock::now();
		do {
			if (!r.dequeue(p))
				std::this_thread::sleep_for(4ns);
			else
				k++;
		} while (k < n);
		auto end_time = Clock::now();
		std::this_thread::sleep_for(20ms);
		cout << "Consumer work cycles = " << k << " " << *p << endl;
	    cout << "Dequeue Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time    - start_time).count() << " ms" << std::endl;
	};

	t[0] = std::thread(pwork, &d, 10*1000*1000);
	t[1] = std::thread(cwork, &p, 10*1000*1000);
	t[0].join();
	t[1].join();

	return 0;
}

