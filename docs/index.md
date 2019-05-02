# Minimaps Research 

I am [Alejandro Gamarra Niño](https://www.linkedin.com/in/alejandro-a-gamarra-ni%C3%B1o-568b6b171/), student of the [Bachelor’s Degree in Video Games by UPC at CITM](https://www.citm.upc.edu/ing/estudis/graus-videojocs/). This content is generated for the second year’s subject Project 2, under supervision of lecturer [Ricard Pillosu](https://es.linkedin.com/in/ricardpillosu).

## Theory about minimaps

### What is a minimap?

Minimap is an UI element of the HUD usually located in a corner of the screen. It shows an abstract, from the bird's eye perspective view of the playfield where the gameplay is being played on a smaller scale. Its main objective is to help the player orientate and help them successfully navigate the environment to reach points of interest and objectives.

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

### Minimap types 

Mini maps can be classified by many of their attributes as their **perspective** (Orthogonal, Isometric, 3D) or their **shape** (Rectangular, Circular) but the two most common types are:

- **Whole world**: 
This mini map type functions as a miniaturized world map. We can see them in Sports, Racing ,MOBA or Strategy games. In some cases they have a rectangle representing the area of the camera with which you can interact by dragging the mouse.

![enter image description here](https://raw.githubusercontent.com/alejandro61299/Minimaps_Personal_Research/master/docs/web_images/world.png)

- **Focused on the player**:
 The **scope** of the mini maps in Action, Shooter and Role-playing games displays only a limited field of view including your immediate surroundings (or your current sector).  

![enter image description here](https://raw.githubusercontent.com/alejandro61299/Minimaps_Personal_Research/master/docs/web_images/player.png)
 
 
### Features

- **Limits & barriers**
- **Player/s, enemies & allied position** 
- **Points of interest with icons**:  Checkpoints, domination points, buildings, save points, traders, etc;
- **Labels**: Can be attached to points of interest or only indicate the name of an area,
- **Peripheral arrows**: When a point of interest is not in the scope an arrow points in the direction where it is surrounding the edges of the minimap.
- **Rotation**:  When a minimap is focused on player it can rotate according to the angle of the axis that represents the height (y or z normally)
- **Automap**: Like the game The Binding of Isaac, the minimap is automatically generated next to the generation of rooms
 

### Interactions
-   **Navigation**: Move to a position by one click and/or drawing the path (Strategy & MOBA games).
-   **Alerts**:  (also called Pings) like Target or Retreat (especially in multiplayer games)
-   **Tooltips** Keeping the cursor on some element in the mini-map opens a panel with more information
-   **Zoom**  Usually this interaction is accompanied by two buttons plus and minus zoom or a slider
-   **Mouse drag**: Change the map  segment using mouse drag.
-   **Change rotation mode**: It allows to iterate between fixed map in the north or rotation

### Where should we place the minimap?

You may be thinking, does the position of the minimap matter? The truth is that yes, it affects above all in fast-paced games such as **First Person Shooters** where you usually put the minimap on the left. This is because our western culture tends to [pay more attention to the left part of the screen](http://www.useit.com/alertbox/horizontal-attention.html).

![enter image description here](https://raw.githubusercontent.com/alejandro61299/Minimaps_Personal_Research/master/docs/web_images/black.gif)

A misplaced minimap can not only lead to **prolonged reaction times** but also to **erroneous clicks** and **misperceptions** (depending on where you place other HUD elements). Therefore it is not uncommon to find that the same game released in the West has the minimap or other elements of HUD different position than in the East. Ideally, we would give the possibility to configure its position and size to make the game more accessible for everyone.

As we can verify each video game  genre has a distinctive position on the screen:

-   **MOBA games**  place the mini map exclusively at the  **bottom.**
-   **Strategy games** use mostly the **bottom left** but most **RTS** use  **top right.**  .
-   **MMORPG’s games**  place exclusively at the  **right** and mostly at the **top.**
-   **Racing games**  use predominantly the  **bottom**  like GPS's.
-   **FPS games**  use also mainly the  **left** for placing the mini map.

![enter image description here](https://raw.githubusercontent.com/alejandro61299/Minimaps_Personal_Research/master/docs/web_images/positions.png)


## Selected approach

After seeing the theory, let's start with the practical part. I'm going to teach you how to program an **isometric** mini-map from 0 using only SDL with a renderer initialized with [Opengles2](https://es.wikipedia.org/wiki/OpenGL_ES) . The final result will be a mini-map that can:

- Switch between **Focused on Player**  or  **Whole world**.
- Change its shape between **rectangular** or **circular**.
- Create **indicators** with icons such as points of interest, positons ,warnings by **clicking** over the minimap or world.
- Move the camera using the **mouse drag**.


![enter image description here](https://github.com/alejandro61299/Minimaps_Personal_Research/blob/master/docs/web_images/final%20result.gif?raw=true)

We have two classes that will contain everything we need:

  - The first is ``` class Minimap```  that will serve as the factory for the instances of ``` class Minimap_Indicator```
- The second one is  ``` class Minimap_Indicator``` that will serve as an infromation container for warnings and positions.

### Introduction

The first step is to generate the necessary information for the subsequent generation of the texture. In order not to distort the image, we will generate the texture from the width value. The information we have from the map will be obtained from a map daata already loaded in the Map module  (`class M_Map`  m_map on app).  This is the responsibility of the  `LoadMinimapInfo()` method.

![enter image description here](https://raw.githubusercontent.com/alejandro61299/Minimaps_Personal_Research/master/docs/web_images/info_1.png)

- We need to know `x_offset` since the 0.0 of the minimap texture does not match the 0.0 of the represented map.
- `apect_ratio_x` and` apect_ratio_y` are constants that directly relate the dimensions in pixels of the map with the size of our texture

```cpp
bool Minimap::LoadMinimapInfo()
{
	if (app->map->MapLoaded() == false) { return false; }  // If there isn't a map loaded break load

	// Information from map data ==================================================================

	float tile_width = app->map->data.tile_width;
	float tile_height = app->map->data.tile_height;
	float half_tile_width = (float)app->map->data.tile_width * 0.5f;
	float half_tile_height = (float)app->map->data.tile_height * 0.5f;

	// Number of tiles that fit on one side of the map =============================================

	float tiles_amount = (float)(app->map->data.columns + app->map->data.rows)* 0.5f;

	// Map Pixel Mesures ==========================================================================

	float map_width = tile_width * tiles_amount;
	float map_height = tile_height * tiles_amount;

	// We found texture height from the width with a rule of 3  ===================================

	texture_height = (texture_width * map_height) / map_width;

	// Now we have enough information to know the size of minimap tiles ===========================

	minimap_tile_width = texture_width / tiles_amount;
	minimap_tile_height = texture_height / tiles_amount;

	// We also find a constant to transform from pixels in the world to pixels in the minimap  ====

	aspect_ratio_x = texture_width / map_width;
	aspect_ratio_y = texture_height / map_height;

	// Finally, the blit x offset ===============================================================

	x_offset = (float)app->map->data.rows *minimap_tile_width * 0.5f;

	return true;
}
```

### Unit transformation methods
These functions are the core of our class. They allow us to transform units and thus be able to interact between the map and the minimap. 

 - **Map Coordinates to Minimap Pixels Coordinate**
```cpp
fPoint Minimap::MapToMinimap(const float x, const float y)
{
	return fPoint((x - y) * minimap_tile_width * 0.5f + x_offset, (x + y) * minimap_tile_height * 0.5f);
}
```
- **Minimap Pixel Coordinate to Map Coordinates**
```cpp
fPoint Minimap::MinimapToMap(const float x, const float y) 
{
	float half_width = minimap_tile_width * .5f;
	float half_height = minimap_tile_height * .5f;

	float x_mod = x - x_offset;

	return fPoint( (x_mod / half_width + y / half_height) * .5f, (y / half_height - x_mod / half_width) * .5f);
}
```
- **World Pixels Coordinates to Minimap Pixels Coordinate**
```cpp
fPoint Minimap::WorldToMinimap(const float x, const float y)
{
	return fPoint(x * aspect_ratio_x + x_offset, y * aspect_ratio_y);
}
```

- **Minimap Pixels Coordinate to World Pixels Coordinates**
```cpp
fPoint Minimap::MinimapToWorld(const float x, const float y)
{
	return fPoint((x - x_offset) / aspect_ratio_x,  y/ aspect_ratio_y);
}
```











## Links to more Documentation

- [Where Should We Place the Mini-Map? ( Gamasutra Blog )](https://www.gamasutra.com/blogs/JacekSliwinski/20130121/185119/Where_should_we_place_the_mini_map.php)
- [Video Game Mini-Maps Might Finally Be Going Away ( Kotaku Article )](https://kotaku.com/video-game-mini-maps-might-finally-be-going-away-1820011897)
- [Horizontal Atention Leans Left ( Nielsen Norman Group Article ) ](http://www.useit.com/alertbox/horizontal-attention.html) 
- [Mini-Map by Jack Davies ( Game UI Patterns Article )](https://gameuipatterns.com/gameui/mini-map/)
- [Following the Little Dotted Line ( Video )](https://www.youtube.com/watch?v=FzOCkXsyIqo)
- [Game Design Affect Minimap Design | Black Ops 4 Minimap ( Dexerto Article ) ](https://www.dexerto.com/call-of-duty/treyarch-dev-reveals-why-there-is-no-vsat-blackbird-in-black-ops-4-mutilplayer-184986)
<!--stackedit_data:
eyJoaXN0b3J5IjpbOTI3MTc5NzQxLDE3MjgyMzUwMzMsLTEwMj
UzNjk5OTQsLTE0MDk4NDIwNjYsLTE4MDUwMjkyMTksLTMyNjU5
NzEzNiwtNTY4OTk5MDg5LC0yMDY5ODExNjMwLDE0Mjc0MjUwOT
QsMTI1MDMzMDU2NywtMTI1Nzc3MjYyOSwtMTcyNzYwNjU2NSwt
MTA5NzQ1NjQ5OCwxMjg2MzcxNTQsODUzOTYxODA4LC0yMDMxMj
M0OTcyLDQwMTg4NTcwNCwxMTU5NDEwMjAwLDE0NTMwNjY0NjIs
MTI3MzExMTc1OF19
-->