#pragma once
#include<algorithm>
#include<vector>
#include<cassert>

#include"Rect.h"
#include"Texture.h"
#include"Color.h"
#include"Shader.h"

namespace sl
{
	class Sprite
	{
	public:
		Sprite() = default;
		Sprite(Texture* texture)
			: texture(texture)
		{
			SetSize(Vec2f(float(texture->GetWidth()), float(texture->GetHeight())));
		}
		Sprite(Texture* texture, float angle, Vec2f pos, Vec2f origin, Vec2f size,
			RectF uv, Color colorTint, bool flipX, bool flipY)
			: texture(texture), angle(angle), pos(pos), origin(origin), size(size), uv(uv), colorTint(colorTint), flipX(flipX), flipY(flipY)
		{
			this->uv = uv / Vec2f(float(texture->GetWidth()), float(texture->GetHeight()));
		}

		~Sprite() = default;

		Texture* GetTexture() const { return texture; }
		Shader* GetShader() const { return shader; }
		Vec2f GetPos() const { return pos; }
		Vec2f GetSize() const { return size; }
		Vec2f GetOrigin() const { return origin; }
		RectF GetRect() const { return RectF(pos, size.x, size.y); }
		RectF GetUV() const { return uv * Vec2f(float(texture->GetWidth()), float(texture->GetHeight())); }
		RectF GetNDCUV() const { return uv; }
		const Color& GetColorTint() const { return colorTint; }
		float GetRotation() const { return angle; }
		bool IsFlippedX() const { return flipX; }
		bool IsFlippedY() const { return flipY; }

		void SetRotation(float angle)
		{
			this->angle = angle;
		}
		void FlipX()
		{
			flipX = !flipX;
			std::swap(uv.left, uv.right);
		}
		void FlipY()
		{
			flipY = !flipY;
			std::swap(uv.top, uv.bottom);
		}
		void SetTexture(Texture* texture)
		{
			this->texture = texture;
		}
		void SetShader(Shader* shader)
		{
			this->shader = shader;
		}
		void SetPos(Vec2f pos)
		{
			this->pos = pos;
		}
		void SetSize(Vec2f size)
		{
			this->size = size;
		}
		void SetNDCUV(const RectF& uv)
		{
			this->uv = uv;
		}
		void SetUV(const RectF& uv)
		{
			assert(texture);
			this->uv = uv / Vec2f(float(texture->GetWidth()), float(texture->GetHeight()));
		}
		void SetOrigin(Vec2f origin)
		{
			this->origin = origin;
		}
		void SetColorTint(Color tint)
		{
			colorTint = tint;
		}
	private:
		Texture* texture = nullptr;
		Shader* shader = nullptr;
		float angle = 0.0f;
		Vec2f pos = { 0,0 };
		Vec2f origin = { 0,0 };
		Vec2f size = { 0,0 };
		RectF uv = { 0.0f,1.0f,0.0f,1.0f };
		Color colorTint = Colors::White;
		bool flipX = false;
		bool flipY = false;
	};

	struct Animation
	{
	public:
		Animation(int frameWidth, int frameHeight, float animationTime, Texture* texture)
			: animationTime(animationTime)
		{
			assert(animationTime != 0.0f && "AnimatedSprite needs to have animation time > 0");
			int framesX = texture->GetWidth() / frameWidth;
			int framesY = texture->GetHeight() / frameHeight;
			nFrames = framesX * framesY;
			frameTime = animationTime / float(nFrames);
			frameUVs.reserve(nFrames);

			for (int i = 0; i < framesY; i++)
			{
				for (int j = 0; j < framesX; j++)
				{
					frameUVs.emplace_back(RectF(float(j) * frameWidth, float(j + 1) * frameWidth, float(i) * frameHeight, float(i + 1) * frameHeight));
				}
			}
		}
		~Animation() = default;
		void Update(float dt)
		{
			currentTime += dt;
			if (currentTime > animationTime)
			{
				currentTime = fmod(currentTime, animationTime);
			}
			currentFrame = int(nFrames * currentTime / animationTime);
		}
		void AdvanceFrames(int n)
		{
			currentFrame += n;
			currentFrame = (currentFrame % nFrames + nFrames) % nFrames;
			currentTime = currentFrame * frameTime;
		}
		RectF GetCurrentUV() const { return frameUVs[currentFrame]; }
	private:
		std::vector<RectF> frameUVs;
		float currentTime = 0.0f;
		int currentFrame = 0;
		int nFrames = 0;
		float animationTime = 0.0f;
		float frameTime = 0.0f;
	};

	class AnimatedSprite : public Sprite
	{
	public:
		AnimatedSprite(int frameWidth, int frameHeight, float animationTime, Texture* texture)
			: Sprite(texture), animation(frameWidth, frameHeight, animationTime, texture)
		{
			SetUV(animation.GetCurrentUV());
		}
		AnimatedSprite(Texture* texture, Animation& animation)
			: Sprite(texture), animation(animation) {}
		~AnimatedSprite() = default;

		void Update(float dt)
		{
			animation.Update(dt);
			SetUV(animation.GetCurrentUV());
		}
		void AdvanceFrames(int n)
		{
			animation.AdvanceFrames(n);
			SetUV(animation.GetCurrentUV());
		}
		void SetAnimation(Animation& animation)
		{
			this->animation = animation;
		}
	private:
		Animation animation;
	};
}