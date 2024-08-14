// class MyClass : public sigslot::observer {
// public:
//     ~MyClass() {
//         // Needed to ensure proper disconnection prior to object destruction
//         // in multithreaded contexts.
//         this->disconnect_all();
//     }

//     MyClass() {

//     }

//     void sum(int a, int b) {
//         std::cout << "a + b = " << a + b << std::endl;
//     }

//     void test() {

//     }
// };


#include <iostream>
#include <list>
#include <memory>
#include "signal.hpp"
#include "core/task_queue.h"
#include "core/task_queue_manager.h"

struct DeviceInfo {
    std::string deviceId;
    std::string deviceName;
};

class UiController : public std::enable_shared_from_this<UiController> {
public:
    ~UiController() {}

    void onDevicePlugged(const std::shared_ptr<DeviceInfo>& info) {
        std::cout << "onDevicePlugged: " << info->deviceName << std::endl;
    }
};

class DeviceController {
public:
    std::list<std::shared_ptr<DeviceInfo>> getDeviceList();

    void mockCallback() {
        auto deviceInfo = std::make_shared<DeviceInfo>();
        deviceInfo->deviceId = "uuid-12345678900987654321";
        deviceInfo->deviceName = "microphone";
        onDeviceEventTriggered(deviceInfo);
    }
private:
    void onDeviceEventTriggered(const std::shared_ptr<DeviceInfo>& info) {
        pluggedSignal(info);
    }

public:
    sigslot::signal<const std::shared_ptr<DeviceInfo>&> pluggedSignal;
};

int main()
{
    // create a task queue
    TQMgr->create({"worker"});

    // example 1
    auto ui = std::make_shared<UiController>();
    auto dc = std::make_shared<DeviceController>();
    dc->pluggedSignal.connect(ui.get(), &UiController::onDevicePlugged, sigslot::connection_type::queued_connection, TQ("worker"));

    dc->mockCallback();


    // expamle 2
    sigslot::signal<int> printSignal;

    printSignal.connect([](int x){
        std::cout << "Hello World: " << x << std::endl;
    }, sigslot::connection_type::blocking_queued_connection, TQ("worker"));

    printSignal(5);

    return 0;
}
