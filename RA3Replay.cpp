//
// Created by dell on 2022/4/1.
//


#include "RA3Replay.h"

unsigned int readUInt32LittleEndian(const unsigned char in[]) {
    return ((unsigned int) ((in)[0] | ((in)[1] << 8) | ((in)[2] << 16) | ((in)[3] << 24)));
}

unsigned int readUInt16LittleEndian(const unsigned char in0, const unsigned char in1) {
    return (((unsigned int) (in1) << 8) | ((unsigned int) (in0)));
}

std::string read2ByteString(std::istream &in, unsigned int separator) {
    CodePoint ccp;
    TBString cbuf;
    std::string s;
    while (true) {
        in.read(reinterpret_cast<char *>(&cbuf), sizeof(TBString));
        unsigned int t = readUInt16LittleEndian(cbuf.byte1, cbuf.byte2);
        if (cbuf.byte1 == separator && cbuf.byte2 == separator) {
            break;
        }
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        s += converter.to_bytes(t);
    }
    return s;
}

std::vector<std::string> tokenize(const std::string & str, const std::string & delimiters)
{
    std::vector<std::string> tokens;

    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }

    return tokens;
}

std::string getFaction(unsigned int f) {
    switch(f)
    {
        case 1:  return "天眼帝国";
        case 3:  return "Commentator";
        case 4:  return "盟军";
        case 8:  return "苏联";
        case 2:  return "帝国";
        case 7:  return "随机";
        case 9:  return "神州";
        default: return "[ERROR]";
    }
}

