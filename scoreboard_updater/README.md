scoreboard writer readme
===========================

This directory includes a  program creates and updates the cyber defense scenario's scoreboard and its supportive files.
The program assumes that there is a file called "up\_and\_down\_time.txt" which contains the blue team's up and down time througout the game, written in this format:

 up:(00min, 00sec) down:(00min, 00sec)

It also assumes that there is a file called "challenge\_list.txt" which contains a list of challenges, writen in the format below. This file is intended to 
be edited as needed by the instructor using the senario; a 1 signifies a completed challenge and 0 signifies an incomplete challenge.

sql\_version: 1
vandalism: 0
ssn\_recon: 0
persistent\_access: 0
wrench\_in\_the\_gears: 0
logout\_user: 0
ftp\_server: 0
stan: 0
plugin: 0
forgot\_my\_pass: 0
compress\_stan: 0
pro\_tools: 0
