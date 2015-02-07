UPTIME CHECKER
==============

The Uptime Checking program checks whether or not the list of servers (in uptime.config) are up or down. It then uptimes the filename (in uptime.config) associated with each server by adding the difference in time since you last ran uptime to the file. This is the code needed to install and run the uptime checker.

To install, first download the zip file and cd into it with ```wget http://bit.ly/16PVIDt; unzip uptime_checker.zip; cd uptime_checker```, then run ```make```.
