# Self-Parking Car

## Uart communication with romi test code 

| Pi         | Romi       |
| ---------- | ---------- |
| GPIO 14    | P3.3       |
| GPIO 15    | P3.2       |

Items done so far:
- auto connect eduroam (https://inrg.soe.ucsc.edu/howto-connect-raspberry-to-eduroam/)
- ubuntu auto login (https://askubuntu.com/questions/819117/how-can-i-get-autologin-at-startup-working-on-ubuntu-server-16-04-1)
- install ydlidar sdk
- install ydlidar ros
- to run the ydlidar ros driver, run
~~~~
    roslaunch ydlidar_ros_driver X2.launch
~~~~
- enable uart /dev/ttyAMA0 (https://askubuntu.com/questions/1254376/enable-uart-communication-on-pi4-ubuntu-20-04)
- communicate with romi (baud: 115200)
  - [code for buckler communicating with romi](https://github.com/icyphy/lf-buckler/blob/main/lib/romi.c)
  - [nrf52 code for uart](https://github.com/lab11/nrf52x-base/blob/6ca5df7892d2a26c864b52f1b5bf383e16885d25/sdk/nrf5_sdk_15.3.0/modules/nrfx/drivers/include/nrfx_uart.h)
- add the user (ubuntu) to gpio group (https://www.earnfs.com/en/html/1912.htm)
- to compile test file, run
~~~
    g++ -o romi_test romi_test.cpp romi.c -lwiringPi -lcrypt -lrt -lpthread
    ./romi_test
~~~

<br>

Next steps:
- ROS
  - https://roboticsbackend.com/use-and-compile-wiringpi-with-ros-on-raspberry-pi/
