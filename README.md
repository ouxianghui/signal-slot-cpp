# signal-slot component, a replacement for Qt's signal-slot system, base on [sigslot](https://github.com/palacaze/sigslot)

### 1.Main Features
#### a.Supported Connection Types
![connection_types](https://github.com/ouxianghui/signal-slot-cpp/assets/4726906/f328c1a9-98ab-4d36-8d5e-b17f4b64e777)

#### b.Supported Slot Types
![slot_types](https://github.com/ouxianghui/signal-slot-cpp/assets/4726906/cf59b4ae-3b2f-45fe-a27d-bc9120c514d4)

#### c.Slots Execution Order
![exection_order](https://github.com/ouxianghui/signal-slot-cpp/assets/4726906/cfb93013-8235-4c0b-a282-279eae16307f)

### 2.Paramaters Copy Times
#### a.Pass By Value
![pass_by_value](https://github.com/ouxianghui/signal-slot-cpp/assets/4726906/60e3f2f3-a52f-41f9-a488-fa26e0c1fe98)

#### b.Pass By Reference
![pass_by_reference](https://github.com/ouxianghui/signal-slot-cpp/assets/4726906/d6a9209c-b78a-480d-b9ff-0860f8c2ad65)

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
![member_function](https://github.com/ouxianghui/signal-slot-cpp/assets/4726906/405534f6-e353-426c-9868-3e26b4160c37)

##### Lambda:
![lambda](https://github.com/ouxianghui/signal-slot-cpp/assets/4726906/415c84e3-6c04-42fa-8265-ed687e39aba2)

### 4.Examples

```
#include <iostream>
#include "signal.hpp"
#include "task_queue.h"
#include "task_queue_manager.h"

using namespace std;

struct DeviceInfo {
    std::string deviceId;
    std::string deviceName;
};

class UiController {
public:
    onDevicePlugged(const std::shared_ptr<DeviceInfo>& info);
};

class DeviceController {
public:
    std::list<std::shared_ptr<DeviceInfo>> getDeviceList();

private:
    void onDeviceEventTriggered(const std::shared_ptr<DeviceInfo>& info) {
        m_pluggedSignal(info);
    }

public:
    rcv::signal<const std::shared_ptr<DeviceInfo>&> pluggedSignal;
};

int main() {
    TQMgr->create({"worker"});

    {
        auto ui = std::make_shared<UiController>();
        auto dc = std::make_shared<DeviceController>();
        dc->pluggedSignal.connect(&ui, &UiController::onDevicePlugged, sigslot::connection_type::queued_connection, TQ("worker"));
    
        auto deviceInfo = std::make_shared<DeviceInfo>();
        deviceInfo->deviceId = "uuid-12345678900987654321";
        deviceInfo->deviceName = "microphone";

        dc->pluggedSignal(deviceInfo);
    }

    {
        sigslot::signal<int> printSignal;
        
        printSignal.connect([](int x){
            cout << "Hello World: " << x << endl;
        }, sigslot::connection_type::blocking_queued_connection, TQ("worker"));
        
        printSignal(5);
    }

    return 0;
}
```

### 5.Dependence
sigslot: https://github.com/palacaze/sigslot

core: The source code for the task queue comes from webrtc, and the namespace has been renamed
