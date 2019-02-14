# STM32F407G-DISC1 Demo

Sample code for interaction with [STM32F407G-DISC1](http://www.st.com/en/evaluation-tools/stm32f4discovery.html) development board:

1) Toggling on/off, fading in/out LEDs
2) Interaction with on-board buttons by toggling LEDs on/off
3) Using multiple on-board timers
4) Processing the accelerometer's data and logging it into a Debug log
5) Displaying the ambient temperature (in Debug log and with LEDs) with on-board temperature sensor
6) TDD test of the LED's API with [Visual Studio](http://visualstudio.microsoft.com) (see: /samples/ESS Source Code (Dmitry Kosjuchenko)/LedDriver_TDD_MSVC).

The project is configured for use with [Keil MDK](http://www2.keil.com/mdk5/).
