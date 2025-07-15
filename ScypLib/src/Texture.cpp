#include<cassert>

#define STB_IMAGE_IMPLEMENTATION
#include"stb/stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include"stb/stb_truetype.h"
#include<GL/glew.h>

#include"ScypLib/Texture.h"

namespace sl
{
	Texture::Texture(int width, int height, int BPP, unsigned char* buffer, TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter)
		: width(width), height(height), BPP(BPP)
	{
		Init(buffer, wrap, minFilter, magFilter);
	}

	Texture::Texture(const std::string& path, TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter)
	{
		stbi_set_flip_vertically_on_load(1);
		unsigned char* buffer = stbi_load(path.c_str(), &width, &height, &BPP, 0);
		assert(buffer);
		Init(buffer, wrap, minFilter, magFilter);
		stbi_image_free(buffer);
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &handle);
	}

	void Texture::Init(const unsigned char* buffer, TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter)
	{
		glGenTextures(1, &handle);
		glBindTexture(GL_TEXTURE_2D, handle);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, unsigned int(minFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, unsigned int(magFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, unsigned int(wrap));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, unsigned int(wrap));

		if (BPP == 4) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)buffer);
		else if (BPP == 3) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (const void*)buffer);
		else if (BPP == 1) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, (const void*)buffer);
		if (minFilter == TextureFilter::NearestMipmapLinear || minFilter == TextureFilter::NearestMipmapNearest ||
			minFilter == TextureFilter::LinearMipmapNearest || minFilter == TextureFilter::LinearMipmapLinear)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		if (BPP == 4)
		{
			{
				for (int i = 0; i < width * height * 4; i += 4)
				{
					unsigned char alpha = buffer[i + 3];
					if (alpha != 0 && alpha != 255)
					{
						binaryAlpha = false;
						break;
					}
				}
			}
		}
	}
}