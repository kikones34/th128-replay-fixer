# Touhou 12.8 ~ Great Fairy Wars replay fixer

This tool can fix Th12.8 replays affected by the [wrong route glitch](https://en.touhouwiki.net/wiki/Great_Fairy_Wars/Gameplay#Glitches). It automatically detects the correct route and produces a fixed replay file that can be watched in-game without issues.

Usage is straight-forward, just drag and drop your bugged replays into the EXE:

(TODO: insert video)

Note that the original files will never be modified, instead a new file will be created with the suffix `.fixed.rpy`.

## CLI usage

You can also use the utility through the command line for some extra options.

### Fix replays

Use the `fix` command to fix replays. For example, to automatically fix any bugged replays in the default replays folder, you can run this command:

```batch
th128-replay-fixer.exe fix "%appdata%\ShanghaiAlice\th128\replay\*.rpy"
```

If a replay is not bugged, it will be ignored.

### Parse replay data

This tool also includes a data parsing mode. It will output detailed information about the replay file, including metadata, game config, stage info and all recorded inputs (see [Inputs legend](#inputs-legend) below).

Use the `decode` command to parse replays, for example:

```batch
th128-replay-fixer.exe decode th128_01.rpy
```

This will produce two files:
- `th128_01.rpy.raw`: a binary file containing the raw decoded data
- `th128_01.rpy.txt`: a text file containing the parsed data in a human-readable format (encoded in UTF-8)

It is also possible to extract the strings from the user data section of replay files. This section is not encrypted nor compressed and follows a very simple format, intended to be easy to read and modify by 3rd party tools. The games ignore this section.

Use the `user` command to extract user data, for example:

```batch
th128-replay-fixer.exe user th128_01.rpy
```

This will produce the `th128_01.rpy.user.txt` file, containing the replay information and user comment strings (encoded in Shift-JIS).

#### Inputs legend

- `.` : No input
- `←→↑↓↖↗↘↙` : Movement, self-explanatory
- `S`, `s` : Press / release shoot button
- `F`, `f` : Press / release focus button
- `A`, `a` : Press / release autoshoot button (also dialogue skip)
- `B` : Press bomb button
- `Q` : Quit the run


## Building

You'll need an MSYS2 or Cygwin environment with gcc to compile the code in Windows. Then, simply execute `make`. The executable will be at `build/release/main.exe`.

## How it works

Below is an outline of what the tool does. For more details, check the source code.

1. Decrypt the replay file. Replay files are encrypted with a custom ZUN function that uses XOR masking with a dynamic key.
2. Decompress the data. Replay data is compressed using ZUN's custom implementation of the LZSS algorithm.
3. Analyze stage data to determine the correct route. Even though bugged replays have the incorrect route, they have the correct stages. For example, if the actual route was A2, then the second stage will always be A2-2.
4. Fix replay route. This involves modifying up to three locations:
    - Replay route value (the most important)
    - Last stage value (only necessary if the run is an All Clear)
    - User data route string (inconsequential, but for completeness)
5. Re-compress and re-encrypt the modified data and pack it into an RPY file.

## Afterword and acknowledgements

I spent a fair amount of time studying the available Touhou replay decoders to be able to come up with a re-encoder, which was necessary for this tool to be possible. I also had to understand the structure of the decoded data to know how to fix bugged replays. Meanwhile, I got somewhat obsessed with figuring out the meaning of as much of the data as possible, resulting in the parsing mode, which to my knowledge is the most comprehensive Th12.8 replay data parser available.

However, none of this would have been possible without the efforts of other members in the community who published their code. The following repositories were very helpful for the development of this tool:

- https://github.com/Fluorohydride/threp : original replay decoder. It served as the base for this project.
- https://github.com/raviddog/threp : updated fork of the original decoder. I used the `zuntypes.h` as the base for my `types.h`.
- https://github.com/32th-System/py-tsadecode : contains an alternative implementation of the decompression algorithm. It helped me understand it and fix the original code.
- https://github.com/wz520/thhyl : another replay decoder. It helped me figure out some of the fields in the decoded data.

Other repos worth mentioning:
- https://github.com/n-rook/thscoreboard
- https://github.com/raviddog/threplay
- https://github.com/yiyuezhuo/touhou-replay-decoder
