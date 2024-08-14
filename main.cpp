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

// Slot object with shared_ptr
class UiController1 {
public:
    ~UiController1() {}

    void onDevicePlugged(const std::shared_ptr<DeviceInfo>& info) {
        std::cout << "onDevicePlugged: " << info->deviceName << std::endl;
    }
};

// Slot object with observer
class UiController2 : public sigslot::observer {
public:
    ~UiController2() {
        // Needed to ensure proper disconnection prior to object destruction in multithreaded contexts.
        this->disconnect_all();
    }

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

    auto dc = std::make_shared<DeviceController>();

    // example 1, slot object with shared_ptr
    auto ui1 = std::make_shared<UiController1>();
    dc->pluggedSignal.connect(ui1.get(), &UiController1::onDevicePlugged, sigslot::connection_type::auto_connection, TQ("worker"));

    // example 2, slot object with observer
    UiController2 ui2;
    dc->pluggedSignal.connect(&ui2, &UiController2::onDevicePlugged, sigslot::connection_type::queued_connection, TQ("worker"));

    dc->mockCallback();

    // expamle 3
    sigslot::signal<int> printSignal;

    printSignal.connect([](int x){
        std::cout << "Hello World: " << x << std::endl;
    }, sigslot::connection_type::blocking_queued_connection, TQ("worker"));

    printSignal(5);

    return 0;
}
