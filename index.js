const parser = require("./build/Release/replayParser");

module.exports = {
    /**
     *
     * @param absPath: 录像文件的绝对路径
     * @returns {any}
     */
    parseReplay: (absPath) => {
        return JSON.parse(parser.replayParser(absPath))
    }
}