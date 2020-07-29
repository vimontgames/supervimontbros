# supervimontbros

A small 2D games I made with my kids during the lockdown in the spirit of old Amiga games I enjoyed when I started to learn programming. The goal was to show them the basics of game programming while making a fun tile-based 2D games that would implement their ideas.

![Title](img/title.png?raw=true "Title")


Following their game design, the game takes place in our actual neighborhood (excepted it was invaded by zombies).

![House](img/house.png?raw=true "House")


Up to 4 players can play at once using pads or even sharing keyboard. Please not that multiplayer and split-screen were implemented very early in the development in order as to avoid them fighting IRL when they want to go into different locations.

![3Players](img/3players.png?raw=true "3 Players")


It features a real-time tile and props editor so that they could easily edit the map.

![Tiles](img/tiles.png?raw=true "Tiles Editor")


There is also a "sprite grabber" used to defines stuff like bounding box, collisions or shadows for the sprites so that my oldest child (8 y.o.) was able to draw and integrate new objects or characters into the game without much help.

![Sprites](img/sprites.png?raw=true "Sprites Editor")


There are many ways to kill these zombies, including vicious traps ^^

![Trap](img/itsatrap.png?raw=true "Trap")


# how to run the game

Binaries are included for the Win32 version only. Run SuperVimontBros_x64_Final.exe. 
It should be easy to port the game onto other platforms than Win32 but my kids were not that interested in having a Linux version so ...


# controls

A/X: Run/turbo

B/O: Kick stuff (could be a ball, poo or zombie head)

X/▢ : Drop a poo (keep button pressed longer for a big one)

Y/△ : Enter/exit cars. Open stuff (ie. electric boxes)


# how to build

All you need is Visual Studio (I personally use the free VS2017 Community edition and ab $(SMFL) environment var pointing to your SMFL path (ie. "$(SFML)=G:\SFML\SFML-2.5.1").

# external stuff used

SFML distributed under the zlib/png license
https://www.sfml-dev.org/license.php

"Maffi - Turle Riddim" song distributed under Creative Commons BY-NC-ND 2.0 licence by JAHTARI
https://jahtari.org/archive/music/JTRNET18.htm
https://creativecommons.org/licenses/by-nc-nd/2.0/
 
