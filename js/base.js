log("started script!");

window.addEventListener("keypress", function (event) {
    log("got keypress " + event);
    curses.addstr(String.fromCharCode(event.toCode()));
});
