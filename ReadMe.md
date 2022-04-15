# RA3-Replay-Parser

用于解析红色警戒三(Command & Conquer: Red Alert 3)的回放文件 `.RA3Replay`

以JSON字符串形式输出，计划后续编译为Nodejs原生模块以备后续使用。

现在是屎山demo，后续会优化

## RA3 Replay文件格式

### 主体结构

`.ra3replay` 包含以下的结构：

-   Header
-   Chunks(长度不固定)
-   End-of-chunks terminator
-   Footer

### 主要数据类型

-   `char`: 长度一个字节，用来表示纯文本
-   `byte`: 长度一个字节，用来表示数值变量
-   `uint16_t`: 无符号的两字节长的整型，以小端排序存储
-   `uint32_t`: 无符号的四字节长的整型，以小段排序存储
-   `tb_ch`: 无符号的双字节值，用于表示BMP Unicode码位**(?)**，以小端排序存储
-   `tb_str`: 双字节小端排序的unicode字符串，以两个separator结尾

-   `player_t`: 玩家部分信息
    -   `player_id`: `uint32_t`
    -   `player_name`: `tb_str`
    -   `team_number`: `byte`

### 常量

-   `MAGIC_SIZE` = 17
-   `U1_SIZE` = 31
-   `U2_SIZE` = 20

### Header部分格式

值得注意一点，录像的Header部分在多人游戏和遭遇战的情况下有些许不同，根据字段 `hnumber` 判断是否是多人游戏。

字段如下(顺序从前到后):

-   `str_magic`: `char`, 固定为 `RA3 REPLAY HEADER`，长度为 `MAGTIC_SIZE` 字节

-   `hnumber1`: `byte`, 若为遭遇战(skirmish)则值为 `0x04`, 若为多人游戏则值 `0x05`==(待验证)==

-   `vermajor`: `uint32_t`

-   `verminor`: `uint32_t`

-   `buildmajor`: `uint32_t`

-   `buildminor`: `uint32_t`

-   `hnumber2`: `byte`, 有评论值为 `0x1E`, 无评论值为 `0x06`

-   `zero1`: `byte`, 值固定为 `0x00`, ==但实际有`0x20`的情况==，根据版本号不同分隔符也不同

-   `match_title`:  `tb_str`，初步观察 `0x E0 65 F9 5B 18`

-   `match_description`: `tb_str`

-   `match_map_name`: `tb_str`

-   `match_map_id`: `tb_str`

-   `number_of_players`: `byte`

-   `player_data`: `player_t[number_ofplayers + 1]`

    -   uint32_t  player_id
    -    tb_str    player_name
    -    IF hnumber1 == 0x05  byte    team_number
    -    ENDIF

-   `offset`: `uint32_t`, 是介于第一个chunk的开始和`str_repl_magic` 的开始的分隔符？

-   `str_repl_leng`: `uint32_t`，固定为`0x08`

-   `str_repl_magic`: `char[str_repl_leng]`

-   `mod_info`: `char[22]`, 默认为 `RA3`

-   `timestamp`: `uint32_t`, 是GMT格式的标准Unix时间戳

-   `unknow1`: `byte[U1_SIZE]`, 全零, 文档为31，==但实测为35，且会出现非全0的情况==

-   `header_len`: `uint32_t`

-   `header`: `char[header_len]`

-   `replay_saver`: `byte`, 是从0开始的玩家数组中保存录像者的索引号

-   `zero3`: `uint32_t`, `0x00000000`

-   `zero4`: `uint32_t`, `0x00000000`

-   `filename_length`: `uint32_t`

-   `filename`: `tb_ch[filename_length]`

-   `date_time`: `tb_ch[8]`

    >   根据位置对应数字有以下意义：
    >
    >   0: year, 1: month, 2: weekday(0-6=Sun-Sat), 3: day, 4: hour, 5: minute, 6: second, 7: unknown

-   `vermagic_len`: `uint32_t`

-   `vermagic`: `char[vermagic_len]`, 包含了版本号

-   `magic_hash`: `uint32_t`, not clear

-   `zero4`: `byte`, `0x00`

其中，Header的主体是一段包含以键值对出现的信息的纯文本序列，其含义如下：

-   `M=`

    -   `unknown`: `short`
    -   `MapName`, 地图名: `char[]`

-   `MC=`

    -   `Map CRC`, 地图CRC校验码?: `int`

-   `MS=`

    -   `Map File Size` ,地图文件大小: `int`

-   `SD=`

    -   `Seed?`: `int`

-   `GSID=`

    -   `GameSpy (Match) ID`: `short`

-   `GT=`

    -   `unknown`: `int`

-   `PC=`

    -   `Post Commentator`: `int`