bool parseReplayFile(char *fileName, std::string &target) {
    auto *headerTemp = (ReplayHeaderTemp *)malloc(sizeof (ReplayHeaderTemp));
    std::string replayTitle, matchDesc, mapName, mapId, replayName, versionMagic, str_anothername;
    size_t size;
    uint32_t timeStamp;
    char cncMagic[8];
    char modInfo[22];
    // 简略信息
    std::vector<int> playerIds1;
    std::vector<std::string> playerNames1;
    std::vector<int> playerTeamIds1;
    // 详细信息
    std::vector<int> playerIds2;
    std::vector<std::vector<std::string>> playerNames2;
    std::vector<int> playerTeamIds2;
    unsigned int hlen;
    std::string mapRealName;
    std::string mapCRC;
    std::string matchSeed;
    std::string postCommentator;
    std::string matchConf;
    std::string playerInfoQuery;


    // 无用的字节
    uint32_t meaningless;
    unsigned char u31[31];
    char timeout[200];
    char playerNumber;
    std::string playerName;
    char teamId;
    uint32_t playerId;

    std::cout << "Opening file: " << fileName << std::endl;
    std::fstream replayFile(fileName, std::ios::in | std::ios::binary);
    if (!replayFile) {
        std::cerr << "Open replay file failed" << std::endl;
        return false;
    }
    // 输出文件大小
    replayFile.seekg(0, std::ios::end);
    size = replayFile.tellg();
    std::cout << "file size: " << size << std::endl;

    replayFile.seekg(0, std::ios::beg);
    replayFile.read(headerTemp->headerFlag, MAGIC_SIZE);
    headerTemp->headerFlag[MAGIC_SIZE] = '\0';
    std::cout << headerTemp->headerFlag << std::endl;
    replayFile.read(reinterpret_cast<char *>(headerTemp->isSkirmish), sizeof(headerTemp->isSkirmish));
    replayFile.read(reinterpret_cast<char *>(headerTemp->majorVersion), sizeof(headerTemp->majorVersion));
    replayFile.read(reinterpret_cast<char *>(headerTemp->minorVersion), sizeof(headerTemp->minorVersion));
    replayFile.read(reinterpret_cast<char *>(headerTemp->majorBuild), sizeof(headerTemp->majorBuild));
    replayFile.read(reinterpret_cast<char *>(headerTemp->minorBuild), sizeof(headerTemp->minorBuild));
    replayFile.read(reinterpret_cast<char *>(headerTemp->hasCommentary), sizeof(headerTemp->hasCommentary));
    replayFile.read(reinterpret_cast<char *>(headerTemp->separator), sizeof(headerTemp->separator));
    // 判定是否是合法的RA3Replay文件
    if (strncmp(headerTemp->headerFlag, "RA3 REPLAY HEADER", 17)
        || ((readUInt32LittleEndian(headerTemp->majorVersion) != 1) && (readUInt32LittleEndian(headerTemp->minorVersion) > 12))) {
        std::cerr << "File does not seem to be a RA3 replay file." << std::endl;
        return false;
    }

    // 读取文件信息
    replayTitle     = read2ByteString(replayFile, headerTemp->separator[0]);
    matchDesc = read2ByteString(replayFile, headerTemp->separator[0]);
    mapName   = read2ByteString(replayFile, headerTemp->separator[0]);
    mapId     = read2ByteString(replayFile, headerTemp->separator[0]);
    std::cout << "录像标题: " << replayTitle << std::endl << "录像备注: " << matchDesc << std::endl << "地图名称: " << mapName << std::endl << "地图ID: " << mapId << std::endl;
    std::cout << "游戏版本: " << std::dec << readUInt32LittleEndian(headerTemp->majorVersion) << "." << readUInt32LittleEndian(headerTemp->minorVersion) << ", Build: " << readUInt32LittleEndian(headerTemp->majorBuild) << "." << readUInt32LittleEndian(headerTemp->minorBuild) << std::endl;
    // 读取玩家数量
    replayFile.read(&playerNumber, 1);
    std::cout << "玩家数量: " << int(playerNumber) << std::endl;

    // 解析玩家信息
    for (int i = 0; i <= int(playerNumber); ++i) {
        replayFile.read(reinterpret_cast<char*>(&playerId), 4);
        playerIds1.push_back(playerId);
        playerName = read2ByteString(replayFile, headerTemp->separator[0]);
        playerNames1.push_back(playerName);
        if (headerTemp->isSkirmish[0] == 5) {
            replayFile.read(reinterpret_cast<char *>(&teamId), 1);
            playerTeamIds1.push_back(int(teamId));
        } else {
            playerTeamIds1.push_back(-1);
        }

        std::cout << "playerName: " << playerName << std::endl;
    }

    // 暂时用不到的字段
    replayFile.read(reinterpret_cast<char*>(&meaningless), 4); // 用于计算从CNC3RPL 到第一个Chunk的偏移量
    replayFile.read(reinterpret_cast<char*>(&meaningless), 4);

    // CNC Replay标志字段
    replayFile.read(cncMagic, 8);
    if (strncmp(cncMagic, "CNC3RPL\0", 8)) {
        std::cerr << "Error: Unexpected content! Aborting." << std::endl;
        return false;
    }
    std::cout << "标志字段: " << cncMagic << std::endl;

    // Mod info
    replayFile.read(modInfo, 22);
    char *p(modInfo), *q(NULL);
    std::vector<std::string> modInfoV;
    while (p < modInfo + 22)
    {
        q = std::strchr(p, '\0');
        if (q == NULL) break;
        if (p[0] != '\0') {
            std::cout << "\"" << p << "\" ";
            modInfoV.emplace_back(p);
        }
        p = q+1;
    }
    std::cout << std::endl;

    //时间
    replayFile.read(reinterpret_cast<char*>(&timeStamp), 4);
    strftime(timeout, 200, "%Y-%m-%d %H:%M:%S (%Z)", gmtime(reinterpret_cast<time_t*>(&timeStamp)));
    std::cout << "时间戳: " << std::dec << timeStamp << ", that is " << timeout << "." << std::endl;

    //跳过无用字节
    replayFile.read(reinterpret_cast<char*>(&u31), 31);

    // Header长度
    replayFile.read(reinterpret_cast<char*>(&hlen), 4);
    if (hlen > 10000) {
        std::cerr << "Requested header length too big." << std::endl;
        return false;
    }

    // 读取头部信息
    std::vector<char> header2(hlen);
    replayFile.read(header2.data(), hlen);
    std::cout << "录像Header长度: " << std::dec << hlen << ". Header 原始数据:" << std::endl
              << std::string(header2.begin(), header2.end()) << std::endl << std::endl;


    // 解析详细信息
    std::string tempS = std::string(header2.begin(), header2.end());
    std::vector<std::string> tokens = tokenize(tempS, ";");
    jsoncons::json playerInfoArr(jsoncons::json_array_arg);
    for(const auto & token : tokens) {
        if (token[0] == 'S' && token[1] == '=') {   // 玩家信息
            playerInfoQuery = token;
            std::vector<std::string> subTokens = tokenize(token.substr(2), ":");
            for (auto & subToken : subTokens) {
                // 跳过非法的玩家名以及电脑玩家
                if (subToken[0] != 'H' && !(subToken.size() > 2 && subToken[0] == 'C' && subToken[2] == ','))
                    continue;
                jsoncons::json playerInfoTemp(jsoncons::json_object_arg);
                std::vector<std::string> sub2Tokens = tokenize(subToken.substr(subToken[0] == 'C' ? 0 : 1), ",");
                if (sub2Tokens.size() < 6)
                    std::cerr << "Invalid player info" << std::endl;
                playerNames2.push_back(sub2Tokens);
                uint32_t v = std::strtoul(sub2Tokens[1].c_str(), NULL, 16);
                if (subToken.size() > 2 && subToken[0] == 'C' && subToken[2] == ',') {
                    fprintf(stdout, "Computer opponent:  %s (Faction: %s) Other data: \"",
                            sub2Tokens[0].c_str(), getFaction(std::atoi(sub2Tokens[2].c_str())).c_str());
                    playerInfoTemp.insert_or_assign("isPlayer", false);
                    playerInfoTemp.insert_or_assign("opponent", sub2Tokens[0]);
                    playerInfoTemp.insert_or_assign("faction", std::atoi(sub2Tokens[2].c_str()));
                    jsoncons::json otherData(jsoncons::json_array_arg);
                    for (size_t j = 1; j < sub2Tokens.size() - 1; ++j) {
                        std::cout << sub2Tokens[j] << ", ";
                        otherData.push_back(sub2Tokens[j]);
                    }
                    playerInfoTemp.insert_or_assign("otherData", otherData);
                } else {
                    fprintf(stdout, "玩家: %s (阵营: %s, IP 地址.: 0x%08X, %d.%d.%d.%d:%s) Other data: \"",
                            sub2Tokens[0].c_str(), getFaction(std::atoi(sub2Tokens[5].c_str())).c_str(), v,
                            v>>24, ((v<<8)>>24), ((v<<16)>>24), ((v<<24)>>24), sub2Tokens[2].c_str());
                    playerInfoTemp.insert_or_assign("isPlayer", true);
                    playerInfoTemp.insert_or_assign("playerName", sub2Tokens[0]);
                    playerInfoTemp.insert_or_assign("faction", std::atoi(sub2Tokens[5].c_str()));
                    jsoncons::json otherData(jsoncons::json_array_arg);
                    for (size_t j = 3; j < sub2Tokens.size() - 1; ++j) {
                        std::cout << sub2Tokens[j] << ", ";
                        otherData.push_back(sub2Tokens[j]);
                    }
                    playerInfoTemp.insert_or_assign("otherData", otherData);
                }
                playerInfoArr.push_back(playerInfoTemp);
                std::cout << sub2Tokens[sub2Tokens.size() - 1] << "\"." << std::endl;
            }
        } else if(token[0] == 'M' && token[1] == '=') {
            mapRealName = token;
        } else if(token[0] == 'M' && token[1] == 'C') {
            mapCRC = token;
        } else if(token[0] == 'S' && token[1] == 'D') {
            matchSeed = token;
        } else if(token[0] == 'R' && token[1] == 'U') {
            matchConf = token;
        } else {

        }
    }
    std::cout << mapRealName << std::endl;
    std::cout << mapCRC << std::endl;
    std::cout << matchSeed << std::endl;
    std::cout << matchConf << std::endl;
    std::cout << playerInfoQuery << std::endl;

    jsoncons::json out;
    out.insert_or_assign("replayTitle", replayTitle);
    out.insert_or_assign("replayDesc", matchDesc);
    out.insert_or_assign("mapName", mapName);
    out.insert_or_assign("mapID", mapId);
    out.insert_or_assign("timestamp", timeStamp);
    out.insert_or_assign("replayFileSize", size);
    out.insert_or_assign("gameMajorVersion", readUInt32LittleEndian(headerTemp->majorVersion));
    out.insert_or_assign("gameMinorVersion", readUInt32LittleEndian(headerTemp->minorVersion));
    out.insert_or_assign("playerNumber", int(playerNumber));

    jsoncons::json modInfoArr(jsoncons::json_array_arg);
    for (const auto & info: modInfoV) {
        modInfoArr.push_back(info);
    }

    out.insert_or_assign("modInfo", modInfoArr);
    out.insert_or_assign("rawHead", tempS);
    out.insert_or_assign("playerInfo", playerInfoArr);
    out.insert_or_assign("mapRealName", mapRealName);
    out.insert_or_assign("mapCRC", mapCRC);
    out.insert_or_assign("matchSeed", matchSeed);
    out.insert_or_assign("matchConf", matchConf);

    target = out.to_string();
    replayFile.close();
    return true;
}
