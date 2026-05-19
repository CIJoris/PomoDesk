# PomoDesk - Physical Pomodoro Timer
**ESP32-based Pomodoro timer with API options**

V1.0 notes:
---

- the whole board is getting a bit warm
- on/off button is hard to reach, might as well remove
- data pin for LEDs needs to be higher voltage, use a levelshifter like: 74AHCT1G125, 74AHCT125, SN74AHCT1G125, SN74AHCT125
- power led connected to IO18 is not standard ON so the pin must be high/mid when the ESP is not programmed
- the corners don't have LEDs, which is not pretty
- startup surge causes USB hub to disconnect shortly -> Add LED power switching:
USB 5V → P-MOSFET/load switch → LED 5V
ESP GPIO → enable LED power after boot 



---

*Feel free to send me constructive criticism or things you would like me to implement, or even if you would like to help me develop this system!*
