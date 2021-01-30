#pragma once

#include "image_compressor.hpp"
#include "task_queue.hpp"

namespace retrojsvice {

class WindowEventHandler {
public:
    // Called when window closes itself (i.e. is not closed by a call to
    // Window::close()). The window is immediately closed as if Window::close
    // was called.
    virtual void onWindowClose(uint64_t window) = 0;

    // See ImageCompressorEventHandler::onImageCompressorFetchImage
    virtual void onWindowFetchImage(
        uint64_t window,
        function<void(const uint8_t*, size_t, size_t, size_t)> func
    ) = 0;

    virtual void onWindowResize(
        uint64_t window,
        size_t width,
        size_t height
    ) = 0;

    virtual void onWindowMouseDown(uint64_t window, int x, int y, int button) = 0;
    virtual void onWindowMouseUp(uint64_t window, int x, int y, int button) = 0;
    virtual void onWindowMouseMove(uint64_t window, int x, int y) = 0;
    virtual void onWindowMouseDoubleClick(uint64_t window, int x, int y, int button) = 0;
    virtual void onWindowMouseWheel(uint64_t window, int x, int y, int delta) = 0;
    virtual void onWindowMouseLeave(uint64_t window, int x, int y) = 0;

    virtual void onWindowKeyDown(uint64_t window, int key) = 0;
    virtual void onWindowKeyUp(uint64_t window, int key) = 0;

    virtual void onWindowLoseFocus(uint64_t window) = 0;
};

class HTTPRequest;
class SecretGenerator;

// Must be closed before destruction (as signaled by the onWindowClose, caused
// by the Window itself or initiated using Window::close)
class Window :
    public ImageCompressorEventHandler,
    public enable_shared_from_this<Window>
{
SHARED_ONLY_CLASS(Window);
public:
    Window(CKey,
        shared_ptr<WindowEventHandler> eventHandler,
        uint64_t handle,
        shared_ptr<SecretGenerator> secretGen,
        string programName
    );
    ~Window();

    // Immediately closes the window (no more event handlers will be called and
    // no member functions may be called for this window). Does not call
    // WindowEventHandler::onWindowClose.
    void close();

    void handleInitialForwardHTTPRequest(shared_ptr<HTTPRequest> request);
    void handleHTTPRequest(MCE, shared_ptr<HTTPRequest> request);

    void notifyPopupCreated(shared_ptr<Window> popupWindow);

    void notifyViewChanged();

    void setCursor(int cursorSignal);

    // ImageCompressorEventHandler:
    virtual void onImageCompressorFetchImage(
        function<void(const uint8_t*, size_t, size_t, size_t)> func
    ) override;

private:
    void afterConstruct_(shared_ptr<Window> self);

    // Closes window and calls WindowEventHandler::onWindowClose.
    void selfClose_(MCE);

    void updateInactivityTimeout_(bool shorten = false);
    void inactivityTimeoutReached_(MCE, bool shortened);

    bool handleTokenizedEvent_(MCE,
        const string& name,
        int argCount,
        const int* args
    );
    bool handleEvent_(MCE,
        string::const_iterator begin,
        string::const_iterator end
    );
    void handleEvents_(MCE, uint64_t startIdx, string eventStr);

    void navigate_(int direction);

    void handleMainPageRequest_(MCE, shared_ptr<HTTPRequest> request);
    void handleImageRequest_(
        MCE,
        shared_ptr<HTTPRequest> request,
        uint64_t mainIdx,
        uint64_t imgIdx,
        int immediate,
        int width,
        int height,
        uint64_t startEventIdx,
        string eventStr
    );
    void handlePrevPageRequest_(shared_ptr<HTTPRequest> request);
    void handleNextPageRequest_(shared_ptr<HTTPRequest> request);

    string programName_;

    shared_ptr<WindowEventHandler> eventHandler_;
    uint64_t handle_;
    string csrfToken_;
    string pathPrefix_;
    bool closed_;

    shared_ptr<ImageCompressor> imageCompressor_;
    shared_ptr<DelayedTaskTag> animationTag_;

    int width_;
    int height_;

    set<int> mouseButtonsDown_;
    set<int> keysDown_;

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

    shared_ptr<DelayedTaskTag> inactivityTimeoutTag_;

    steady_clock::time_point lastNavigateOperationTime_;
};

}
