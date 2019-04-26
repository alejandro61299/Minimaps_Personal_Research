#ifndef __M_TEXTURES_H__
#define __M_TEXTURES_H__

#include "Module.h"
#include <map>
#include <string>
#include "Point.h"
#include "Defs.h"
#include "SDL/include/SDL_stdinc.h"

struct SDL_Texture;
struct SDL_Surface;
struct SDL_Rect;

enum Textures
{
	TANK_BASE,
	TANK_TURR,
};

enum class TEXTURE_TYPE
{
	NORMAL,
	TEXT,
	STREAMING
};

class M_Textures : public Module
{
public:

	M_Textures();

	virtual ~M_Textures();

	bool Awake(pugi::xml_node&) override;

	bool Start() override;

	bool CleanUp() override;

	// Manipulate textures methods ===================================

	SDL_Texture* const	Load(const char* path);

	bool				UnLoad(SDL_Texture* texture, TEXTURE_TYPE type = TEXTURE_TYPE::NORMAL);

	SDL_Texture * const LoadTextSurface(SDL_Surface* surface);

	void				GetSize(const SDL_Texture* texture, uint& width, uint& height) const;

	SDL_Texture       * LoadStreamingTex(const char * path);

	bool                LockTexture(SDL_Texture * tex, const SDL_Rect * clipRect, void ** pixels, int & pitch);

	bool                UnlockTexture(SDL_Texture * tex);

	void                CopyTextureOn(SDL_Texture * target_texture, SDL_Texture * source_texture, const iPoint trg_point, const iPoint src_point, const int w, const int h);

	SDL_Texture       * CreateStreamingTexture(const int texture_width, const int texture_height);


private:

	SDL_Texture* const	LoadSurface(SDL_Surface* surface, std::string path);
	Uint32                pixel_format;

public:
			//Path		  //Texture pointer
	std::map<std::string, SDL_Texture *> textures;

	std::list<SDL_Texture*> streaming_textures;

	std::list<SDL_Texture*> text_textures;
};


#endif // __j1TEXTURES_H__