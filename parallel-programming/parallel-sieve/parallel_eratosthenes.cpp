#include <iostream>
#include <chrono>
#include <cmath>
#include <thread>

using namespace std;
// 3*5*7 = 105  *   64 

class prime_bits {
private:
	uint64_t n;  // the number of bits
	uint64_t num_words; // the number of 64-bit words needed to store
	uint64_t* p; // point to the dynamic memory for the bits
public:
	prime_bits(uint64_t n) : // round up because we need n+1 bits
		n(n+1), num_words(n/64+1), p(new uint64_t[n/128+1]) {
			for (int i =0; i < num_words; i++)
			  p[i] = 0;
		}
	~prime_bits() {
		delete [] p;
	}
	// prevent you from accidentally copying bits. You really don't want to
	prime_bits(const prime_bits& orig) = delete;
	prime_bits& operator =(const prime_bits& orig) = delete;

	// let's store primes as 0 and not prime (composite) as 1 it's easier.
	// to set a bit we shift the mask to the right spot and OR it in
	void clear_prime(uint64_t i) {
		// this is storing only odd numbers in each mask!
		// note 1LL is crucial. If you write just 1 it's an int.
		// (1 << 40) would be 0   (1LL << 40) is 10000000000000000000000000....
		p[i/128] |= (1ULL << ((i%128) >> 1));
	}
	bool is_prime(uint64_t i) const {
		return (p[i / 128] & (1ULL << ((i%128) >> 1))) == 0;
	}


    std::vector<bool> precompute(uint64_t n) {
        std::vector<bool> primes(n+1, true);
        primes[0] = primes[1] = false;

        for (uint64_t i = 2; i*i <= n; i++) {
            if (primes[i]) {
                for (uint64_t j = i * i; j <= n; j += i) {
                    primes[j] = false;
                }
            }
        }
        return primes;
    }




	// passing in size so we can use this to initialize subsections
	void parallel_eratosthenes(uint64_t a, uint64_t b, std::vector<bool> p, uint64_t* c) {
		uint64_t count = 1; // 2 is a special case
		uint64_t lim = sqrt(b);
		for (uint64_t i = a; i <= lim; i += 2)
			if (p[i]) {
				count++;
				for (uint64_t j = i*i; j <= b; j += 2*i)
					clear_prime(j);
			}

		for (uint64_t i = (lim + 1)| 1; i <= b; i += 2)
			if (p[i])
				count++;
        *c = count;
    }


};

int main(int argc, char* argv[]) {
	if (argc < 2) {
		cerr << "pass in the number of primes on the command line\n";
		exit(-1);
	}
	uint64_t n = atol(argv[1]);

	prime_bits primes(n);
    const std::vector<bool> p = primes.precompute(200);
    
    // COPYING prime_bits objects is ILLEGAL (no copy constructor)
    // see me if you have questions
    //	prime_bits p2 = primes;

    uint64_t count_ptr = 0;
    uint64_t count_ptr2 = 0;

	auto t0 = std::chrono::high_resolution_clock::now();
    //	uint64_t count = primes.eratosthenes(n);


    thread p1([&]{ primes.eratosthenes(1LL, 100LL, p, &count_ptr); });
    thread p2([&]{ primes.eratosthenes(101LL, 200LL,p, &count_ptr2); });

    p1.join(); 
    p2.join();
    uint64_t final = count_ptr2; 

	auto t1 = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);

	cout << "bitwise eratosthenes: " << final << " elapsed: " << elapsed.count() << "usec\n";

}
