# bomberman-type game on an 8x8 LED matrix
In this homework i needed to create a game with a 16x16 map on an 8x8 led Matrix and a game menu on an LCD display. I chose to implement a bomberman-type game in which the player needs to find a goal-point in the map, exploding any obstacles needed to get there. There will also be an LED indicator that glows brighter the closer the player gets to the point

## Requirements:
 - Arduino Uno
 - 8x8 LED matrix (1088AS)
 - MAX7219 driver for the matrix
 - 16x2 LCD Display
 - double axis joystick controller
 - electrolytic capacitor 10uF
 - ceramic capacitor 104pF
 - 10k ohm resistor
 - 220 ohm resistor
 - passive buzzer

## Circuit:
![matrix-1](https://github.com/blwUsama/8x8MatrixGame/assets/95240433/4755f6ca-6930-454a-9f50-19ff0f861e9c)
![matrix-2](https://github.com/blwUsama/8x8MatrixGame/assets/95240433/742d440a-f5aa-4582-b92a-5df3e50fc09f)

Demo can be viewed at: https://youtu.be/ks3EWWa9jfk

 ## implementation details
### Game:
- Because the map is 16x16 and the led matrix is 8x8, only part of a map can be visible at any given time, this is implemented by defining a "field of view" square of 8x8, all we need to define this field of view square is the X and Y coordinate of the upper left corner of the square, if we know that point we know all we need about the square
- Considering we're dealing with a matrix and we have the player and the goal's X,Y coordinates, it's very easy to calculate the distance between the two, i chose to calculate the euclidian distance (in a straight line) over other distance types for no particular reason. the maximum possible distance would be if the player and the goal point are diagonally opposed, and considering the map is 16x16 this means the maximum distance is 16 * rad(2) = 22, we'll be using this value to map the distance (0-22) to analogWrite's limits (0-255)


