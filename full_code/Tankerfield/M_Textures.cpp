#include "Brofiler/Brofiler.h"

#include "Defs.h"
#include "Log.h"
#include "App.h"
#include "M_Render.h"
#include "M_Textures.h"
#include "M_Window.h"

#include "SDL_image/include/SDL_image.h"
#pragma comment( lib, "SDL_image/libx86/SDL2_image.lib" )

M_Textures::M_Textures() : Module()
{
	name = "textures";
}

// Destructor
M_Textures::~M_Textures()
{}

// Called before render is available
bool M_Textures::Awake(pugi::xml_node& config)
{
	LOG("Init Image library");
	bool ret = true;
	// load support for the PNG image format
	int flags = IMG_INIT_PNG;
	int init = IMG_Init(flags);

	if((init & flags) != flags)
	{
		LOG("Could not initialize Image lib. IMG_Init: %s", IMG_GetError());
		ret = false;
	}

	pixel_format = SDL_PIXELFORMAT_ARGB8888;

	return ret;
}

// Called before the first frame
bool M_Textures::Start()
{
	LOG("start textures");
	bool ret = true;

	return ret;
}

// Called before quitting
bool M_Textures::CleanUp()
{
	LOG("Freeing textures and Image library");

	for(std::map<std::string, SDL_Texture*>::iterator iter = textures.begin(); iter != textures.end();)
	{
		SDL_DestroyTexture(iter->second);
		iter->second = nullptr;
		iter = textures.erase(iter);
	}
	textures.clear();

	for (std::list<SDL_Texture*>::iterator iter = text_textures.begin(); iter != text_textures.end();)
	{
		SDL_DestroyTexture((*iter));
		(*iter) = nullptr;
		iter = text_textures.erase(iter);
	}
	text_textures.clear();

	IMG_Quit();
	return true;
}

// Load new texture from file path
SDL_Texture* const M_Textures::Load(const char* path)
{
	SDL_Texture* texture = nullptr;

	std::map<std::string, SDL_Texture *>::iterator iter = textures.find(path);

	if (iter != textures.end())
	{
		//Return the texture if it's already been loaded onto the map
		texture = iter->second;
	}
	else
	{
		SDL_Surface* surface = IMG_Load(path);
		//Load a new texture if there isn't a texture loaded with that path
		if (surface == nullptr)
		{
			LOG("Could not load surface with path: %s. IMG_Load: %s", path, IMG_GetError());
		}
		else
		{
			texture = LoadSurface(surface, path);
			SDL_FreeSurface(surface);
		}
	}

	return texture;
}

// Unload texture
bool M_Textures::UnLoad(SDL_Texture* texture, TEXTURE_TYPE type )
{

	if (type == TEXTURE_TYPE::NORMAL)
	{
		for (std::map<std::string, SDL_Texture *>::iterator iter = textures.begin(); iter != textures.end(); ++iter)
		{
			if ((iter->second) == texture)
			{
				SDL_DestroyTexture((iter->second));
				iter->second = nullptr;
				textures.erase(iter);
				return true;
			}
		}
	}
	else if (type == TEXTURE_TYPE::STREAMING)
	{
		std::list<SDL_Texture*>::iterator texture_found = std::find(streaming_textures.begin(), streaming_textures.end(), texture);

		if (texture_found != text_textures.end())
		{
			SDL_DestroyTexture(*texture_found);
			text_textures.erase(texture_found);
		}
	}
	else if (type == TEXTURE_TYPE::TEXT)
	{
		std::list<SDL_Texture*>::iterator texture_found = std::find(text_textures.begin(), text_textures.end(), texture);

		if (texture_found != text_textures.end())
		{
			SDL_DestroyTexture(*texture_found);
			text_textures.erase(texture_found);
		}
	}

	return true;
}

// Translate a surface into a texture
SDL_Texture* const M_Textures::LoadSurface(SDL_Surface* surface, std::string path)
{
	BROFILER_CATEGORY("M_TexturesLoad", Profiler::Color::LightPink)
	SDL_Texture* texture = SDL_CreateTextureFromSurface(app->render->renderer, surface);

	if(texture == NULL)
	{
		LOG("Unable to create texture from surface! SDL Error: %s\n", SDL_GetError());
	}
	else
	{
		textures[path] = texture;
	}

	return texture;
}

SDL_Texture * const M_Textures::LoadTextSurface(SDL_Surface* surface)
{
	BROFILER_CATEGORY("M_TexturesTextLoad", Profiler::Color::LightPink)
		SDL_Texture* texture = SDL_CreateTextureFromSurface(app->render->renderer, surface);

	if (texture == NULL)
	{
		LOG("Unable to create texture from surface! SDL Error: %s\n", SDL_GetError());
	}
	else
	{
		text_textures.push_back(texture);
	}

	return texture;
}

// Retrieve size of a texture
void M_Textures::GetSize(const SDL_Texture* texture, uint& width, uint& height) const
{
	SDL_QueryTexture((SDL_Texture*)texture, NULL, NULL, (int*) &width, (int*) &height);
}


SDL_Texture* M_Textures::LoadStreamingTex(const char* path)
{
	SDL_Surface* loadedSurface = IMG_Load(path);

	//StreamTex* newStreamTex = nullptr; // test

	if (loadedSurface == NULL)
	{
		LOG("failed to load surface from path, %s", IMG_GetError());
	}
	else
	{
		// converto to display format
		SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(loadedSurface, pixel_format, NULL); //SDL_ConvertSurfaceFormat(loadedSurface, SDL_GetWindowPixelFormat(App->win->window), NULL);

		if (formattedSurface == NULL)
		{
			LOG("unable to convert surface format %s", SDL_GetError());
		}
		else
		{
			SDL_Texture* newTex = SDL_CreateTexture(app->render->renderer, pixel_format, SDL_TEXTUREACCESS_STREAMING, formattedSurface->w, formattedSurface->h);
			if (newTex == NULL)
			{
				LOG("failed to create blank streamable texture");
			}
			else
			{
				void* mPixels;
				int mPitch;
				int mWidth;
				int mHeight;

				SDL_SetTextureBlendMode((SDL_Texture *)newTex, SDL_BLENDMODE_BLEND);

				// lock texture for manipulation
				SDL_LockTexture(newTex, NULL, &mPixels, &mPitch);

				// copy loaded/formatted surface pixels
				memcpy(mPixels, formattedSurface->pixels, formattedSurface->pitch * formattedSurface->h);

				// unlock texture
				SDL_UnlockTexture(newTex);
				mPixels = nullptr;

				streaming_textures.push_back(newTex);

				SDL_FreeSurface(loadedSurface);
				SDL_FreeSurface(formattedSurface);

				return newTex;
			}

	}
		SDL_FreeSurface(loadedSurface);
	}

	return nullptr;
}

bool M_Textures::LockTexture(SDL_Texture* tex, const SDL_Rect* clipRect, void** pixels, int& pitch)
{

		if (SDL_LockTexture(tex, clipRect, pixels, &pitch) != 0)
		{
			LOG("unable to lock texture %s", SDL_GetError());
			return false;
		}

		return true;

}

bool M_Textures::UnlockTexture(SDL_Texture* tex)
{
	SDL_UnlockTexture(tex);
	return true;
}

SDL_Texture* M_Textures::CreateTargetTexture(const int width, const int height)
{
	SDL_Texture* ret = nullptr;
	ret = SDL_CreateTexture(app->render->renderer, pixel_format, SDL_TEXTUREACCESS_TARGET, width, height);

	if (ret != NULL)
		return ret;

	return nullptr;

}

