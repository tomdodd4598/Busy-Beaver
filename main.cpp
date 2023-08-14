#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <thread>
#include <unordered_set>
#include <vector>

typedef int_fast8_t i8;
typedef int64_t i64;
typedef uint64_t u64;

typedef std::atomic<i64> a_i64;
typedef std::atomic<size_t> a_size;

typedef std::unordered_set<size_t> hash_set;
typedef std::vector<i8> tape_vec;
typedef std::vector<std::thread> thread_vec;

void bb1();
void bb2();
void bb3();
void bb4();

int main() {
	int n;
	std::cout << "Card count: ";
	std::cin >> n;
	std::cout << "\n";

	if (n < 0) std::cout << "Can not compute busy beaver function for negative number of cards!\n";
	else if (n == 0) std::cout << "Score: " << 0 << "\n";
	else if (n == 1) bb1();
	else if (n == 2) bb2();
	else if (n == 3) bb3();
	else if (n == 4) bb4();
	else std::cout << "Can not compute busy beaver function for more than 4 cards!\n";

	return 0;
}

int get_bb_thread_count() {
	int threads;
	const int max_threads = std::thread::hardware_concurrency() * 2;
	std::cout << "Worker thread count, " << 1 << " minimum and " << max_threads << " maximum: ";
	std::cin >> threads;
	std::cout << "\n";

	if (threads < 1) {
		threads = 1;
	}
	else if (threads > max_threads) {
		threads = max_threads;
	}

	return threads;
}

void set_bb_threads(thread_vec& vec, int bb_thread_count, std::function<void(a_i64*, a_size*)> bb_internal, a_i64* perm_count, a_size* score) {
	for (int i = 0; i < bb_thread_count; ++i) {
		vec.push_back(std::thread(bb_internal, perm_count, score));
	}
}

void join_bb_threads(thread_vec& vec) {
	std::for_each(vec.begin(), vec.end(), std::mem_fn(&std::thread::join));
}

void update_max_atomic(a_size* max_value, const size_t new_value) {
	size_t prev_value = *max_value;
	while (prev_value < new_value && !max_value->compare_exchange_weak(prev_value, new_value));
}

i8 to_dir(i64 raw) {
	return (raw > 0l) - (raw <= 0l);
}

size_t tape_index(i64 pos) {
	const i64 mask = pos >> 63;
	const u64 mag = (pos + mask) ^ mask;
	return static_cast<size_t>((mag << 1) - (pos < 0));
}

void tape_check(tape_vec& vec, size_t idx) {
	if (idx >= vec.size()) vec.resize(idx << 1, 0);
}

size_t tape_count(tape_vec const& vec) {
	size_t num = 0;
	for (i8 c : vec) {
		num += (c > 0);
	}
	return num;
}

size_t bb_hash(tape_vec const& vec, i64 state, i8 in, size_t idx) {
	size_t h = 5381 + vec.size();
	for (i8 c : vec) {
		h = ((h << 5) + h) + c;
	}
	h = ((h << 5) + h) + state;
	h = ((h << 5) + h) + in;
	return ((h << 5) + h) + idx;
}

bool contains(hash_set const& set, size_t elem) {
	return set.find(elem) != set.end();
}

void bb1() {
	constexpr size_t VECLIMIT = 4;
	constexpr size_t SETLIMIT = 16;

	size_t score = 0;

	i64 perm = -1;
	i64 card[1][2][3];

	while (++perm < 64) {
		i64 valid = 0;
		i64 state;
		i64 c0 = perm / 8;
		i64 c1 = perm % 8;

		card[0][0][0] = c0 & 1;
		card[0][0][1] = to_dir(c0 & 2);
		card[0][0][2] = state = c0 >> 2;
		valid += (state < 1);

		card[0][1][0] = c1 & 1;
		card[0][1][1] = to_dir(c1 & 2);
		card[0][1][2] = state = c1 >> 2;
		valid += (state < 1);

		if (!valid) continue;

		auto vec = tape_vec(1, 0);
		auto hashes = hash_set();

		state = 0;
		i8 in = 0;
		i64 pos = 0;
		size_t hash, idx = 0;

		do {
			if (vec.size() > VECLIMIT || hashes.size() > SETLIMIT || contains(hashes, hash = bb_hash(vec, state, in, idx))) {
				state = -1;
				break;
			}
			else hashes.insert(hash);
			idx = tape_index(pos);
			tape_check(vec, idx);
			in = vec[idx];
			vec[idx] = static_cast<i8>(card[state][in][0]);
			pos += card[state][in][1];
			state = card[state][in][2];
		}
		while (state < 1);

		if (state > 0) score = std::max(score, tape_count(vec));
	}

	std::cout << "Score: " << score << "\n";
}

