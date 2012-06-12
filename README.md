kinectArmTracker
================

Copyright Design I/O LLC 2011

http://design-io.com - hello [-at–] design-io.com - @design_io 


------ USAGE 

Feel free to use this tool for your projects or prototypes.
If you are interested in using it in a commercial context please contact us. 
Please do not use it to copy any existing projects from other artists/designers/companies ( including ourselves ). 

Have fun with it! 


------ OSC FORMAT

The format for the osc data is the following.
osc address: "armTracker" 
arg 0: shoulder angle - float - degrees
arg 1: elbow angle - float - degrees
arg 2: wrist angle - float - degrees
arg 3: thumb angle - float - degrees


------ OSC IP AND PORT

By default the app binds to port 9555 of the broadcast address. So packets are sent to all computers on the network. 
If you don't have a router it should default to 127.0.0.1 
If you want to override the ip address and set a specific one edit the file "data/oscIP.xml"
NOTE: the receive test app is hardcoded to port 9555. 


------ MISC INFO

Made with openFrameworks 0071 - download openFrameworks 0071 from http://openFrameworks.cc and clone this repo into your apps/ folder
