#include "src/replay.h"

int main() {
    system("chcp 65001");
    std::string target;
    char file[] = "E:\\MyProject\\RA3ReplayParser\\1.RA3Replay";

    std::cout << ReplayParser::parseReplayFile(file);
//    std::cout << target;
//    std::ofstream outfile;
//    outfile << target;
}