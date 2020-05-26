These work on the Xinput library. If you want to learn how to install the board packages required, 
please watch my tutorial: https://youtu.be/61U-S7_XTDw or you can read the specifics here: https://github.com/dmadison/ArduinoXInput

This code is a modification of this example: https://github.com/dmadison/ArduinoXInput/tree/master/examples/GamepadPins

This controller is based around using two Arduino Pro Micros that communicate with eather other via I2C. The reason for this is pro micros
do not have enough pins to cover all of the buttons on an Xbox gamepad. If you use a Teensy board 3.1 it has enough pins to cover all the buttons.

This project is unique because it's an xbox controller split in two, making I2C a great option to limit the number of wires between controllers.

The I2C section of this code was written by: https://www.twitch.tv/gilbertsgadgets

If you make this controller I hope to see a photo of it!


