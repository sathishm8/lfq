#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include "mpmcs.h"

using namespace std;
typedef std::chrono::high_resolution_clock Clock;

typedef int* Obj_P;
typedef int Obj_T;

int main(int argc, char* argv[])
{
	ringBufM<Obj_P, 4, 1024> r;
	thread t[8];
	uint64_t eq = 0, dq = 0;
	mutex stat_lock;
	Obj_T d = 1;
	Obj_P p;

	auto pwork = [&r, &stat_lock, &eq](Obj_P addr, int n, uint8_t idx, uint32_t tok) {
		int k = 0, ret = 0;
		auto start_time = Clock::now();
		do {
			if ((ret = r.enqueue(addr, idx, tok)) == 0)
				std::this_thread::sleep_for(2ns);
			else if (ret == -1)
				break;
			else
				k++;
		} while (k < n);
		auto end_time = Clock::now();
		stat_lock.lock();
		eq += k;
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
		cout << "Producer_id: " << tok << " work cycles = " << k << " " << addr << " Time: " << time << " ms" << endl;
		stat_lock.unlock();
	};

	auto cwork = [&r, &stat_lock, &dq](Obj_P *p, int n, uint8_t idx, uint32_t tok) {
		int k = 0, ret = 0;
		auto start_time = Clock::now();
		do {
			if ((ret = r.dequeue(p, idx, tok)) == 0)
				std::this_thread::sleep_for(4ns);
			else if (ret == -1)
				break;
			else
				k++;
		} while (k < n);
		auto end_time = Clock::now();
		stat_lock.lock();
		dq += k;
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
		cout << "Consumer_id: " << tok << " work cycles = " << k << " " << *p << " Time: " << time << " ms" << endl;
		stat_lock.unlock();
	};

	auto start_time = Clock::now();
	r.update_token(0, 0x0);
	t[0] = std::thread(pwork, &d, 4*1000*1000, 0, 0);
	t[1] = std::thread(cwork, &p, 4*1000*1000, 0, 0);

	r.update_token(1, 0x1);
	t[2] = std::thread(pwork, &d, 2*1000*1000, 1, 1);
	t[3] = std::thread(cwork, &p, 2*1000*1000, 1, 1);

	r.update_token(2, 0x2);
	t[4] = std::thread(pwork, &d, 2*1000*1000, 2, 2);
	t[5] = std::thread(cwork, &p, 2*1000*1000, 2, 2);

	r.update_token(3, 0x3);
	t[6] = std::thread(pwork, &d, 2*1000*1000, 3, 0x3);
	t[7] = std::thread(cwork, &p, 2*1000*1000, 3, 0x3);

	for (int l = 0; l < 8; l++)
		t[l].join();

	auto end_time = Clock::now();

	cout << endl << endl;
	cout << "Total enqueues: " << eq << endl;
	cout << "Total dequeues: " << dq << endl;
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	cout << "Total Time: " << time << " ms" << std::endl;
	cout << endl << endl;

	return 0;
}