-   `RU=`

    -   `Initial Camera Player`: `int`
    -   `Game Speed`: `int`
    -   `Initial Resources`: `int`
    -   `Broadcast Game`: `bool`
    -   `Allow Commentary`: `bool`
    -   `Tape Delay`: `int`
    -   `Random Crates`: `bool`
    -   `Enable VoIP`: `bool`
    -   `unkwn`: `int`, -1
    -   `unkwn`: `int`, -1
    -   `unkwn`: `int`, -1
    -   `unkwn`: `int`, -1
    -   `unkwn`: `int`, -1

-   `S=`

    -   `player name`: `char[]`
        -   H[Human]PlayerName || C(CPU)(E(Easy) || M(Medium) || H(Hard) || B(Brutal))
    -   `IP`: `int`
    -   `unkwn`: `int`
    -   `TT|FT`

    -   `unkwn`: `int`

    -   `Faction`: 1 based, 与ini对应

        >    1: Observer
        >
        >    3: Commentator
        >
        >    7: Random
        >
        >    2: Empire
        >
        >    4: Allies
        >
        >    8: Soviets

    -   `unkwn`: `int`

    -   `unkwn`: `int`

    -   `unkwn`: `int`

    -   `unkwn`: `int`

    -   `unkwn`: `int`

    -   `Clan tag`: `char[]`

特殊情况：

-   MS=
    -   `File Size` = 0, 第三方地图
-   `GSID`
    -   `GameSpy (Match) ID` = `0x5D91`, 遭遇战情况
-   `RU`
    -   `Initial Camera Player`, 1 based ?

## Body

`.ra3replay`的Body部分由多个 `chunk` 构成，每个 `chunk` 的结构如下：

-   `time_code`: `uint32_t`

-   `chunk_type`: `byte`

    >   值为1、2、3、4

-   `chunk_size`: `uint32_t`

-   `data`: `byte[chunk_size]`

-   `zero`: `uint32_t`

    >   固定为0x00000000

连续`chunk`之前存在先序后序的关系，必须按照顺序连续读取，其中最后一个 `chunk` 的 `time_code` 的值固定为 `0x7FFFFFFF`

一个 `chunk` 的 `time_code` 对应两帧(1/15 秒)

### `chunk` 的含义

`chunk`的类型根据 `chunk_type` 判断

其中 `chunk_type=3` 和 `chunk_type=4` 只出现在包含 `commentary track` 的回放中，`type=3` 包含音频数据，`type=4` 包含 `telestrator data`~~👴不知道咋翻译~~

`chunk_type=1` 和 `chunk_type=2` 的 `data` 字段的首位均为1

#### `chunk_type=1` 的`data` 字段的格式

-   `default`: `byte`

    >   默认为1

-   `number_of_commands`: `uint32_t`

-   `payload`: `byte[chunk_size-5]`

其中 `payload` 字段包含命令个数，每个命令之间使用 `0xFF` 进行分隔

命令的格式: 

-   `command_id`: `byte`

-   `player_id`: `byte`

-   `code`: `byte[command_size - 3]`

-   `terminator`: `byte`

    >   固定为0xFF

其中 `command_size` 由`command_id` 决定

-   部分命令 `command_size` 固定
-   部分长度可变但是具有标准布局(standard layout)，这个布局依赖于一个单一的偏移量 `n`

Standard layout 命令

-   payload[0]: CommandID

-   payload[1]: PlayerID

    >   换算算法
    >
    >   player index = player_id /8 - k, k = 2

-   如果n > 2，则

    >   1.  Here comes a loop: Let`x`be a byte, and set `x = payload[n]`;
    >       -   If `x == 0xFF` stop, you have reached the end of the command.
    >       -   Let `c = (x >> 4) + 1;`, i.e. take the upper four bits of `x` and add one.
    >       -   Read in `c` values that are 32-bit integers.
    >       -   Read in one more byte and assign it to `x`, and repeat the loop.

命令具体信息见文件末尾

#### `chunk_type=2` 的`data` 字段的格式(没怎么读明白)

-   `default1`: `byte`

    >   固定为1

-   `default2`: `byte`

    >   固定为0

-   `n`: `uint32_t`

    >   玩家的索引号，对应于Header中的玩家信息列表

-   `default3`: `byte`

    >   固定为0x0F

-   `time_code`: `uint32_t`

-   `payload`: `byte[chunk_size - 11]`

据观察 `type=2` 的 `chunk` 只在 `chunk_size=24` 或 `chunk_size=40` 时出现。

从 `payload + 12` 位置开始的 `chunk data`由3或7个32位的IEEE 754的浮点型变量构成。前三个与以英尺为单位的地图坐标相关，最后四个决定了相机视角的角度和缩放。

```
byte      flags;        // 0x01: position, 0x02: rotation
IF flags & 0x01
  float32 position[3];  // (x, z, height) in units of feet
ENDIF
IF flags & 0x02
  float32 rotation[4];  // quaternion components?
ENDIF
```

目前最明确的是该类的 `chunk` 包含 视角移动数据，玩家数量和 `heartbeat`

#### `heartbeats` 类型

