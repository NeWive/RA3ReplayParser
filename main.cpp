#include "RA3Replay.h"

int main() {
    system("chcp 65001");
    std::string target;
    char file[] = "xxx";
    parseReplayFile(file, target);
    std::cout << target;
}