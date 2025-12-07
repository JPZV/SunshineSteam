// Standard Includes
#include <cstring>
#include <iostream>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#ifdef _WIN32
    #include <windows.h>
#endif

// Project Includes
#include "constants.hpp"
#include "main.hpp"
#include "process_monitor.hpp"
#include "version.hpp"

// Functions
bool ParseArgs(int argc, char *argv[]);
void PrintHelp();
#ifdef _WIN32
BOOL WINAPI CTRLHandler(DWORD dwType);
#endif
void SignalHandler(int signum);

// Globals
char steam_id[STEAM_APP_ID_LENGTH];
char game_path[MAX_PATH_LENGTH];
bool running = true;
bool found = false;
unsigned long timeout_limit = DEFAULT_TIMEOUT;
ProcessMonitor *monitor = new ProcessMonitor();

int main(int argc, char *argv[])
{
    if (!ParseArgs(argc, argv))
    {
        return 1;
    }

    monitor->onStart = [](int pid)
    {
        printf("Game started with pid %d\n", pid);
        found = true;
    };

    monitor->onExit = []()
    {
        printf("Game fully exited by its own\n");
        running = false;
    };

#ifdef _WIN32
    if (!SetConsoleCtrlHandler(CTRLHandler, TRUE))
    {
        printf("ERROR: Couldn't install Control Handler\n");
        return 2;
    }
#endif
    signal(SIGINT, SignalHandler);

    monitor->start(game_path);

    char steamCmd[STEAM_CMD_LENGTH];
#ifdef _WIN32
    snprintf(steamCmd, STEAM_CMD_LENGTH, "start steam://rungameid/%s", steam_id);
#else
    snprintf(steamCmd, STEAM_CMD_LENGTH, "setsid steam steam://rungameid/%s", steam_id);
#endif

    int ret = system(steamCmd);
    if (ret != 0)
    {
        printf("ERROR: Couldn't launch game. Is Steam Installed?\n");
        return 3;
    }

    unsigned long findingTime = 0;
    while (running)
    {
        if (!found && timeout_limit != 0 && ++findingTime > timeout_limit)
        {
            printf("ERROR: Time-out while launching and waiting the game.\n");
            printf("Check the Game Executable path if the game did actually launched\n");
            return 4;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

bool ParseArgs(int argc, char *argv[])
{
    if (argc < 3)
    {
        PrintHelp();
        return false;
    }
    uint8_t currentArg = 0;
    for (uint8_t i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
        {
            PrintHelp();
            return false;
        }
        else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--timeout"))
        {
            if (i + 1 >= argc)
            {
                printf("Missing value for timeout\n");
                return false;
            }
            timeout_limit = std::stoul(argv[++i]);
        }
        else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version"))
        {
            printf("Current version: %s\n", VERSION);
        }
        else
        {
            switch (currentArg++)
            {
                case 0:
                    strncpy(steam_id, argv[i], STEAM_APP_ID_LENGTH);
                    break;
                case 1:
                    strncpy(game_path, argv[i], MAX_PATH_LENGTH);
                    break;
                
                default:
                    printf("ERROR: Unknown option: \"%s\"\n", argv[i]);
                    PrintHelp();
                    return false;
            }
        }
    }
    if (currentArg == 0)
    {
        printf("ERROR: Missing Game Id\n");
        PrintHelp();
        return false;
    }
    else if (currentArg == 1)
    {
        printf("ERROR: Missing Game executable\n");
        PrintHelp();
        return false;
    }
    return true;
}

void PrintHelp()
{
#ifdef _WIN32
    printf("Usage: SunshineSteam.exe [OPTIONS] GAME_ID GAME_EXE\n");
    printf("Example: SunshineSteam.exe 698780 \"G:\\steamapps\\common\\Doki Doki Literature Club\\lib\\windows-i686\\DDLC.exe\"\n");
#else
    printf("Usage: SunshineSteam [OPTIONS] GAME_ID GAME_EXE\n");
    printf("Example: SunshineSteam 698780 \"/home/me/steam/steamapps/common/Doki Doki Literature Club/lib/linux-x86_64/DDLC\"\n");
#endif
    printf("\nOptions:\n");
    printf("\t-h\t--help\t\tPrint this text\n");
    printf("\t-t\t--timeout\t\tSet timeout in seconds for waiting the game to launch (0 to disable timeout)\n");
    printf("\t-v\t--version\t\tPrint current version\n");
    printf("\nTip: Use your Task Manager to find the real GAME_EXE, as it's not always at the main directory\n");
}

#ifdef _WIN32
BOOL WINAPI CTRLHandler(DWORD dwType)
{
    printf("Received control %d\n", dwType);
    if (dwType == CTRL_C_EVENT)
    {
        monitor->killAll();
        return TRUE;
    }
    return FALSE;
}
#endif
void SignalHandler(int signum)
{
    printf("Received signal %d\n", signum);
    monitor->killAll();
    exit(signum); // Is it necessary?
}