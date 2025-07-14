#pragma once
#include<string>

#include<GL/glew.h>

namespace sl
{
	enum class TextureFilter
	{
		Nearest = GL_NEAREST,
		Linear = GL_LINEAR,
		NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
		LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
		NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
		LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
	};

	enum class TextureWrap
	{
		ClampToEdge = GL_CLAMP_TO_EDGE,
		ClampToBorder = GL_CLAMP_TO_BORDER,
		Repeat = GL_REPEAT,
		MirroredRepeat = GL_MIRRORED_REPEAT,
		MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE
	};

	class Texture
	{
	public:
		Texture(int width, int height, int BPP, unsigned char* buffer, TextureWrap wrap = TextureWrap::ClampToEdge, TextureFilter minFilter = TextureFilter::Nearest, TextureFilter magFilter = TextureFilter::Nearest);
		Texture(const std::string& path, TextureWrap wrap = TextureWrap::ClampToEdge, TextureFilter minFilter = TextureFilter::Nearest, TextureFilter magFilter = TextureFilter::Nearest);
		~Texture();

		inline int GetWidth() const { return width; }
		inline int GetHeight() const { return height; }
		unsigned int GetHandle() const { return handle; }
		int GetChannels() const { return BPP; }
		bool IsBinaryAlpha() const { return binaryAlpha; }
	private:
		void Init(const unsigned char* buffer, TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter);
	private:
		unsigned int handle = 0;
		int width = 0;
		int height = 0;
		int BPP = 0;//bits per pixel
		bool binaryAlpha = true;
	};
}