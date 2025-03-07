var image, ws;
var pressedKeys = new Set();

const COMMAND_CODES = {
   moveForward: 60,
   rotateRight: 61,
   moveBack: 62,
   rotateLeft: 63,
   stopMovement: 64,
   changeResolutionToLow: 22,
   changeResolutionToMedium: 26,
   changeResolutionToHigh: 27,
};

function setImage(blob) {
   let url = URL.createObjectURL(blob);
   image.src = url;
}

function sendCommand(command) {
   let buffer = new Uint8Array(1);
   buffer[0] = command;
   ws.send(buffer);
}

window.onload = () => {
   image = document.querySelector("#current_frame");
   ws = new WebSocket(`ws://${window.location.hostname}/ws`);

   ws.onopen = () => {
      console.log("Connected!");
   };
   ws.onmessage = (event) => {
      let blob = event.data;
      setImage(blob);
   };
};

var isUpPressed = () => (pressedKeys.has("KeyW") || pressedKeys.has("ArrowUp"));
var isLeftPressed = () => (pressedKeys.has("KeyA") || pressedKeys.has("ArrowLeft"));
var isDownPressed = () => (pressedKeys.has("KeyS") || pressedKeys.has("ArrowDown"));
var isRightPressed = () => (pressedKeys.has("KeyD") || pressedKeys.has("ArrowRight"));

function sendMoveCommand() {
   if (isUpPressed()) {
      sendCommand(COMMAND_CODES.moveForward);
   } else if (isLeftPressed()) {
      sendCommand(COMMAND_CODES.rotateLeft);
   } else if (isDownPressed()) {
      sendCommand(COMMAND_CODES.moveBack);
   } else if (isRightPressed()) {
      sendCommand(COMMAND_CODES.rotateRight);
   }
}

setInterval(sendMoveCommand, 750);

window.onkeydown = (event) => {
//   console.log("KEYDOWN", event);
   pressedKeys.add(event.code);

   if (event.code == "Digit1") {
      sendCommand(COMMAND_CODES.changeResolutionToLow);
   } else if (event.code == "Digit2") {
      sendCommand(COMMAND_CODES.changeResolutionToMedium);
   } else if (event.code == "Digit3") {
      sendCommand(COMMAND_CODES.changeResolutionToHigh);      
   }

   sendMoveCommand();
};

window.onkeyup = (event) => {
//   console.log("KEYUP", event);
   pressedKeys.delete(event.code);

   if (!isUpPressed() && !isLeftPressed() && !isDownPressed() && !isRightPressed()) {
      sendCommand(COMMAND_CODES.stopMovement);
      console.log("STOP!!");
   }
};

