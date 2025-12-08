// Standard includes 
#include "process_monitor.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <filesystem>

#ifdef _WIN32
    #include <windows.h>
    #include <tlhelp32.h>
#else
    #include <dirent.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <signal.h>
#endif

//Project includes

#include "constants.hpp"

using namespace std;

static string toLowerCopy(const string& s)
{
    string r = s;
    transform(r.begin(), r.end(), r.begin(), [](unsigned char c)
    {
        return std::tolower(c);
    });
    return r;
}

ProcessMonitor::ProcessMonitor(unsigned pollMs)
    : pollIntervalMs(pollMs),
      running(false)
{
}

ProcessMonitor::~ProcessMonitor()
{
    stop();
}

void ProcessMonitor::start(const string& exePath)
{
    if (running) return;
    running = true;
    targetExe = std::filesystem::absolute(exePath).string();
#ifdef _WIN32
    targetExe = toLowerCopy(targetExe);
#endif
    worker = thread(&ProcessMonitor::loop, this);
}

void ProcessMonitor::stop()
{
    running = false;
    if (worker.joinable()) worker.join();
}

void ProcessMonitor::loop()
{
    while (running)
    {
        auto procs = listProcesses(targetExe);

        unordered_map<int, bool> newSet;
        for (auto& [pid, path] : procs)
        {
            newSet[pid] = true;
            if (!tracked.count(pid))
            {
                tracked[pid] = true;
                if (onStart) onStart(pid);
            }
        }

        for (auto it = tracked.begin(); it != tracked.end();)
        {
            int pid = it->first;
            if (!newSet.count(pid))
            {
                it = tracked.erase(it);
                if (onExit && it == tracked.end()) onExit();
            }
            else
            {
                ++it;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(pollIntervalMs));
    }
}

bool ProcessMonitor::killAll()
{
    for (auto it = tracked.begin(); it != tracked.end();)
    {
        int pid = it->first;
        goto start_loop;

cannot_kill:
        printf("WARN: Couldn't kill process %d\n", pid);
        ++it;
        continue;

start_loop:
#ifdef _WIN32
        HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (!h) goto cannot_kill;
        BOOL ok = TerminateProcess(h, 0);
        CloseHandle(h);
        if (!ok) goto cannot_kill;
#else
        if (kill(pid, SIGKILL) != 0) goto cannot_kill;
#endif
        ++it;
    }
    return true;
}

#ifdef _WIN32
std::unordered_map<int, std::string> ProcessMonitor::listProcesses(std::string target)
{
    unordered_map<int, string> result;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return result;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);

    if (Process32First(snapshot, &pe))
    {
        do
        {
            HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
            if (hProc)
            {
                char buffer[MAX_PATH_LENGTH];
                DWORD size = MAX_PATH_LENGTH;
                if (QueryFullProcessImageNameA(hProc, 0, buffer, &size))
                {
                    string p = toLowerCopy(std::filesystem::absolute(buffer).string());
                    if (target == "" || p == target)
                    {
                        result[pe.th32ProcessID] = p;
                    }
                }
                CloseHandle(hProc);
            }
        } while (Process32Next(snapshot, &pe));
    }

    CloseHandle(snapshot);
    return result;
}
#else
std::unordered_map<int, std::string> ProcessMonitor::listProcesses(std::string target)
{
    unordered_map<int, string> result;

    DIR* d = opendir("/proc");
    if (!d) return result;

    struct dirent* ent;
    while ((ent = readdir(d)))
    {
        if (!isdigit(ent->d_name[0])) continue;

        int pid = atoi(ent->d_name);

        string exePath = "/proc/" + string(ent->d_name) + "/exe";
        char buf[MAX_PATH_LENGTH];
        ssize_t len = readlink(exePath.c_str(), buf, sizeof(buf) - 1);
        if (len <= 0) continue;

        buf[len] = 0;
        string path = std::filesystem::absolute(buf).string();

        if (target == "" || path == target)
        {
            result[pid] = path;
        }
    }

    closedir(d);
    return result;
}
#endif

void ProcessMonitor::printProcesses()
{
    auto procs = listProcesses("");
    for (auto& [pid, path] : procs)
    {
        printf("[%d] => %s\n", pid, path.c_str());
    }
}