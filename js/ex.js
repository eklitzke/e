core.addKeypressListener("ex", function (event) {
  if (event.getCode() == 13) {
    switch (core.exBuffer) {
    case "wq":
    case "wqa":
    case "wqa!":
    case "q":
    case "qa":
    case "q!":
    case "qa!":
      world.stopLoop();
      break;
    }
    core.exBuffer = "";
    core.switchMode('command');
  } else {
    core.exBuffer += event.getChar();
  }
  return;
});
