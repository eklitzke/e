log("started script!");

var logKeypress = function (code) {
    log("got keypress " + code);
};

var onKeyPress = function (k) {
    logKeypress(k);
};
