[![Hookup Guide Banner Image](/img/9DoF-IMU-HG-Banner.png)]

The [SparkFun 9DoF IMU Breakout](https://www.sparkfun.com/products/15335) incorporates all the amazing features of Invensense's ICM-20948 into a Qwiic-enabled breakout board replete with logic shifting and broken out GPIO pins for all your motion sensing needs. The ICM-20948 itself is an extremely low powered, I<sup>2</sup>C and SPI enabled 9-axis motion tracking device that is ideally suited for smartphones, tablets, wearable sensors, and IoT applications. Featuring a 3-Axis Gyroscope with four selectable ranges, a 3-Axis Accelerometer, again with four selectable ranges, a 3-axis compass with a wide range to  ±4900 µT, and an on-board Digital Motion Processor, this little breakout can even detect the motion of invisibility cloaks. Not really. Just checking to see if you were still with me. But it _is_ pretty amazing.

In this hookup guide, we'll connect our sensor up to our [Esp32 Thing Plus](https://www.sparkfun.com/products/14689) microcontroller and run a of quick (Qwiic) example to get you up and running with this fantastic board! 


### Required Materials

To follow along with this tutorial, you will need the 9DoF IMU Breakout with the following materials:

* [SparkFun Thing Plus - ESP32 WROOM](https://www.sparkfun.com/sparkfun-thing-plus-esp32-wroom-micro-b.html)
* [Qwiic Cable](https://www.sparkfun.com/flexible-qwiic-cable-100mm.html)
* [USB Micro-B Cable](https://www.sparkfun.com/reversible-usb-a-to-reversible-micro-b-cable-0-3m.html)

### Tools

If you choose to utilize the broken out GPIO, you will need a soldering iron, solder, and [general soldering accessories](https://www.sparkfun.com/categories/49).

* [Soldering Iron - 60W (Adjustable Temperature)](https://www.sparkfun.com/soldering-iron-60w-adjustable-temperature.html)
* [Solder Lead Free - 15-gram Tube](https://www.sparkfun.com/solder-lead-free-15-gram-tube.html)

### Suggested Reading

If you aren't familiar with the Qwiic system, we recommend reading [here for an overview](https://www.sparkfun.com/qwiic).


We would also recommend taking a look at the following tutorials if you aren't familiar with them:

* [How to Solder: Through-Hole Soldering](https://learn.sparkfun.com/tutorials/how-to-solder-through-hole-soldering)
* [Accelerometer Basics](https://learn.sparkfun.com/tutorials/accelerometer-basics)
* [I2C](https://learn.sparkfun.com/tutorials/i2c)
* [Serial Terminal Basics](https://learn.sparkfun.com/tutorials/terminal-basics)