void bb2() {
	constexpr size_t VECLIMIT = 8;
	constexpr size_t SETLIMIT = 32;

	size_t score = 0;

	i64 perm = -1;
	i64 card[2][2][3];

	while (++perm < 20736) {
		i64 valid = 0;
		i64 state;
		i64 c0 = perm / 1728;
		i64 c1 = (perm / 144) % 12;
		i64 c2 = (perm / 12) % 12;
		i64 c3 = perm % 12;

		card[0][0][0] = c0 & 1;
		card[0][0][1] = to_dir(c0 & 2);
		card[0][0][2] = state = c0 >> 2;
		valid += (state < 2);

		card[0][1][0] = c1 & 1;
		card[0][1][1] = to_dir(c1 & 2);
		card[0][1][2] = state = c1 >> 2;
		valid += (state < 2);

		card[1][0][0] = c2 & 1;
		card[1][0][1] = to_dir(c2 & 2);
		card[1][0][2] = state = c2 >> 2;
		valid += (state < 2);

		card[1][1][0] = c3 & 1;
		card[1][1][1] = to_dir(c3 & 2);
		card[1][1][2] = state = c3 >> 2;
		valid += (state < 2);

		if (!valid) continue;

		auto vec = tape_vec(1, 0);
		auto hashes = hash_set();

		state = 0;
		i8 in = 0;
		i64 pos = 0;
		size_t hash, idx = 0;

		do {
			if (vec.size() > VECLIMIT || hashes.size() > SETLIMIT || contains(hashes, hash = bb_hash(vec, state, in, idx))) {
				state = -1;
				break;
			}
			else hashes.insert(hash);
			idx = tape_index(pos);
			tape_check(vec, idx);
			in = vec[idx];
			vec[idx] = static_cast<i8>(card[state][in][0]);
			pos += card[state][in][1];
			state = card[state][in][2];
		}
		while (state < 2);

		if (state > 0) score = std::max(score, tape_count(vec));
	}

	std::cout << "Score: " << score << "\n";
}

constexpr i64 BB3_PERMCOUNT = 16777216;

void bb3_internal(a_i64* perm_count, a_size* score) {
	constexpr size_t VECLIMIT = 12;
	constexpr size_t SETLIMIT = 64;

	i64 perm;
	i64 card[3][2][3];

	while ((perm = ++*perm_count) < BB3_PERMCOUNT) {
		i64 valid = 0;
		i64 state;
		i64 c0 = perm / 1048576;
		i64 c1 = (perm / 65536) % 16;
		i64 c2 = (perm / 4096) % 16;
		i64 c3 = (perm / 256) % 16;
		i64 c4 = (perm / 16) % 16;
		i64 c5 = perm % 16;

		card[0][0][0] = c0 & 1;
		card[0][0][1] = to_dir(c0 & 2);
		card[0][0][2] = state = c0 >> 2;
		valid += (state < 3);

		card[0][1][0] = c1 & 1;
		card[0][1][1] = to_dir(c1 & 2);
		card[0][1][2] = state = c1 >> 2;
		valid += (state < 3);

		card[1][0][0] = c2 & 1;
		card[1][0][1] = to_dir(c2 & 2);
		card[1][0][2] = state = c2 >> 2;
		valid += (state < 3);

		card[1][1][0] = c3 & 1;
		card[1][1][1] = to_dir(c3 & 2);
		card[1][1][2] = state = c3 >> 2;
		valid += (state < 3);

		card[2][0][0] = c4 & 1;
		card[2][0][1] = to_dir(c4 & 2);
		card[2][0][2] = state = c4 >> 2;
		valid += (state < 3);

		card[2][1][0] = c5 & 1;
		card[2][1][1] = to_dir(c5 & 2);
		card[2][1][2] = state = c5 >> 2;
		valid += (state < 3);

		if (!valid) continue;

		auto vec = tape_vec(1, 0);
		auto hashes = hash_set();

		state = 0;
		i8 in = 0;
		i64 pos = 0;
		size_t hash, idx = 0;

		do {
			if (vec.size() > VECLIMIT || hashes.size() > SETLIMIT || contains(hashes, hash = bb_hash(vec, state, in, idx))) {
				state = -1;
				break;
			}
			else hashes.insert(hash);
			idx = tape_index(pos);
			tape_check(vec, idx);
			in = vec[idx];
			vec[idx] = static_cast<i8>(card[state][in][0]);
			pos += card[state][in][1];
			state = card[state][in][2];
		}
		while (state < 3);

		if (state > 0) {
			update_max_atomic(score, tape_count(vec));
		}
	}
}

