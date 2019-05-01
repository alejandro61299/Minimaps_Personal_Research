# Minimaps Research 

## Theory about minimaps

### What is a minimap?

Minimap is an UI element of the HUD usually located in a corner of the screen. It shows a abstracted view from the bird’s eye perspective of the playfield where the gameplay is being developed on a smaller scale. Its main objective is to help orientate the player and help them successfully navigate the environment to points of interest and objectives.

![enter image description here](https://raw.githubusercontent.com/alejandro61299/Minimaps_Personal_Research/master/docs/web_images/lol.png)

### When is a minimap necessary?

This question has a different answer depending on the genre and the pillars of the video game but in general the main problems that make the minimap a necessity are:

-  The playfield is large
-  There are multiple ways to reach the same destination (No linear game)
-  There are lots of different points of interest
-  You need to know the position of allied and enemy units
-  The game requires [macro management](https://gamemakers.com/micro-vs-macro-consolepc-vs-mobile-gaming/) ( RTS )

![enter image description here](https://raw.githubusercontent.com/alejandro61299/Minimaps_Personal_Research/master/docs/web_images/massive_enemies.png)

In order to solve these problems **one of the several options** is to implement a minimap. Other options are in-game elements like arrows or the famous compas buts this elements are not compatible with most genres.

### Minimap types , interactions & features 

- **Whole world**
This mini map type functions as a miniaturized world map. We can see them in Sports, Racing ,MOBA or Strategy games. In some cases they have a rectangle representing the area of the camera with which you can interact by dragging the mouse.

![enter image description here](https://raw.githubusercontent.com/alejandro61299/Minimaps_Personal_Research/master/docs/web_images/world.png)

- **Focused on the player**
 The **scope** of the mini maps in Action, Shooter and Role-playing games displays only a limited field of view including your immediate surroundings (or your current sector).  

![enter image description here](https://raw.githubusercontent.com/alejandro61299/Minimaps_Personal_Research/master/docs/web_images/player.png)

### Where should we place the minimap?

You will be asking, does the position of the minimap matter? The truth is that yes, it affects above all the fast-paced games such as **First Person Shooters** where you usually put the minimap on the left. This is because our western culture tends to [pay more attention to the left part of the screen](http://www.useit.com/alertbox/horizontal-attention.html)

![enter image description here](https://raw.githubusercontent.com/alejandro61299/Minimaps_Personal_Research/master/docs/web_images/black.gif)

A misplaced minimap can not only lead to **prolonged reaction times** but also to **erroneous clicks** and **misperceptions** (depending on where you place other HUD elements). Therefore it is not uncommon to find that the same game released in the West has the minimap or other elements of HUD different position than in the East. Ideally, give the possibility to configure its position and size, so we give access to the game.

As we can verify each video game  genre has a distinctive position on the screen:

-   **MOBA games**  place the mini map exclusively at the  **bottom.**
-   **Strategy games** use mostly the **bottom left** but most **RTS** use  **top right.**  .
-   **MMORPG’s games**  place exclusively at the  **right** and mostly at the **top.**
-   **Racing games**  use predominantly the  **bottom**  like GPS's.
-   **FPS games**  use also mainly the  **left** for placing the mini map.

![enter image description here](https://raw.githubusercontent.com/alejandro61299/Minimaps_Personal_Research/master/docs/web_images/positions.png)


## Selected approach

## Webgraphy

- [Where Should We Place the Mini-Map? ( Gamasutra Blog )](https://www.gamasutra.com/blogs/JacekSliwinski/20130121/185119/Where_should_we_place_the_mini_map.php)
- [Video Game Mini-Maps Might Finally Be Going Away ( Kotaku Article )](https://kotaku.com/video-game-mini-maps-might-finally-be-going-away-1820011897)
- [Horizontal Atention Leans Left ( Nielsen Norman Group Article ) ](http://www.useit.com/alertbox/horizontal-attention.html) 
- [Mini-Map by Jack Davies ( Game UI Patterns Article )](https://gameuipatterns.com/gameui/mini-map/)
- [Following the Little Dotted Line ( Video )](https://www.youtube.com/watch?v=FzOCkXsyIqo)
- [Game Design Affect Minimap Design | Black Ops 4 Minimap ( Dexerto Article ) ](https://www.dexerto.com/call-of-duty/treyarch-dev-reveals-why-there-is-no-vsat-blackbird-in-black-ops-4-mutilplayer-184986)
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTUzOTczMDk1MCwtNjkzNDU2ODkyLDEyNz
k4NDE3NzAsLTQ5NzUyNzI1MywtMTc5OTQ4MzAxOSwtMTkzOTcx
MDE2MywtMTc1NjA1ODQ1NywzOTUxODAyMTQsLTE3NTYwNTg0NT
csLTEzMjA4NjcxMTUsMjA3MjQ4NjAwNywxODgzOTEzNzY4LC0x
NTA2MDQzNTY4LDE1ODk0NjIxMjEsLTE3MzA2NDA2NzUsNDExMT
QyNDcwLDM4NDY2NjQzMSwtODUyNzUwNTksLTEzMTYxOTMwNzAs
MjE0MjkwODc2XX0=
-->