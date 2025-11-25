#pragma once
#include<string>

#include"Color.h"
namespace sl
{
	enum class TextureFilter
	{
		Nearest,
		Linear,
		NearestMipmapNearest,
		LinearMipmapNearest,
		NearestMipmapLinear,
		LinearMipmapLinear
	};

	enum class TextureWrap
	{
		ClampToEdge,
		ClampToBorder,
		Repeat,
		MirroredRepeat,
		MirrorClampToEdge
	};

	class Texture
	{
	public:
		Texture(int width, int height, int BPP, unsigned char* buffer, TextureWrap wrap = TextureWrap::ClampToEdge, TextureFilter minFilter = TextureFilter::Nearest, TextureFilter magFilter = TextureFilter::Nearest);
		Texture(const std::string& path, TextureWrap wrap = TextureWrap::ClampToEdge, TextureFilter minFilter = TextureFilter::Nearest, TextureFilter magFilter = TextureFilter::Nearest);
		~Texture();

		int GetWidth() const { return width; }
		int GetHeight() const { return height; }
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