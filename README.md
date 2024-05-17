# signal-slot component, a replacement for Qt's signal-slot system, base on [sigslot](https://github.com/palacaze/sigslot)

### Examples

```
#include <iostream>
#include "signal.hpp"
#include "task_queue.h"
#include "task_queue_manager.h"

using namespace std;

class MyClass : public sigslot::observer {
public:
    ~MyClass() {
        // Needed to ensure proper disconnection prior to object destruction
        // in multithreaded contexts.
        this->disconnect_all();
    }

    void sum(int a, int b) {
        std::cout << "a + b = " << a + b << std::endl;
    }
};

class MySharedClass : public std::enable_shared_from_this<MySharedClass> {
public:
    ~MySharedClass() {
    }

    void sum(int a, int b) {
        std::cout << "a + b = " << a + b << std::endl;
    }
};

int main()
{
    TQMgr->create({"worker"});
    
    sigslot::signal<int> sig;
    
    sig.connect([](int x){
        cout << "Hello World: " << x << endl;
    }, sigslot::connection_type::queued_connection, TQ("worker"));
    
    sig(3);

    
    MyClass myc;
    
    sigslot::signal<int, int> sum;
    
    sum.connect(&myc, &MyClass::sum, sigslot::queued_connection, TQ("worker"));
    
    sum(3, 5);
    
    sum.disconnect_all();
    
    MySharedClass mysc;
    
    sum.connect(&mysc, &MySharedClass::sum, sigslot::blocking_queued_connection, TQ("worker"));
    
    sum(3, 7);
    
    sum.disconnect_all();
    
    sum.connect_extended([](sigslot::connection& conn, int a, int b){
        conn.disconnect();
        std::cout << "connect_extended, a + b = " << a + b << std::endl;
    });
    
    sum(1, 2);
    
    {
        auto soped_connection = sum.connect_scoped([](int a, int b){
            std::cout << "connect_scoped, a + b = " << a + b << std::endl;
        });
    }
    
    sum(2, 3);
    
    std::this_thread::sleep_for(std::chrono::seconds(3));

    return 0;
}
```

### Dependence
sigslot: https://github.com/palacaze/sigslot
core: The source code for the task queue comes from webrtc and rename the namespace
