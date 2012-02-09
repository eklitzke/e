log("started script!");

var logKeypress = function (code) {
    log("JAVASCRIPT says: \"got keypress " + code + "\"");
};

var onKeyPress = function (k) {
    logKeypress(k);
};
