# signal-slot component, a replacement for Qt's signal-slot system, base on [sigslot](https://github.com/palacaze/sigslot)

### 1.Main Features
#### a.Supported Connection Types
![connection_types@2x](https://github.com/ouxianghui/signal-slot-cpp/assets/4726906/7be1867b-ef78-480e-aac4-feced8652168)

#### b.Supported Slot Types
![slot_types@2x](https://github.com/ouxianghui/signal-slot-cpp/assets/4726906/615c03ce-cded-48be-8779-11068a52e3bb)

#### c.Slots Execution Order
![slot_executions_order](https://github.com/ouxianghui/signal-slot-cpp/assets/4726906/e925fbf6-bd0f-4f07-aa66-7d5749349426)

### 2.Paramaters Copy Times
#### a.Pass By Value
![pass_by_value@2x](https://github.com/ouxianghui/signal-slot-cpp/assets/4726906/bc994b4c-3cad-4d92-a9d2-3ce15f65d0d0)

#### b.Pass By Reference
![pass_by_reference@2x](https://github.com/ouxianghui/signal-slot-cpp/assets/4726906/7a981cec-307b-40c6-8d13-7cfd43ee98f7)

### 3.Performance
#### a.Test environment:

##### CPU: 2.6 GHz 6-Core Intel Core i7

##### Memory：16 GB 2667 MHz DDR4

##### Operating System: macOS 13.6.4 (22G513)

##### Call Times: 1000000 times empty function calls

##### Execution thread: The task queue used is from webrtc (based on the C++ standard thread library)

##### Qt version: 6.2.4

#### b.Test Cases
##### Member Functions:
![performance_memer_function@2x](https://github.com/ouxianghui/signal-slot-cpp/assets/4726906/f621b18c-e201-4e5b-9a5d-da73ca6b967d)

##### Lambda:
![performance_lambda@2x](https://github.com/ouxianghui/signal-slot-cpp/assets/4726906/aa483447-f65b-4ae6-bd36-ea34feab0393)

### 4.Examples

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

### 5.Dependence
sigslot: https://github.com/palacaze/sigslot

core: The source code for the task queue comes from webrtc, and the namespace has been renamed
