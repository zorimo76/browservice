#include "task_queue.hpp"

namespace retrojsvice {

class WindowEventHandler {
public:
    // Exceptionally, onWindowClose is called directly instead of the task queue
    // to ensure atomicity of window destruction
    // When called, the window is closed immediately (after the call, no more
    // event handlers will be called and none of the member functions of Window
    // may be called; the Window also drops the shared pointer to the event
    // handler).
    virtual void onWindowClose(uint64_t handle) = 0;
};

class HTTPRequest;

// Must be closed before destruction (as signaled by the onWindowClose, caused
// by the Window itself or initiated using Window::close)
class Window {
SHARED_ONLY_CLASS(Window);
public:
    Window(CKey, shared_ptr<WindowEventHandler> eventHandler, uint64_t handle);
    ~Window();

    // Immediately closes the window, calling WindowEventHandler::onWindowClose
    // directly
    void close();

    void handleHTTPRequest(shared_ptr<HTTPRequest> request);

private:
    void navigate_(int direction);

    shared_ptr<WindowEventHandler> eventHandler_;
    uint64_t handle_;
    bool closed_;

    bool prePrevVisited_;
    bool preMainVisited_;
    bool prevNextClicked_;

    // How many times the main page has been requested. The main page mentions
    // its index to all the requests it makes, and we discard all the requests
    // that are not from the newest main page.
    uint64_t curMainIdx_;

    // Latest image index. We discard image requests that do not have a higher
    // image index to avoid request reordering.
    uint64_t curImgIdx_;

    // How many events we have handled for the current main index. We keep track
    // of this to avoid replaying events; the client may send the same events
    // twice as it cannot know for sure which requests make it through.
    uint64_t curEventIdx_;

    steady_clock::time_point lastNavigateOperationTime_;
};

}