function splitter(s) {
    return s.split("=")[1];
}

function strToNumber(s) {
    return Number(s);
}

function strToBool(s) {
    return Boolean(Number(s));
}

const transValueType = {
    "number": strToNumber,
    "bool": strToBool
};

const transFaction = {
    1: "observer",
    3: "commentator",
    4: "ally",
    8: "soviet",
    2: "empire",
    7: "random",
    9: "神州"
}

function handleMatchConf(s) {
    const conf = splitter(s).split(" ").filter(i => {
        if (i) {
            return true;
        }
    });
    const mapToObj = [
        {
            name: "initialCameraPlayer",
            type: "number"
        },
        {
            name: "gameSpeed",
            type: "number"
        },
        {
            name: "initialResources",
            type: "number"
        },
        {
            name: "broadcastGame",
            type: "bool"
        },
        {
            name: "allowCommentary",
            type: "bool"
        },
        {
            name: "tapeDelay",
            type: "number"
        },
        {
            name: "randomCrates",
            type: "bool"
        },
        {
            name: "enableVoIP",
            type: "bool"
        }
    ];
    let res = {};
    mapToObj.forEach((e, i) => {
        let v = transValueType.hasOwnProperty(e.type) ? transValueType[e.type](conf[i]) : conf[i];
        res[e.name] = v;
    });
    return res;
}

function handlePlayerDetailedInfo(arr) {
    return arr.map((player, i) => {
         return {
             playerName: player.playerName,
             isPlayer: player.isPlayer,
             faction: {
                 id: player.faction,
                 name: transFaction.hasOwnProperty(player.faction) ? transFaction.hasOwnProperty(player.faction) : "unknown"
             },
             ip: player.ip
         }
    });
}

function replayDataWrapper(dataStr) {
    const data = JSON.parse(dataStr);
    data.mapCRC = `0x${splitter(data.mapCRC)}`;
    const temp = data.mapRealName.split("/");
    data.mapRealName = temp[temp.length - 1];
    data.matchConf = handleMatchConf(data.matchConf);
    data.matchSeed = strToNumber(splitter(data.matchSeed));
    data.playerDetailedInfo = handlePlayerDetailedInfo(data.playerDetailedInfo)
    return data;
}

module.exports = replayDataWrapper;
