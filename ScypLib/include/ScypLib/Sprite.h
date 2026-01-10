#pragma once
#include<algorithm>
#include<vector>
#include<cassert>
#include<unordered_map>

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
		Sprite(Texture* texture, float angle, Vec2f pos, Vec2f pivot, Vec2f size,
			RectF uv, Color colorTint, bool flipX, bool flipY)
			: texture(texture), angle(angle), pos(pos), pivot(pivot), size(size), uv(uv), colorTint(colorTint), flipX(flipX), flipY(flipY)
		{
			this->uv = uv / Vec2f(float(texture->GetWidth()), float(texture->GetHeight()));
		}

		~Sprite() = default;

		Texture* GetTexture() const { return texture; }
		Shader* GetShader() const { return shader; }
		Vec2f GetPos() const { return pos; }
		Vec2f GetSize() const { return size; }
		Vec2f GetPivot() const { return pivot; }
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
		void SetUV(const RectF& uv)
		{
			this->uv = uv;
		}
		void SetPixelUV(const RectF& uv)
		{
			assert(texture);
			this->uv = uv / Vec2f(float(texture->GetWidth()), float(texture->GetHeight()));
		}
		void SetPivot(Vec2f pivot)
		{
			this->pivot = pivot;
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
		Vec2f pivot = { 0,0 };
		Vec2f size = { 0,0 };
		RectF uv = { 0.0f,1.0f,0.0f,1.0f };
		Color colorTint = Colors::White;
		bool flipX = false;
		bool flipY = false;
	};

	enum class AnimationMode
	{
		Once,
		Loop
	};
	struct Animation
	{
	public:
		Animation() = default;
		Animation(Vec2f size, float animationTime, int nFrames, Texture* texture, Vec2f uvStart = {0.0f,0.0f})
			: animationTime(animationTime), atlas(texture), nFrames(nFrames)
		{
			assert(nFrames > 0);
			assert(texture);
			assert(animationTime > 0.0f && "AnimatedSprite needs to have animation time > 0");
			int framesX = texture->GetWidth() / int(size.x);
			int framesY = texture->GetHeight() / int(size.y);
			frameTime = animationTime / float(nFrames);
			int framesLeft = nFrames;
			frameUVs.reserve(nFrames);

			for (int i = int(uvStart.y); i + size.y <= texture->GetHeight() && framesLeft > 0; i += int(size.y))
			{
				for (int j = int(uvStart.x); j + size.x <= texture->GetWidth() && framesLeft > 0; j += int(size.x))
				{
					frameUVs.emplace_back(RectF(float(j), float(j + size.x), float(i), float(i + size.y)));
					framesLeft--;
				}
				uvStart.x = 0.0f;
			}
		}
		~Animation() = default;
		void Update(float dt)
		{
			if (paused) return;
			currentTime += dt;
			if (currentTime > animationTime)
			{
				if (currentTime > animationTime && AnimationMode::Once == mode) paused = true;
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
		void SetFrame(int n)
		{
			assert(n < nFrames && n >= 0);
			currentFrame = n;
			currentTime = currentFrame * frameTime;
		}
		int CurrentFrame() const
		{
			return currentFrame;
		}
		int Length()
		{
			return nFrames;
		}
		void Pause()
		{
			paused = true;
		}
		void Resume()
		{
			paused = false;
		}
		void Reset()
		{
			paused = false;
			currentFrame = 0;
			currentTime = 0.0f;
		}
		void SetAnimationMode(AnimationMode mode)
		{
			this->mode = mode;
		}
		Texture* GetTexture() const
		{
			return atlas;
		}
		RectF GetCurrentUV() const { return frameUVs[currentFrame]; }
	private:
		std::vector<RectF> frameUVs;
		float currentTime = 0.0f;
		int currentFrame = 0;
		int nFrames = 0;
		float animationTime = 0.0f;
		float frameTime = 0.0f;
		Texture* atlas = nullptr;
		bool paused = false;
		AnimationMode mode = AnimationMode::Loop;
	};

	class AnimatedSprite : public Sprite
	{
	public:
		AnimatedSprite() = default;
		~AnimatedSprite() = default;

		void Update(float dt)
		{
			assert(curAnimation);
			curAnimation->Update(dt);
			SetPixelUV(curAnimation->GetCurrentUV());
		}
		void AdvanceFrames(int n)
		{
			assert(curAnimation);
			curAnimation->AdvanceFrames(n);
			SetPixelUV(curAnimation->GetCurrentUV());
		}
		void AddAnimation(const Animation& animation, const std::string& id)
		{
			animations[id] = animation;
			if (animations.size() == 1) curAnimation = &animations[id];
		}
		void RemoveAnimation(const std::string& id)
		{
			assert(animations.contains(id));
			if (&animations[id] == curAnimation) curAnimation = nullptr;
			animations.erase(id);
		}
		void SetFrame(int n)
		{
			assert(curAnimation);
			curAnimation->SetFrame(n);
			SetPixelUV(curAnimation->GetCurrentUV());
		}
		void Play(const std::string& id, AnimationMode mode)
		{
			assert(animations.contains(id));
			curAnimation = &animations[id];
			curAnimation->SetAnimationMode(mode);
			SetTexture(curAnimation->GetTexture());
			curAnimation->Reset();

		}
		void Play(const std::string& id)
		{
			assert(animations.contains(id));
			curAnimation = &animations[id];
			SetTexture(curAnimation->GetTexture());
			
			curAnimation->Reset();
		}
		void Stop()
		{
			assert(curAnimation);
			curAnimation->Reset();
			curAnimation->Pause();
		}
		void Pause()
		{
			assert(curAnimation);
			curAnimation->Pause();
		}
		void Resume()
		{
			assert(curAnimation);
			curAnimation->Resume();
		}
	private:
		std::unordered_map<std::string, Animation> animations;
		Animation* curAnimation = nullptr;
	};
}