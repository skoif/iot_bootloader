This is source of bootloader, which can be used on IoT devices, based on ESP8266. Main feature of this code is automatical wifi setup.
If device started for the first time or wifi is unreachable, bootloader will create it's own wi-fi mesh.
If connection is successfull, it will not do anything.

USAGE:
In your void setup(){} and void loop(){} you can find this construction:
  if(bl_afterstart_state){
    /* Fired if normal start */
  }else{
    /* Fired if AP mode */
  }
I think, that now you understand, what you can do with this code.
Thanks for usage.

By Alexander Zemskov.
WEB: sazima.ru

License: GPL
