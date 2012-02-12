log("started script!");

log("window is " + window);
log("window.test is " + window.test);
log("calling window.test()");
window.test();
log("done calling window.test()");

var logKeypress = function (code) {
    log("got keypress " + code);
};

var onKeyPress = function (k) {
    logKeypress(k);
};
