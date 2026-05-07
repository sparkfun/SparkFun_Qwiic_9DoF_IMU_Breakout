# Hardware Assembly

One of the many advantages of the Qwiic system is that hooking up your hardware is extremely simple. Simply grab a Qwiic cable and plug your 9DoF in! 

[![Connecting the 9DoF Breakout to a microcontroller via Qwiic cable](/img/15335_qwiic_9Dof_hookup.jpg)]

<div class="alert alert-danger" role="alert"><b>&#x26A1;WARNING!</b> <br />If you are using the ESP32 Thing Plus, make sure your power supply is <b>5V</b>, NOT 5.1V. We have noticed a power spike in our <a href="https://www.sparkfun.com/products/13831"> 5.1V power supplies</a>, that can damage the IC on the ESP32 Thing Plus. Long cables can also generate a large enough voltage spike to damage the IC. We recommend keeping power supply cables shorter than 6 feet to minimize potential damage.</div>

If you'd like to use the broken out GPIO pins, things get a bit more complicated. That said, to make life a little easier we've organized them by function, and provided lots of labels. You'll first notice that one side has the text 'I<sup>2</sup>C' and the other side says 'SPI.' The labels on either side are those that apply to that kind of communication. 

|     |     |
| --- | --- |
| [![](/img/15335-GPIO_I2C.jpg)](/img/15335-GPIO_I2C.jpg) | [![](/img/15335-GPIO_SPI.jpg)](/img/15335-GPIO_SPI.jpg) |
| _I2C Pin Labels_ | _SPI Pin Labels_ |


Next, you'll see that on the left side are the main connections to the host microcontroller. When connecting I<sup>2</sup>C you'll have a 'No Connect' pin that serves as the chip select when using SPI. As noted before, there are no modifications required to switch between I<sup>2</sup>C mode and SPI out of the box. However if the 'ADR' jumper is closed SPI will be unavailable.

On the right side are connections to external sensors that can be controlled by the ICM, as well as the 'INT' and 'FSYNC' interrupt pins. The auxiliary I<sup>2</sup>C bus pins are level shifted to/from the 'VIN' level that you supply. 

| Breakout Board Pin Functions (SPI) |     |     |     |
| --- | --- | --- | --- |
| Breakout Pin | Arduino Uno | Esp32 Thing Plus | Microcontroller Pin Requirements |
| MOSI | 11  | 18  | Data output of chosen SPI port |
| SCLK | 13  | 5   | Clock output of chosen SPI port |
| MISO | 12  | 19  | Data input of chosen SPI port |
| CS  | 2   | 2   | An output pin to select the ICM for SPI |

| Breakout Board Pin Functions (I2C) |     |     |     |
| --- | --- | --- | --- |
| Breakout Pin | Arduino Uno | Esp32 Thing Plus | Microcontroller Pin Requirements |
| DA  | SDA | 23  | Data line of chosen I2C port |
| CL  | SCL | 22  | Clock line of chosen I2C port |
| AD0 | \-  | \-  | Optional - use to control I2C address from software |

| Breakout Board Pin Functions (Auxiliary I2C and Interrupts) |     |     |     |
| --- | --- | --- | --- |
| Breakout Pin | Arduino Uno | Esp32 Thing Plus | Microcontroller Pin Requirements |
| ADA | \-  | \-  | Data line of auxiliary I2C bus |
| ACL | \-  | \-  | Clock line of auxiliary I2C bus |
| FSYNC | \-  | \-  | Optional - synchronize measurements with a signal out from the microcontroller |
| INT | \-  | \-  | Optional - respond to configurable interrupts in from the ICM |
