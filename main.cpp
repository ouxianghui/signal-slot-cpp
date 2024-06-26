#include <iostream>
#include "signal.hpp"
#include "core/task_queue.h"
#include "core/task_queue_manager.h"

using namespace std;

class MyParam {
public:
    ~MyParam() {

    }

    MyParam() {

    }

    MyParam(const MyParam& m) {
        std::cout << "MyParam copy" << std::endl;
    }

    MyParam& operator=(const MyParam&) {
        std::cout << "MyParam assign copy" << std::endl;
        return *this;
    }

    MyParam(MyParam&& m) {
        std::cout << "MyParam move" << std::endl;
    }

    MyParam& operator=(MyParam&&) {
        std::cout << "MyParam assign move" << std::endl;
        return *this;
    }

};

class MyClass : public sigslot::observer {
public:
    ~MyClass() {
        // Needed to ensure proper disconnection prior to object destruction
        // in multithreaded contexts.
        this->disconnect_all();
    }

    MyClass() {

    }

    void sum(int a, int b) {
        std::cout << "a + b = " << a + b << std::endl;
    }

    void test() {

    }
};

class MySharedClass : public std::enable_shared_from_this<MySharedClass> {
public:
    ~MySharedClass() {}

    void sum(int a, int b) {
        std::cout << "a + b = " << a + b << std::endl;
    }
};

void func(MyParam p){
    cout << "Hello World!"<< endl;
}

int main()
{
    TQMgr->create({"worker"});

    sigslot::signal<> sig;

    MyClass myc;
    sig.connect(&myc, &MyClass::test, sigslot::queued_connection, TQ("worker"));

    auto start = std::chrono::steady_clock::now();

    const int32_t times = 1000000;

    for (int i = 0; i < times; ++i) {
         sig();
    }

    auto end = std::chrono::steady_clock::now();

    double duration_millsecond = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << duration_millsecond << "ms" << std::endl;

    // sigslot::signal<MyParam> sig;

    // sig.connect(func, sigslot::queued_connection, TQ("worker"));

    // MyParam param;
    // sig(param);

    // MyClass myc;

    // sigslot::signal<int, int> sum;

    // sum.connect(&myc, &MyClass::sum, sigslot::queued_connection, TQ("worker"));

    // sum(3, 5);

    // sum.disconnect_all();

    // MySharedClass mysc;

    // sum.connect(&mysc, &MySharedClass::sum, sigslot::blocking_queued_connection, TQ("worker"));

    // sum(3, 7);

    // sum.disconnect_all();

    // sum.connect_extended([](sigslot::connection& conn, int a, int b){
    //     conn.disconnect();
    //     std::cout << "connect_extended, a + b = " << a + b << std::endl;
    // });

    // sum(1, 2);

    // {
    //     auto soped_connection = sum.connect_scoped([](int a, int b){
    //         std::cout << "connect_scoped, a + b = " << a + b << std::endl;
    //     });
    // }

    // sum(2, 3);

    std::this_thread::sleep_for(std::chrono::seconds(300));

    return 0;
}
