//
// Created by dell on 2022/4/15.
//

#define MAGIC_SIZE 17
#include<codecvt>
#include<locale>
#include<iostream>
#include<fstream>
#include<cstring>
#include<vector>
#include<ostream>
#include<jsoncons/json.hpp>
#include<node/node.h>
#include<unistd.h>

namespace ReplayParser {

    using v8::FunctionCallbackInfo;
    using v8::Isolate;
    using v8::Local;
    using v8::Object;
    using v8::String;
    using v8::Value;
    using v8::Exception;

    typedef struct ReplayHeader {
        unsigned char isSkirmish[1]; // 0x04 or 0x05
        unsigned char majorVersion[4];
        unsigned char minorVersion[4];
        unsigned char majorBuild[4];
        unsigned char minorBuild[4];
        unsigned char hasCommentary[1]; // 0x06 or 0x1E
        unsigned char separator[1]; // 目前有0x00 0x20
    } ReplayHeader;

    typedef struct CodePoint {
        char c[4];
        unsigned char u[4];
        unsigned int i;
    } CodePoint;

    typedef struct TBString {
        unsigned char byte1;
        unsigned char byte2;
    } TBString;

    typedef struct dateText {
        uint16_t data[8];
    } dateText;

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

    std::string read2ByteStringN(std::istream &in, unsigned int separator, size_t len) {
        CodePoint ccp;
        TBString cbuf;
        std::string s;
        size_t n = len;
        while (n--) {
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

    std::vector<std::string> tokenize(const std::string &str, const std::string &delimiters) {
        std::vector<std::string> tokens;

        std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
        std::string::size_type pos = str.find_first_of(delimiters, lastPos);

        while (std::string::npos != pos || std::string::npos != lastPos) {
            tokens.push_back(str.substr(lastPos, pos - lastPos));
            lastPos = str.find_first_not_of(delimiters, pos);
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
            default: return "未知";
        }
    }

    std::string genFailedMessage(std::string message) {
        jsoncons::json out;
        out.insert_or_assign("message", message);
        out.insert_or_assign("status", false);
        return out.to_string();
    }

    std::string parseReplayFile(char *f) {
        // replay头信息
        ReplayHeader* header = (ReplayParser::ReplayHeader *) malloc(sizeof (ReplayParser::ReplayHeader));
        std::string replayTitle, matchDescription, mapName, mapId, replayName, str_anotherName, gameVer;
        size_t size;
        uint32_t timestamp;
        // 标志字符串, Mod信息
        char cncMagic[8], modInfo[22], headerFlag[MAGIC_SIZE + 1];
        // 简略信息
        std::vector<int> playerIds1;
        std::vector<std::string> playerNames1;
        std::vector<int> playerTeamIds1;
        // 详细信息
        std::vector<int> playerIds2;
        std::vector<std::vector<std::string>> playerNames2;
        std::vector<int> playerTeamIds2;
        unsigned int headerLength;
        std::string mapRealName, mapCRC, matchSeed, postCommentator, matchConf, playerInfoQuery;
        uint32_t firstChunkLoc;
        // 无用的字节
        uint32_t meaningless;
        unsigned char u31[31];
        char timeout[200];
        char playerNumber;
        std::string playerName;
        char teamId;
        uint32_t playerId;
        // Header尾部信息
        char u1[9];
        uint32_t strFileNameLen;
        std::string strFileName;
        dateText dateTime;
        uint32_t versionMagicLen;
        uint32_t u20[20];
        uint32_t after_vermagic;
        char tempByte;
        // 输出
        jsoncons::json out;

        std::fstream replayFile(f, std::ios::in | std::ios::binary);
        // 读取头部Flag
        replayFile.read(headerFlag, MAGIC_SIZE);
        headerFlag[MAGIC_SIZE] = '\0';
        if (strncmp(headerFlag, "RA3 REPLAY HEADER", 17)) {
            return ReplayParser::genFailedMessage("invalid replay flag, it should be: RA3 REPLAY HEADER");
        }
        // 基本信息
        replayFile.read(reinterpret_cast<char *>(header), sizeof (ReplayParser::ReplayHeader));
        replayTitle = read2ByteString(replayFile, header->separator[0]);
        matchDescription = read2ByteString(replayFile, header->separator[0]);
        mapName = read2ByteString(replayFile, header->separator[0]);
        mapId = read2ByteString(replayFile, header->separator[0]);
        gameVer = std::to_string(readUInt32LittleEndian(header->majorVersion)) + "." + std::to_string(
                readUInt32LittleEndian(header->minorVersion));
        out.insert_or_assign("matchDescription", matchDescription);
        out.insert_or_assign("replayTitle", replayTitle);
        out.insert_or_assign("mapName", mapName);
        out.insert_or_assign("mapID", mapId);
        out.insert_or_assign("gameVersion", gameVer);
        // 玩家数量
        replayFile.read(&playerNumber, 1);
        out.insert_or_assign("playerNumber", int(playerNumber));
        // 解析玩家信息
        jsoncons::json playerInfo(jsoncons::json_array_arg);
        for (int i = 0; i <= int(playerNumber); ++i) {
            jsoncons::json player(jsoncons::json_object_arg);
            replayFile.read(reinterpret_cast<char *>(&playerId), 4);
            playerName = read2ByteString(replayFile, header->separator[0]);
            playerIds1.push_back(playerId);
            playerNames1.push_back(playerName);
            player.insert_or_assign("id", playerId);
            player.insert_or_assign("name", playerName);
            playerNames1.push_back(playerName);
            if (header->isSkirmish[0] == 5) {
                replayFile.read(reinterpret_cast<char *>(&teamId), 1);
                playerTeamIds1.push_back(int(teamId));
                player.insert_or_assign("teamID", int(teamId));
            } else {
                playerTeamIds1.push_back(-1);
                player.insert_or_assign("teamID", -1);
            }
            playerInfo.push_back(player);
        }
        out.insert_or_assign("playerInfo", playerInfo);
        // 从CNC3RPL到第一个Chunk的偏移量,用于定位第一个Chunk
        replayFile.read(reinterpret_cast<char*>(&meaningless), 4); // 用于计算从CNC3RPL 到第一个Chunk的偏移量
        firstChunkLoc = (unsigned int)replayFile.tellg() + 4 + meaningless;
        // 无用字段
        replayFile.read(reinterpret_cast<char*>(&meaningless), 4);
        // CNC Replay标志字段
        replayFile.read(cncMagic, 8);
        if (strncmp(cncMagic, "CNC3RPL\0", 8)) {
            return genFailedMessage("invalid CNC Replay flag, it should be CNC3RPL");
        }
        // Mod 信息
        replayFile.read(modInfo, 22);
        char *p(modInfo), *q(NULL);
        std::vector<std::string> modInfoV;
        while (p < modInfo + 22)
        {
            q = std::strchr(p, '\0');
            if (q == NULL) break;
            if (p[0] != '\0') {
                modInfoV.emplace_back(p);
            }
            p = q+1;
        }
        out.insert_or_assign("modInfo", modInfoV);
        // 时间戳
        replayFile.read(reinterpret_cast<char*>(&timestamp), 4);
        out.insert_or_assign("timestamp", timestamp);
        //无用字节
        replayFile.read(reinterpret_cast<char*>(&u31), 31);
        // Header长度
        replayFile.read(reinterpret_cast<char*>(&headerLength), 4);
        if (headerLength > 10000) {
            return genFailedMessage("Requested header length too big.");
        }
        // 处理头部具体信息
        std::vector<char> headerData(headerLength);
        replayFile.read(headerData.data(), headerLength);
        // 解析详细信息
        std::string tempS = std::string(headerData.begin(), headerData.end());
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
                    std::string ipAdd = std::to_string(v>>24) + "." + std::to_string(((v<<8)>>24)) + "." + std::to_string(((v<<16)>>24)) + "." + std::to_string(((v<<24)>>24)) + ":" + sub2Tokens[2];
                    playerInfoTemp.insert_or_assign("ip", ipAdd);
                    if (subToken.size() > 2 && subToken[0] == 'C' && subToken[2] == ',') {
                        playerInfoTemp.insert_or_assign("isPlayer", false);
                        playerInfoTemp.insert_or_assign("opponent", sub2Tokens[0]);
                        playerInfoTemp.insert_or_assign("faction", std::atoi(sub2Tokens[2].c_str()));
                        jsoncons::json otherData(jsoncons::json_array_arg);
                        for (size_t j = 1; j < sub2Tokens.size() - 1; ++j) {
                            otherData.push_back(sub2Tokens[j]);
                        }
                        playerInfoTemp.insert_or_assign("otherData", otherData);
                    } else {
                        playerInfoTemp.insert_or_assign("isPlayer", true);
                        playerInfoTemp.insert_or_assign("playerName", sub2Tokens[0]);
                        playerInfoTemp.insert_or_assign("faction", std::atoi(sub2Tokens[5].c_str()));
                        jsoncons::json otherData(jsoncons::json_array_arg);
                        for (size_t j = 3; j < sub2Tokens.size() - 1; ++j) {
                            otherData.push_back(sub2Tokens[j]);
                        }
                        playerInfoTemp.insert_or_assign("otherData", otherData);
                    }
                    playerInfoArr.push_back(playerInfoTemp);
                }
                out.insert_or_assign("playerDetailedInfo", playerInfoArr);
            } else if(token[0] == 'M' && token[1] == '=') {
                mapRealName = token;
                out.insert_or_assign("mapRealName", mapRealName);
            } else if(token[0] == 'M' && token[1] == 'C') {
                mapCRC = token;
                out.insert_or_assign("mapCRC", mapCRC);
            } else if(token[0] == 'S' && token[1] == 'D') {
                matchSeed = token;
                out.insert_or_assign("matchSeed", matchSeed);
            } else if(token[0] == 'R' && token[1] == 'U') {
                matchConf = token;
                out.insert_or_assign("matchConf", matchConf);
            } else {}
        }
        // 跳过无用字节
        replayFile.read(u1, 9);
        // replay名
        replayFile.read(reinterpret_cast<char *>(&strFileNameLen), 4);
        strFileName = read2ByteStringN(replayFile, header->separator[0], strFileNameLen);
        out.insert_or_assign("replayName", strFileName);
        // replay 时间
        replayFile.read(reinterpret_cast<char *>(&dateTime), sizeof(dateTime));
        // 版本号
        replayFile.read(reinterpret_cast<char *>(&versionMagicLen), 4);
        std::vector<char> versionMagic(versionMagicLen);
        replayFile.read(versionMagic.data(), versionMagicLen);
        std::string str_vermagic = std::string(versionMagic.begin(), versionMagic.end());
        replayFile.read(reinterpret_cast<char*>(&after_vermagic), 4);

        // 无用字节
        replayFile.read(&tempByte, 1);
        replayFile.read(reinterpret_cast<char*>(&u20), 20*4);
        replayFile.close();
        out.insert_or_assign("status", true);
        return out.to_string();
    }

    bool checkFileExists(char *fName) {
        return (access(fName, F_OK ) != -1 );
    }

    void parse(const FunctionCallbackInfo<Value> &args) {
        Isolate *isolate = args.GetIsolate();
        if (args.Length() < 1) {
            isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "wrong number of args").ToLocalChecked()));
            return ;
        }
        if (!args[0]->IsString()) {
            isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "invalid arg type which should be String").ToLocalChecked()));
            return ;
        }
        String::Utf8Value fName(isolate, args[0]);
        if (checkFileExists(*fName)) {
            std::string out = parseReplayFile(*fName);
            args.GetReturnValue().Set(String::NewFromUtf8(isolate, out.c_str()).ToLocalChecked());
        } else {
            isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Target replay file does not exist").ToLocalChecked()));
            return ;
        }
    }

    void Initialize(Local<Object> exports) {
        NODE_SET_METHOD(exports, "replayParser", parse);
    }

    NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)
}

