# Sunshine Steam

This is a helper program to open Steam Games on Sunshine and having full control of it. With this, if the game closes, the streaming will be terminated too, or if you quit the game from the client app (like moonlight), the game will be terminate on the host side too.

## Supported platforms

Currently this project only works in Windows and Linux (tested on Ubuntu, but it should work on any distro). While Steam and Sunshine works on MacOS, I don't have any way to test this program there, but I'm open to contributions from Mac users.

## Installation

Download the latest version available in the [Releases](https://github/JPZV/sunshinesteam/releases/latest) for your system and put it anywhere PATH is looking at (for example, `C:\Windows\System32` or `/usr/bin/`).

## Usage

Within Sunshine admin page (typically https://localhost:47990), go to Apps and add a new game of you desire, then on Command put

```shell
# For Windows
SunshineSteam.exe {GAME_ID} {GAME_EXE}
# For Linux
SunshineSteam {GAME_ID} {GAME_EXE}
```

Where `{GAME_ID}` is the Game's ID (found in Steam > Library > Your game > Settings > Updates > Application ID) and `{GAME_EXE}` is the Game's executable.

Keep in mind that many games uses launchers within the Steam itself. For example, the game Doki Doki Literature Club has its launcher in Windows at `...\steamapps\common\Doki Doki Literature Club\DDLC.exe`, but the actual game's executable is in `...\steamapps\common\Doki Doki Literature Club\lib\windows-i686\DDLC.exe`

So for example this should be the real command:

```shell
# For Windows
SunshineSteam.exe 698780 "C:\Program Files (x86)\Steam\steamapps\common\Doki Doki Literature Club\lib\windows-i686\DDLC.exe"

# For Linux
SunshineSteam 698780 "/home/me/Steam/steamapps/common/Doki Doki Literature Club/lib/linux-x86_64/DDLC"
```

You can use your task manager to look at the real path while the game is running

### Usage outside Sunshine

As this is an external and independent program, you can use it outside of sunshine as a Steam-Game Launcher in a Sync-way, as the program will not terminate until the game does. You just have to execute it as the same way as Sunshine does:

```shell
# For Windows
SunshineSteam.exe {GAME_ID} {GAME_EXE}
# For Linux
SunshineSteam {GAME_ID} {GAME_EXE}
```

## Contributing

### Requirements

You need to have Cmake and GNU toolchain (for Windows MinGW is recommended). Also Visual Studio Code with Cmake extension is optional but recommended for fast compiling and debugging

### Compiling

This depends of your setting and environment, but if you're using VSCode, you need to create a `.vscode/settings.json` file with the following:

```json
{
    "cmake.debugConfig": {
        "args": [
            "{GAME_ID}",
            "{GAME_EXE}"
        ]
    }
}
```

Where `{GAME_ID}` is the Game's id and `{GAME_EXE}` is the Game's executable. Then you just need to press Shift+F5 to start debugging (keep in mind that CTRL+C may not work. If you want to test the close-the-game feature, you need to run the program from the console manually).

On the other hand, if you don't want to use VSCode, then you have to build it and run it manually with:

```shell
mkdir build && cd build # ignore the first time
cmake .. && make
./SunshineSteam {GAME_ID} {GAME_EXE}
```

## Known issues

- Sometimes the streaming will not terminate when the game closes itself. This seems to happen when Steam re-open itselfs instead of using the already-opened instance.
- SunshineSteam seems to not get the SIGIN when it's being closed from the client/moonlight at least on Windows.