几种类型的回放数据是自动生成的，并且不包含任何用户操作，因此统一称为 `heartbeat`，其数据的具体意义是未知的，但是它明确用于确认多人个玩家之间游戏状态的完整性。

每个活跃的玩家每秒和最开始的时候创建一个 `chunk_type=2` 的 `chunk`，这个 `chunk` 的长度固定为40字节，其包含在这个时间点完整的相机视角配置。

在 `chunk_type=1`的 `chunk` 中也存在 `heatbeat`: 每三秒当时间码的形式是45k+1(?)时，每个玩家触发一次ID为 `0x21` 的 `heatbeat`命令。



| command_id | command_size | Description                                                  |
| :--------- | :----------- | :----------------------------------------------------------- |
| 0x00       | 45           | Harder secundary ability, like the bunker of Soviet Combat Engineer?? |
| 0x01       | special      | For example at the end of every replay; shows the creator of the replay; also observed in other places. |
| 0x02       | special      | Set rally point.(设置集结点)                                 |
| 0x03       | 17           | Start/resume research upgrade.(开始/继续研究升级)            |
| 0x04       | 17           | Pause/cancel research upgrade.(暂停/取消研究升级)            |
| 0x05       | 20           | Start/resume unit production.(开始/继续单位生产)             |
| 0x06       | 20           | Pause/cancel unit production.(暂停/取消单位生产)             |
| 0x07       | 17           | Start/resume building construction. (Allies and Soviets only, Empire Cores are treated as units.)(开始/继续建筑建造，帝国核心被当作单位处理) |
| 0x08       | 17           | Pause/cancel building construction.(取消/暂停建筑建造)       |
| 0x09       | 35           | Place building on map (Allies and Soviets only).             |
| 0x0A       | std: 2       | Sell building.(售卖建筑)                                     |
| 0x0C       | special      | Possibly ungarrison?(可能是取消占据房屋)                     |
| 0x0D       | std: 2       | Attack.(攻击)                                                |
| 0x0E       | std: 2       | Force-fire.(强制攻击)                                        |
| 0x0F       | 16           |                                                              |
| 0x10       | special      | Garrison a building.(占据房屋)                               |
| 0x12       | std: 2       |                                                              |
| 0x14       | 16           | Move units.(移动单位)                                        |
| 0x15       | 16           | Attack-move units.(警戒移动)                                 |
| 0x16       | 16           | Force-move units.(强制移动？)                                |
| 0x1A       | std: 2       | Stop command.(停止指令)                                      |
| 0x1B       | std: 2       |                                                              |
| 0x21       | 20           | A heartbeat that every player generates at 45`n` + 1 frames (every 3 seconds). |
| 0x28       | std: 2       | Start repair building.(开始维修建筑)                         |
| 0x29       | std: 2       | Stop repair building.(停止维修建筑)                          |
| 0x2A       | std: 2       | ‘Q’-select.                                                  |
| 0x2C       | 29           | Formation-move preview.(预览队形？)                          |
| 0x2E       | std: 2       | Stance change.                                               |
| 0x2F       | std: 2       | Possibly related to waypoint/planning?                       |
| 0x32       | 53           | Harder Security Point usage like Surveillance Sweep.         |
| 0x33       | special      | Some UUID followed by an IP address plus port number.        |
| 0x34       | 45           | Some UUID.                                                   |
| 0x35       | 1049         | Player info?                                                 |
| 0x36       | 16           |                                                              |
| 0x37       | std: 2       | “Scrolling”, an irregularly, automatically generated command. |
| 0x47       | std: 2       | Unknown, always appears in logical frame 5, and than this logical frame contains this command equally as the number of players. |
| 0x48       | std: 2       |                                                              |
| 0x4B       | special      | Place beacon.                                                |
| 0x4C       | std: 2       | Delete beacon (F9 has something to do with this??).          |
| 0x4D       | ???          | Place text in beacon.                                        |
| 0x4E       | std: 2       | Player power (Secret Protocols).                             |
| 0x52       | std: 2       |                                                              |
| 0x5F       | 11           |                                                              |
| 0xF5       | std: 5       | Drag a selection box and/or select units.                    |
| 0xF6       | std: 5       | Unknown. You get this command when building a Empire Dojo Core and deploying it. Than it should appear once, no idea what it does. |
| 0xF8       | std: 4       | Left mouse button click.                                     |
| 0xF9       | std: 2       |                                                              |
| 0xFA       | std: 7       | Create group.                                                |
| 0xFB       | std: 2       | Select group.                                                |
| 0xFC       | std: 2       |                                                              |
| 0xFD       | std: 7       |                                                              |
| 0xFE       | std: 15      | Simple use of secundary ability, like those of War Bear, Conscript and Flaktrooper. |
| 0xFF       | std: 34      | Simple select and klick Security Point usage like Sleeper Ambush. |

## 感谢

感谢远古大佬 [louisdx (Louis Delacroix) (github.com)](https://github.com/louisdx)制作的RA3 Replay文件格式文档。
