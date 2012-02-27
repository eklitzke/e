log("started script!");

window.addEventListener("keypress", function (event) {
    log("got keypress, code is " + event.getCode() + ", name is \"" + event.getName() + "\"");
    var name = event.getName();
    if (event.isASCII()) {
        if (name == "q")
            window.stopLoop();
        else
            curses.addstr(name);
    } else {
        if (event.getName() !== "key_backspace") {
            // NOTHING
        }
    }
});
