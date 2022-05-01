const parser = require("./build/Release/replayParser");
const dataWrapper = require("./module/wrapper");

module.exports = {
    /**
     *
     * @param absPath: 录像文件的绝对路径
     * @returns {any}
     */
    parseReplay: (absPath) => {
        return dataWrapper(parser.replayParser(absPath));
    }
}
