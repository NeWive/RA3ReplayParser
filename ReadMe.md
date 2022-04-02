# RAT-RA3-Replay-Parser

用于解析红色警戒三(Command & Conquer: Red Alert 3)的回放文件 `.RA3Replay`

以JSON字符串形式输出，计划后续编译为Webassembly以备后续使用。

后续会优化代码

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

## 感谢

感谢远古大佬 [louisdx (Louis Delacroix) (github.com)](https://github.com/louisdx)制作的RA3 Replay文件格式文档。
