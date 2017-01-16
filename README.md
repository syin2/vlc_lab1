# vlc_lab1
This lab is used to demonstrate the basic modulation and demodulation of data transmitted by visible light.  
# Prerequisite
* Hardware:
    * One BeagleBone Black Rev.C
    * One [OpenVLC BBB Cape](http://nsl.cs.uh.edu/~syin/openvlc/vlc_v2.JPG)
* Software:
    * Linux image: [debian-7.4-machinekit-armhf-2014-05-19-4gb.img.xz](https://drive.google.com/file/d/0BwGT2J3dvAfNOEVibS1KQ2d5MGc/view)
    * [Configuration](http://www.openvlc.org/openvlc.html)
# Compile  
`make`  
This will generate the `led_send.ko` kernel module.  
`sudo insmod led_send.ko`  
Load this kernel module.  


