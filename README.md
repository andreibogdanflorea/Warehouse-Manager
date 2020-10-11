<div align="left">
  <h1>Warehouse-Manager</h1>
  <h3>A program used to manage the loading and distributing of packages from a warehouse using robots/trucks</h3>
</div>
<br/>
<br/>

## Problem details ##
* app.c: the app used for general management
* WearhouseManager.h: contains the definitions of the data structures
* WearhouseManager.c: contains the implementation of the functions that work
with the data structures in WearhouseManager.h
* wearhouse and wearhouse_small: contains information about the packages in the warehouse
  * first line contains the number of packages
  * each of the next lines contains information about a certain package: its priority and destination
* parkinglot and parkinglot_small: information about trucks and robots
  * the lines that start with "T" represent a truck and contain information about: 
  the destination, the capacity, transit time, time of departure, state of the truck (0 for departed and 1 for stand-by)
  * the lines that start with "R" represent a robot and also contain the capacity of the robot
  
The general view of the system in action:

![This was supposed to be an image](https://raw.githubusercontent.com/andreibogdanflorea/Warehouse-Manager/main/image.jpg)

## How to use ##
Firstly, create the wearhouse and parkinglot files described previously and run the app in the terminal with:

`./WearhouseManager <warehouse file> <parkinglot file>`

The output represents the way the warehouse operates in a seven day working week, hour by hour: the packages in the warehouse,
the robots and trucks and the packages they carry.
