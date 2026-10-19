#include <iostream>
#include <thread>
#include <vector>
#include <cstdint>
#include <chrono>
#include <algorithm>
#include <cstdlib>

void count_reciprocals(int64_t a, int64_t b, double* result) {
    double sum = 0.0;
    for (int64_t i = a; i <= b; ++i) sum += 1.0 /i;
    *result = sum;
}

void count_backwards(int64_t a, int64_t b, double* result) {
    double sum = 0.0;
    for(int64_t i = b; i >= a; --i) sum += 1.0/i;
    *result = sum;
}

int main() {
    double count[32];
    int64_t max = 10'000'000'000;
    double total = 0;


    auto start0 = std::chrono::high_resolution_clock::now();

    std::thread t0(count_backwards, 1, max, &count[0]);
    t0.join();
    total = count[0];

    auto stop0 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed0 = stop0 - start0;

    std::cout << "1 Thread Backward: " << std::endl;
    std::cout << "Result: " << total << "\n";
    std::cout << "Elapsed: " << elapsed0.count() << " s\n";






    auto start1 = std::chrono::high_resolution_clock::now();

    std::thread t1(count_reciprocals, 1, max, &count[0]);
    t1.join();
    total = count[0];

    auto stop1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed1 = stop1 - start1;

    std::cout << "1 Thread: Forward" << std::endl;
    std::cout << "Result: " << total << "\n";
    std::cout << "Elapsed: " << elapsed1.count() << " s\n";


    auto start2 = std::chrono::high_resolution_clock::now();

    std::thread t2(count_reciprocals, 1, max/2, &count[0]);
    std::thread t3(count_reciprocals, max/2 + 1, max, &count[1]);
    t2.join();
    t3.join();
    total = count[0] + count[1]; 

    auto stop2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed2 = stop2 - start2;

    std::cout << "2 Threads: " << std::endl; 
    std::cout << "Result: " << total << "\n";
    std::cout << "Elapsed: " << elapsed2.count() << " s\n";


    auto start3 = std::chrono::high_resolution_clock::now();

    std::thread t4(count_reciprocals, 0 *max/4 + 1, 1 *max/4, &count[0]);
    std::thread t5(count_reciprocals, 1 *max/4 + 1, 2 *max/4, &count[1]);
    std::thread t6(count_reciprocals, 2 *max/4 +1,  3 *max/4, &count[2]);
    std::thread t7(count_reciprocals, 3 *max/4 + 1, 4 *max/4, &count[3]);

    t4.join();
    t5.join();
    t6.join();
    t7.join();

    total = 0;
    total = count[0] + count[1] + count[2] + count[3];

    auto stop3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed3 = stop3 - start3;

    std::cout << "4 Threads: " << std::endl;
    std::cout << "Result: " << total << "\n";
    std::cout << "Elapsed: " << elapsed3.count() << " s\n";


    auto start4 = std::chrono::high_resolution_clock::now();

    std::thread t8 (count_reciprocals, 0 *max/8 + 1, 1 *max/8, &count[0]);
    std::thread t9 (count_reciprocals, 1 *max/8 + 1, 2 *max/8, &count[1]);
    std::thread t10(count_reciprocals, 2 *max/8 + 1, 3 *max/8, &count[2]);
    std::thread t11(count_reciprocals, 3 *max/8 + 1, 4 *max/8, &count[3]);
    std::thread t12(count_reciprocals, 4 *max/8 + 1, 5 *max/8, &count[4]);
    std::thread t13(count_reciprocals, 5 *max/8 + 1, 6 *max/8, &count[5]);
    std::thread t14(count_reciprocals, 6 *max/8 + 1, 7 *max/8, &count[6]);
    std::thread t15(count_reciprocals, 7 *max/8 + 1, 8 *max/8, &count[7]);

    t8.join();  t9.join();  t10.join(); t11.join();
    t12.join(); t13.join(); t14.join(); t15.join();

    total = 0;
    total = count[0] + count[1] + count[2] + count[3] +
        count[4] + count[5] + count[6] + count[7];

    auto stop4 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed4 = stop4 - start4;

    std::cout << "8 Threads:\n";
    std::cout << "Result: " << total << "\n";
    std::cout << "Elapsed: " << elapsed4.count() << " s\n";

    // ---------- 16 threads ----------
    auto start5 = std::chrono::high_resolution_clock::now();

    std::thread t16(count_reciprocals,  0 *max/16 + 1,  1 *max/16, &count[0]);
    std::thread t17(count_reciprocals,  1 *max/16 + 1,  2 *max/16, &count[1]);
    std::thread t18(count_reciprocals,  2 *max/16 + 1,  3 *max/16, &count[2]);
    std::thread t19(count_reciprocals,  3 *max/16 + 1,  4 *max/16, &count[3]);
    std::thread t20(count_reciprocals,  4 *max/16 + 1,  5 *max/16, &count[4]);
    std::thread t21(count_reciprocals,  5 *max/16 + 1,  6 *max/16, &count[5]);
    std::thread t22(count_reciprocals,  6 *max/16 + 1,  7 *max/16, &count[6]);
    std::thread t23(count_reciprocals,  7 *max/16 + 1,  8 *max/16, &count[7]);
    std::thread t24(count_reciprocals,  8 *max/16 + 1,  9 *max/16, &count[8]);
    std::thread t25(count_reciprocals,  9 *max/16 + 1, 10 *max/16, &count[9]);
    std::thread t26(count_reciprocals, 10 *max/16 + 1, 11 *max/16, &count[10]);
    std::thread t27(count_reciprocals, 11 *max/16 + 1, 12 *max/16, &count[11]);
    std::thread t28(count_reciprocals, 12 *max/16 + 1, 13 *max/16, &count[12]);
    std::thread t29(count_reciprocals, 13 *max/16 + 1, 14 *max/16, &count[13]);
    std::thread t30(count_reciprocals, 14 *max/16 + 1, 15 *max/16, &count[14]);
    std::thread t31(count_reciprocals, 15 *max/16 + 1, 16 *max/16, &count[15]);

    t16.join(); t17.join(); t18.join(); t19.join();
    t20.join(); t21.join(); t22.join(); t23.join();
    t24.join(); t25.join(); t26.join(); t27.join();
    t28.join(); t29.join(); t30.join(); t31.join();

    total = 0;
    for (int i = 0; i < 16; ++i) total += count[i];

    auto stop5 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed5 = stop5 - start5;

    std::cout << "16 Threads:\n";
    std::cout << "Result: " << total << "\n";
    std::cout << "Elapsed: " << elapsed5.count() << " s\n";



    return 0;
}

