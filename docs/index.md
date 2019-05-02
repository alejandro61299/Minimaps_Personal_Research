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

Después de ver la teoría vamos a comenzar con la parte práctica. Os voy a enseñar a como programar un minimapa desde 0 usando únicamente SDL con un renderer inicializado con [Opengles2](https://es.wikipedia.org/wiki/OpenGL_ES).
El resultado final será un minimapa que podrá:

- Cambair entre **Focused on Player** o **Whole world**
- Cambiar su shape entre rectangular o circular
- Crear indicadores con iconos como **points of interest**, **positons**, **alerts** clickando en el minimapa o en el mismo mapa. 
- Mover la cámara usando el **mouse drag**

Aquí podéis ver como quedará el resultado final: 

![enter image description here](https://github.com/alejandro61299/Minimaps_Personal_Research/blob/master/docs/web_images/final%20result.gif?raw=true)

### Code Classes 
Tendemos dos clases que contendrán todo lo que necesitamos:

La primera será la principal  ```classMinimap```que servirá como factory para       los Minimap_Indicators

```cpp
class Minimap : public Module
{
public:

	Minimap(
		const SDL_Rect minimap_rect,
		const float texture_width,
		const PROJECTION_TYPE projection_type = PROJECTION_TYPE::ISOMETRIC,
		const SHAPE_TYPE shape_type = SHAPE_TYPE::RECTANGLE,
		const INTERACTION_TYPE interaction_type = INTERACTION_TYPE::MOUSE_DRAG,
		Object* target = nullptr);
	~Minimap();
	
	bool PreUpdate() override;
	bool Update(float dt) override;
	bool PostUpdate(float dt) override;
	
public:

	void	AddIndicator(const fPoint map_pos, const SDL_Rect icon_rect = { 0,0,0,0 }, const SDL_Color color = { 0,0,0,0 }, Object*  target = nullptr);

	void	SetInteractionType(const INTERACTION_TYPE new_type);
	void	SetShapeType(const SHAPE_TYPE new_type);
	fPoint  GetTextureScreenPos();

	// Mesures transformations methods ==========================

	fPoint	MapToMinimap(const float x, const float y);
	fPoint	MinimapToMap(const float x, const float y);
	fPoint	WorldToMinimap(const float x, const float y);
	fPoint	MinimapToWorld(const float x, const float y);

private:

	void	MouseDragInput( float dt);
	bool	LoadMinimap();
	bool    LoadMinimapData();
	bool	LoadMinimapTexture();
	void	UpdateMinimapTexture();
	
public:

	Object*             target_to_follow = nullptr;

private:
	Camera*             camera = nullptr;
	fPoint				camera_target_pos = { 0,0 };

	// General info ======================================

	SDL_Rect            minimap_rect = { 0, 0, 0,0 };   // - Determinate the minimap position on screen & its scope
	bool                minimap_loaded = false;         

	PROJECTION_TYPE		projection_type = PROJECTION_TYPE::ISOMETRIC;
	SHAPE_TYPE			shape_type = SHAPE_TYPE::RECTANGLE;
	INTERACTION_TYPE	interaction_type = INTERACTION_TYPE::MOUSE_DRAG;

	// Textures info ====================================

	SDL_Texture*		minimap_texture = nullptr;      // - Scaled map texture
	SDL_Texture*		alpha_mask_texture = nullptr;   // - Mask texture used to aplly alpha mask
	SDL_Texture*        final_texture = nullptr;        // - Final texture with masks
	SDL_Texture*		icons_texture = nullptr;

	float				x_offset = 0;                   // - Offset between texture and his map 0,0 represented in pixels
	fPoint				texture_pos = { 0.f, 0.f };     // - Texture position respect minimap_rect position
	float				texture_width = 0.f;            
	float				texture_height = 0.f;

	float				minimap_tile_width = 0.f;
	float				minimap_tile_height = 0.f;

	float				aspect_ratio_x = 0.f;           
	float				aspect_ratio_y = 0.f;

	// Values ===========================================

	bool                allow_interaction = false;

	// Indicators =======================================

	std::list<Minimap_Indicator*>	indicators_list;

};
```
## Links to more Documentation

- [Where Should We Place the Mini-Map? ( Gamasutra Blog )](https://www.gamasutra.com/blogs/JacekSliwinski/20130121/185119/Where_should_we_place_the_mini_map.php)
- [Video Game Mini-Maps Might Finally Be Going Away ( Kotaku Article )](https://kotaku.com/video-game-mini-maps-might-finally-be-going-away-1820011897)
- [Horizontal Atention Leans Left ( Nielsen Norman Group Article ) ](http://www.useit.com/alertbox/horizontal-attention.html) 
- [Mini-Map by Jack Davies ( Game UI Patterns Article )](https://gameuipatterns.com/gameui/mini-map/)
- [Following the Little Dotted Line ( Video )](https://www.youtube.com/watch?v=FzOCkXsyIqo)
- [Game Design Affect Minimap Design | Black Ops 4 Minimap ( Dexerto Article ) ](https://www.dexerto.com/call-of-duty/treyarch-dev-reveals-why-there-is-no-vsat-blackbird-in-black-ops-4-mutilplayer-184986)
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTgyNTU0Mzc1MSwtMTI1Nzc3MjYyOSwtMT
cyNzYwNjU2NSwtMTA5NzQ1NjQ5OCwxMjg2MzcxNTQsODUzOTYx
ODA4LC0yMDMxMjM0OTcyLDQwMTg4NTcwNCwxMTU5NDEwMjAwLD
E0NTMwNjY0NjIsMTI3MzExMTc1OCwxMjgyMjYxNTgyLC03NTQ3
NjcwMDEsMTE0OTAwMjcxNSwtMTcyOTEyMTI0MywxMTIxNDk3MT
gsLTk0MjYwNDc1NSwtMTcxNzYxMjc1NiwtNjM4NzY4MTE5LC0z
NjE2NDk1NDddfQ==
-->