#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include <atomic>
#include <chrono>

class ProcessMonitor
{
public:
    using StartCallback = std::function<void(int pid)>;
    using ExitCallback = std::function<void()>;

    ProcessMonitor(
        unsigned pollMs = 500
    );

    ~ProcessMonitor();

    void start(const std::string& exePath);
    void stop();

    bool killAll();

    // Callbacks
    StartCallback onStart;
    ExitCallback onExit;

private:
    std::string targetExe;
    unsigned pollIntervalMs;
    std::atomic<bool> running;
    std::thread worker;

    std::unordered_map<int, bool> tracked;

    void loop();

    // OS-specific
    std::unordered_map<int, std::string> listProcesses();
};