void bb3() {
	int threads = get_bb_thread_count();
	std::cout << "Using " << threads << " worker thread" << (threads == 1 ? "." : "s.") << " This may take some time...\n0%";

	a_i64 perm_count = 0;
	a_size score = 0;
	bool finished = false;

	std::thread progress_daemon([&perm_count, &finished] {
		int progress = 0;
		while (!finished) {
			const int new_progress = static_cast<int>(100.0 * perm_count / BB3_PERMCOUNT);
			if (new_progress > progress) std::cout << "\r" << new_progress << "%";
			progress = new_progress;
		}
	});

	auto vec = thread_vec();
	set_bb_threads(vec, threads, &bb3_internal, &perm_count, &score);

	join_bb_threads(vec);
	finished = true;
	progress_daemon.join();

	std::cout << "\r100%\n\nScore: " << score << "\n";
}

constexpr i64 BB4_PERMCOUNT = 25600000000;

void bb4_internal(a_i64* perm_count, a_size* score) {
	constexpr size_t VECLIMIT = 24;
	constexpr size_t SETLIMIT = 128;

	i64 perm;
	i64 card[4][2][3];

	while ((perm = ++*perm_count) < BB4_PERMCOUNT) {
		i64 valid = 0;
		i64 state;
		i64 c0 = perm / 1280000000;
		i64 c1 = (perm / 64000000) % 20;
		i64 c2 = (perm / 3200000) % 20;
		i64 c3 = (perm / 160000) % 20;
		i64 c4 = (perm / 8000) % 20;
		i64 c5 = (perm / 400) % 20;
		i64 c6 = (perm / 20) % 20;
		i64 c7 = perm % 20;

		card[0][0][0] = c0 & 1;
		card[0][0][1] = to_dir(c0 & 2);
		card[0][0][2] = state = c0 >> 2;
		valid += (state < 4);

		card[0][1][0] = c1 & 1;
		card[0][1][1] = to_dir(c1 & 2);
		card[0][1][2] = state = c1 >> 2;
		valid += (state < 4);

		card[1][0][0] = c2 & 1;
		card[1][0][1] = to_dir(c2 & 2);
		card[1][0][2] = state = c2 >> 2;
		valid += (state < 4);

		card[1][1][0] = c3 & 1;
		card[1][1][1] = to_dir(c3 & 2);
		card[1][1][2] = state = c3 >> 2;
		valid += (state < 4);

		card[2][0][0] = c4 & 1;
		card[2][0][1] = to_dir(c4 & 2);
		card[2][0][2] = state = c4 >> 2;
		valid += (state < 4);

		card[2][1][0] = c5 & 1;
		card[2][1][1] = to_dir(c5 & 2);
		card[2][1][2] = state = c5 >> 2;
		valid += (state < 4);

		card[3][0][0] = c6 & 1;
		card[3][0][1] = to_dir(c6 & 2);
		card[3][0][2] = state = c6 >> 2;
		valid += (state < 4);

		card[3][1][0] = c7 & 1;
		card[3][1][1] = to_dir(c7 & 2);
		card[3][1][2] = state = c7 >> 2;
		valid += (state < 4);

		if (!valid) continue;

		auto vec = tape_vec(1, 0);
		auto hashes = hash_set();

		state = 0;
		i8 in = 0;
		i64 pos = 0;
		size_t hash, idx = 0;

		do {
			if (vec.size() > VECLIMIT || hashes.size() > SETLIMIT || contains(hashes, hash = bb_hash(vec, state, in, idx))) {
				state = -1;
				break;
			}
			else hashes.insert(hash);
			idx = tape_index(pos);
			tape_check(vec, idx);
			in = vec[idx];
			vec[idx] = static_cast<i8>(card[state][in][0]);
			pos += card[state][in][1];
			state = card[state][in][2];
		} 		while (state < 4);

		if (state > 0) {
			update_max_atomic(score, tape_count(vec));
		}
	}
}

void bb4() {
	int threads = get_bb_thread_count();
	std::cout << "Using " << threads << " worker thread" << (threads == 1 ? "." : "s.") << " This may take a very, very long time...\n0.00%";
	std::cout << std::fixed << std::setprecision(2);

	a_i64 perm_count = 0;
	a_size score = 0;
	bool finished = false;

	std::thread progress_daemon([&perm_count, &finished] {
		int progress = 0;
		while (!finished) {
			double percent = 100.0 * perm_count / BB4_PERMCOUNT;
			int new_progress = static_cast<int>(100.0 * percent);
			if (new_progress > progress) std::cout << "\r" << percent << "%";
			progress = new_progress;
		}
	});

	auto vec = thread_vec();
	set_bb_threads(vec, threads, &bb4_internal, &perm_count, &score);

	join_bb_threads(vec);
	finished = true;
	progress_daemon.join();

	std::cout << "\r100.00%\n\nScore: " << score << "\n";
}
