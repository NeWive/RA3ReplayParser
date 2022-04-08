//
// Created by dell on 2022/4/1.
//

#ifndef RAT_REPLAY_PARSER_RA3REPLAY_H
#define RAT_REPLAY_PARSER_RA3REPLAY_H

#include<cstdio>
#include<codecvt>
#include<locale>
#include<iostream>
#include<fstream>
#include<cstring>
#include<vector>
#include<ostream>
#include<jsoncons/json.hpp>
#include<sstream>

#define MAGIC_SIZE 17
#define U1_SIZE 31
#define U2_SIZE 20

typedef struct ReplayHeaderTemp {
    char headerFlag[MAGIC_SIZE + 1];
    unsigned char isSkirmish[1]; // 0x04 or 0x05
    unsigned char majorVersion[4];
    unsigned char minorVersion[4];
    unsigned char majorBuild[4];
    unsigned char minorBuild[4];
    unsigned char hasCommentary[1]; // 0x06 or 0x1E
    unsigned char separator[1]; // 目前有0x00 0x20
} ReplayHeaderTemp;

typedef struct CodePoint {
    char c[4];
    unsigned char u[4];
    unsigned int i;
} CodePoint;

typedef struct TBString {
    unsigned char byte1;
    unsigned char byte2;
} TBString;

typedef struct dateText
{
    uint16_t data[8];
} dateText;



unsigned int readUInt32LittleEndian(const unsigned char in[]);

unsigned int readUInt16LittleEndian(const unsigned char in[]);

void unicodePointToUTF8(unsigned int, CodePoint *);

std::string read2ByteString(std::istream &in, unsigned int separator);

std::string read2ByteStringN(std::istream &in, unsigned int separator, size_t n);

std::vector<std::string> tokenize(const std::string &, const std::string &separator);

std::string getFaction(unsigned int f);

bool parseReplayFile(char f[], std::string &);

#endif //RAT_REPLAY_PARSER_RA3REPLAY_H
