#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<memory>
#include <stdexcept>
#include<bit>
#include <cstddef>
#include <cstdint>
#include<stb/stb_truetype.h>

#include"ScypLib/Graphics.h"

namespace sl
{
	Graphics::Graphics(Window* wnd)
		: Graphics(wnd, float(wnd->GetWidth()), float(wnd->GetHeight())) {}

	Graphics::Graphics(Window* wnd, float canvasWidth, float canvasHeight)
		: window(wnd), canvasWidth(canvasWidth), canvasHeight(canvasHeight)
	{
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureSlots);
		for (int i = 0; i < maxTextureSlots; i++) availableSlots.insert(i);
		SetVSyncInterval(1);
		unsigned char whiteTexture[3] = { 255,255,255 };
		blankTexture = CreateTextureFromMemory(1, 1, 3, whiteTexture, TextureWrap::ClampToEdge, TextureFilter::Nearest, TextureFilter::Nearest);
		
		const std::string vertexShader = R"(
			#version 330 core
			
			layout(location = 0) in vec3 aPosition;

			layout(location = 1) in vec4 iUV;
			layout(location = 2) in vec4 iColor;
			layout(location = 3) in mat4 iModel;		

			layout(std140) uniform CameraBuffer 
			{
			    mat4 view;
			    mat4 projection;
			};
			
			out vec2 vTexCoord;	
			out vec4 vColorTint;
			
			void main()
			{
			    gl_Position = projection * view * iModel * vec4(aPosition, 1.0);
				int corner = gl_VertexID & 3;
				vec2 uv;
				vec2 local01 = aPosition.xy + vec2(0.5, 0.5);
   
				uv.x = mix(iUV.x, iUV.y, local01.x);
				uv.y = mix(iUV.w, iUV.z, local01.y);
				vTexCoord = uv;
			    
			    vColorTint = iColor;
			}
			)";
		const std::string fragmentShader = R"(
			#version 330 core
			
			in vec2 vTexCoord;
			in vec4 vColorTint;
			
			out vec4 FragColor;
			uniform sampler2D uTexture;
			
			void main()
			{
			    vec4 texColor = texture(uTexture, vTexCoord);
			    vec4 finalColor = texColor * vColorTint;
			
			    if (finalColor.a < 0.1) discard;
			
			    FragColor = finalColor;
			}
			)";

		builtInShader = LoadShader(vertexShader, fragmentShader, false);
		SetDefaultShader(builtInShader);
		currentShader = defaultShader;

		constexpr float quadVertices[] =
		{
			-0.5f, -0.5f, 0.0f,  // top-left
			 0.5f, -0.5f, 0.0f,  // top-right
			 0.5f,  0.5f, 0.0f,  // bottom-right

			-0.5f, -0.5f, 0.0f,  // top-left
			 0.5f,  0.5f, 0.0f,  // bottom-right
			-0.5f,  0.5f, 0.0f   // bottom-left
		};

		glGenVertexArrays(1, &quadVao);
		BindVertexArray(quadVao);
		glGenBuffers(1, &quadVbo);
		BindVertexBuffer(quadVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), (void*)0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &quadInstanceVbo);
		
		BindVertexBuffer(quadInstanceVbo);
		glBufferData(GL_ARRAY_BUFFER, maxQuadsInBatch * sizeof(QuadInstanceData), nullptr, GL_DYNAMIC_DRAW);

		BindVertexArray(quadVao);
		//uvs
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(QuadInstanceData), (void*)offsetof(QuadInstanceData, uv));
		glVertexAttribDivisor(1, 1);

		//color
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(QuadInstanceData), (void*)offsetof(QuadInstanceData, color));
		glVertexAttribDivisor(2, 1);

		//mat4f
		for (int i = 0; i < 4; i++)
		{
			glEnableVertexAttribArray(3 + i);
			glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(QuadInstanceData), (void*)(offsetof(QuadInstanceData, model) + sizeof(float) * 4 * i));
			glVertexAttribDivisor(3 + i, 1);
		}

		glGenBuffers(1, &vpMatUbo);
		BindUniformBuffer(vpMatUbo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(ViewProjMat), &vpMat, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, vpMatUboBindingPoint, vpMatUbo);

		SetCanvasSize(Vec2f(1.0f, 1.0f));//SMTHING BUGGER IF CALLED TWICE IT WORKS PROPERLY OR OUTSIDE OF CONSTRUCTOR
		SetCanvasSize(Vec2f(window->GetSize()));

		quadInstanceDataBuffer.reserve(maxQuadsInBatch * 4);
	}

	Graphics::QuadVertex::QuadVertex(Vec3f pos)
		: pos(pos) {}

	bool Graphics::QuadVertex::operator==(const QuadVertex& other) const
	{
		return pos == other.pos;
	}

	Graphics::~Graphics()
	{
		glDeleteFramebuffers(1, &fbo);
		glDeleteRenderbuffers(1, &rbo);
		glDeleteVertexArrays(1, &quadVao);
		glDeleteBuffers(1, &quadVbo);
		glDeleteBuffers(1, &quadInstanceVbo);
		glDeleteBuffers(1, &vpMatUbo);
		ClearTextures();
	}

	void Graphics::BeginFrame()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, GLsizei(GetCanvasWidth()), GLsizei(GetCanvasHeight()));
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Graphics::EndFrame(Shader* shader)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		float scale = window->GetHeight() / GetCanvasHeight();
		float width = GetCanvasWidth() * scale;
		float pillar = (float(window->GetWidth()) - width) * 0.5f;
		glViewport(GLint(pillar), 0, GLsizei(width), GLsizei(window->GetHeight()));
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		if (!shader) shader = defaultShader;
		DrawTexture(GetCanvasRect(), framebufferTexture, shader);
		RenderQuads();
		glfwSwapBuffers(reinterpret_cast<GLFWwindow*>(window->GetWindowBackend()));
		glEnable(GL_DEPTH_TEST);
	}

	void Graphics::EndFrame(std::vector<Shader*>& shaders)
	{
		ApplyPostProcessing(shaders);
		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, window->GetWidth(), window->GetHeight());
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		DrawTexture(GetCanvasRect(), framebufferTexture, defaultShader);
		RenderQuads();
		glfwSwapBuffers(reinterpret_cast<GLFWwindow*>(window->GetWindowBackend()));
		glEnable(GL_DEPTH_TEST);
	}

	void Graphics::BeginView(Vec2f cameraPosition, float zoom)
	{
		drawMode = DrawMode::Sprite2d;
		cam2d.pos = cameraPosition;
		cam2d.zoom = zoom;
		vpMat.view = Mat4f(1.0f);
		Vec2f center = Vec2f(GetCanvasWidth() / 2.0f, GetCanvasHeight() / 2.0f);
		vpMat.view.Translate(Vec3f(center.x, center.y, 0.0f));
		vpMat.view.Scale(Vec3f(zoom, zoom, 1.0f));
		vpMat.view.Translate(Vec3f(-center.x, -center.y, 0.0f));
		vpMat.view.Translate(Vec3f(-cameraPosition.x, -cameraPosition.y, 0.0f));
		BindUniformBuffer(vpMatUbo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(vpMat), &vpMat);
	}

	void Graphics::EndView(std::vector<Shader*>& shaders)
	{
		if (drawMode == DrawMode::Sprite2d)
		{
			RenderQuads();
		}
		else if (drawMode == DrawMode::Quad3d)
		{
			RenderQuads();
		}
		drawMode = DrawMode::Sprite2d;
		vpMat.view = Mat4f(1.0f);
		cam2d.pos = { 0.0f,0.0f };
		cam2d.zoom = 1.0f;
		BindUniformBuffer(vpMatUbo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(vpMat), &vpMat);
		ApplyPostProcessing(shaders);
	}

	void Graphics::EndView(Shader* shader)
	{
		if (drawMode == DrawMode::Sprite2d)
		{
			RenderQuads();
		}
		else if (drawMode == DrawMode::Quad3d)
		{
			RenderQuads();
		}
		drawMode = DrawMode::Sprite2d;
		vpMat.view = Mat4f(1.0f);
		cam2d.pos = { 0.0f,0.0f };
		cam2d.zoom = 1.0f;
		BindUniformBuffer(vpMatUbo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(vpMat), &vpMat);
		glDisable(GL_DEPTH_TEST);
		if (!shader) shader = defaultShader;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTextureSecondary->GetHandle(), 0);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		DrawTexture(GetCanvasRect(), framebufferTexture, shader);
		RenderQuads();
		std::swap(framebufferTexture, framebufferTextureSecondary);
		glEnable(GL_DEPTH_TEST);
	}

	void Graphics::SetDrawDepth(float layer)
	{
		curDrawDepth = layer;
	}

	void Graphics::SetCanvasSize(Vec2f size)
	{
		UpdateCanvasSize(size.x, size.y);
	}

	void Graphics::SetCanvasWidth(float width)
	{
		UpdateCanvasSize(width, canvasHeight);
	}

	void Graphics::SetCanvasHeight(float height)
	{
		UpdateCanvasSize(canvasWidth, height);
	}

	void Graphics::SetVSyncInterval(int interval)
	{
		glfwSwapInterval(interval);
	}

	void Graphics::ApplyPostProcessing(std::vector<Shader*>& shaders)
	{
		glDisable(GL_DEPTH_TEST);
		Texture* currentTarget = framebufferTextureSecondary;
		Texture* otherTarget = framebufferTexture;
		for (Shader* shader : shaders)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, currentTarget->GetHandle(), 0);
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			DrawTexture(GetCanvasRect(), otherTarget, shader);
			RenderQuads();
			std::swap(currentTarget, otherTarget);
		}
		if (otherTarget != framebufferTexture) std::swap(framebufferTexture, framebufferTextureSecondary);
		glEnable(GL_DEPTH_TEST);
	}

	void Graphics::SetDefaultFont(Font* font)
	{
		defaultFont = font;
	}

	void Graphics::SetDefaultShader(Shader* shader)
	{
		if (!shader) defaultShader = builtInShader;
		else defaultShader = shader;
	}

	void Graphics::DrawTexture(float x, float y, const Texture* texture)
	{
		assert(drawMode == DrawMode::Sprite2d && "Attempt to draw in 2d in non 2d view");
		assert(texture && "Failed to draw texture. Texture is nullptr");
		Mat4f model(1.0f);
		model.Translate(Vec3f(x + float(texture->GetWidth()) * 0.5f, y + float(texture->GetHeight()) * 0.5f, curDrawDepth));
		model.Scale(Vec3f(float(texture->GetWidth()), float(texture->GetHeight()), 1.0f));
		auto renderable = std::make_unique<QuadRenderable>(model, texture, RectF(0.0f, 1.0f, 0.0f, 1.0f), Colors::White, curDrawDepth);
		if (texture->IsBinaryAlpha()) opaqueQuads[defaultShader].emplace_back(std::move(renderable));
		else transparentQuads[defaultShader].emplace_back(std::move(renderable));
	}

	void Graphics::DrawTexture(Vec2f pos, Vec2f size, const Texture* texture, Shader* shader, bool flipX, bool flipY, float angle, Vec2f origin, const RectF* uv, const Color& tint)
	{
		assert(drawMode == DrawMode::Sprite2d && "Attempt to draw in 2d in non 2d view");
		assert(texture && "Failed to draw texture. Texture is nullptr");
		RectF finalUV(0.0f, 1.0f, 0.0f, 1.0f);

		Mat4f model(1.0f);
		
		if (!shader) shader = defaultShader;
		if (uv) finalUV = *uv / Vec2f(float(texture->GetWidth()), float(texture->GetHeight()));
		if (flipX) std::swap(finalUV.left, finalUV.right);
		if (flipY) std::swap(finalUV.top, finalUV.bottom);
		
		
		model.Translate(Vec3f(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f, curDrawDepth));
		model.Scale(Vec3f(size, 1.0f));
		model.Rotate(Vec3f(0.0f, 0.0f, ToRadians(angle)));
		
		auto renderable = std::make_unique<QuadRenderable>(model, texture, finalUV, tint, curDrawDepth);
		if (texture->IsBinaryAlpha() && (tint.a == 1.0f || tint.a == 0.0f)) opaqueQuads[shader].emplace_back(std::move(renderable));
		else transparentQuads[shader].emplace_back(std::move(renderable));
	}

	void Graphics::DrawTexture(const RectF& targetRect, const Texture* texture, Shader* shader, bool flipX, bool flipY, float angle, Vec2f origin, const RectF* uv, const Color& tint)
	{
		DrawTexture({ targetRect.left, targetRect.top }, { targetRect.GetWidth(), targetRect.GetHeight() }, texture, shader, flipX, flipY, angle, origin, uv, tint);
	}

	void Graphics::DrawSprite(const Sprite& sprite)
	{
		assert(drawMode == DrawMode::Sprite2d && "Attempt to draw in 2d in non 2d view");
		assert(sprite.GetTexture() && "Failed to draw sprite. Texture is nullptr");
		Mat4f model(1.0f);
		Vec2f pos = sprite.GetPos();
		Vec2f size = sprite.GetSize();
		assert(size.x > 0.0f && size.y > 0.0f);
		
		Shader* shader = sprite.GetShader();
		if (!shader) shader = defaultShader;
		Vec2f origin = sprite.GetOrigin();
		model.Translate(Vec3f(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f, curDrawDepth));
		model.Scale(Vec3f(size, 1.0f));
		model.Rotate(Vec3f(0.0f, 0.0f, ToRadians(sprite.GetRotation())));
			
		auto renderable = std::make_unique<QuadRenderable>(model, sprite.GetTexture(), sprite.GetNDCUV(), sprite.GetColorTint(), curDrawDepth);

		if (sprite.GetTexture()->IsBinaryAlpha() && (sprite.GetColorTint().a == 1.0f || sprite.GetColorTint().a == 0.0f)) opaqueQuads[shader].emplace_back(std::move(renderable));
		else transparentQuads[shader].emplace_back(std::move(renderable));
	}

	void Graphics::DrawAnimatedSprite(const AnimatedSprite& animatedSprite)
	{
		assert(drawMode == DrawMode::Sprite2d && "Attempt to draw in 2d in non 2d view");
		assert(animatedSprite.GetTexture() && "Failed to draw sprite. Texture is nullptr");
		Mat4f model(1.0f);
		Vec2f pos = animatedSprite.GetPos();
		Vec2f size = animatedSprite.GetSize();
		assert(size.x > 0.0f && size.y > 0.0f);
		RectF uv = animatedSprite.GetNDCUV();
		Shader* shader = animatedSprite.GetShader();
		if (!shader) shader = defaultShader;
		if (animatedSprite.IsFlippedX()) std::swap(uv.left, uv.right);
		if (animatedSprite.IsFlippedY()) std::swap(uv.top, uv.bottom);

		model.Translate(Vec3f(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f, curDrawDepth));
		model.Scale(Vec3f(size, 1.0f));
		model.Rotate(Vec3f(0.0f, 0.0f, ToRadians(animatedSprite.GetRotation())));

		auto renderable = std::make_unique<QuadRenderable>(model, animatedSprite.GetTexture(), animatedSprite.GetNDCUV(), animatedSprite.GetColorTint(), curDrawDepth);

		if (animatedSprite.GetTexture()->IsBinaryAlpha() && (animatedSprite.GetColorTint().a == 1.0f || animatedSprite.GetColorTint().a == 0.0f)) opaqueQuads[shader].emplace_back(std::move(renderable));
		else transparentQuads[shader].emplace_back(std::move(renderable));
	}

	void Graphics::DrawLine(float x1, float y1, float x2, float y2, float thickness, const Color& c, Shader* shader)
	{
		assert(drawMode == DrawMode::Sprite2d && "Attempt to draw in 2d in non 2d view");

		if (!shader)
			shader = defaultShader;

		float dx = x2 - x1;
		float dy = y2 - y1;

		float length = std::sqrt(dx * dx + dy * dy);
		if (length <= 0.0001f) return;

		float angle = std::atan2(dy, dx);

		float cx = (x1 + x2) * 0.5f;
		float cy = (y1 + y2) * 0.5f;

		Mat4f model(1.0f);

		model.Translate(Vec3f(cx, cy, curDrawDepth));
		model.Scale(Vec3f(length, thickness, 1.0f));
		model.Rotate(Vec3f(0.0f, 0.0f, angle));

		RectF uv(0.0f, 1.0f, 0.0f, 1.0f);

		auto renderable = std::make_unique<QuadRenderable>(model, blankTexture, uv, c, curDrawDepth );

		if (c.a == 1.0f || c.a == 0.0f) opaqueQuads[shader].emplace_back(std::move(renderable));
		else transparentQuads[shader].emplace_back(std::move(renderable));
	}

	void Graphics::DrawRect(const RectF& rect, const Color& c)
	{
		assert(drawMode == DrawMode::Sprite2d && "Attempt to draw in 2d in non 2d view");
		Mat4f model(1.0f);
		
		model.Translate(Vec3f(rect.left, rect.top, curDrawDepth));
		model.Scale(Vec3f(rect.GetSize(), 1.0f));

		auto renderable = std::make_unique<QuadRenderable>(model, blankTexture, RectF(0.0f, 1.0f, 0.0f, 1.0f), c, curDrawDepth);
		if (c.a == 0.0f || c.a == 1.0f) opaqueQuads[defaultShader].emplace_back(std::move(renderable));
		else transparentQuads[defaultShader].emplace_back(std::move(renderable));
	}

	void Graphics::DrawRect(Vec2f pos, Vec2f size, const Color& c)
	{
		DrawRect(RectF(pos, size.x, size.y), c);
	}

	void Graphics::DrawRect(Vec2f pos, Vec2f size, const Color& c, float angle, Shader* shader)
	{
		DrawRect(RectF(pos, size.x, size.y), c, angle, shader);
	}

	void Graphics::DrawRect(const RectF& rect, const Color& c, float angle, Shader* shader)
	{
		assert(drawMode == DrawMode::Sprite2d && "Attempt to draw in 2d in non 2d view");

		if (!shader) shader = defaultShader;
		Mat4f model(1.0f);	
		
		model.Translate(Vec3f(rect.left + rect.GetWidth() * 0.5f, rect.top + rect.GetHeight() * 0.5f, curDrawDepth));
		model.Scale(Vec3f(rect.GetSize(), 1.0f));
		model.Rotate(Vec3f(0.0f, 0.0f, ToRadians(angle)));

		auto renderable = std::make_unique<QuadRenderable>(model, blankTexture, RectF(0.0f, 1.0f, 0.0f, 1.0f), c, curDrawDepth);
		if (c.a == 0.0f || c.a == 1.0f) opaqueQuads[shader].emplace_back(std::move(renderable));
		else transparentQuads[shader].emplace_back(std::move(renderable));
	}

	void Graphics::DrawText(sl::Vec2f pos, const std::string& text, Font* font, float height, const Color& c)
	{
		assert(drawMode == DrawMode::Sprite2d && "Attempt to draw in 2d in non 2d view");
		if (!font) font = defaultFont;
		assert(font->GetTextureAtlas() && "Failed to draw text. Font atlas is nullptr");
		assert(font && "Failed to draw text. Both font and default font are nullptrs");

		const std::vector<Font::Glyph>& charData = font->GetCharData();
		Texture* atlas = font->GetTextureAtlas();
		float baseLineHeight = float(font->GetLineHeight());

		float scale = height / baseLineHeight;
		for (char ch : text)
		{
			if (ch < font->GetFirstChar() || ch >= font->GetLastChar()) continue;
			const Font::Glyph& glyph = charData[ch - font->GetFirstChar()];
			Vec2f size(glyph.rect.GetWidth() * scale, -glyph.rect.GetHeight() * scale);
			pos.y += (baseLineHeight * scale - size.y);
			DrawTexture(pos, size, atlas, nullptr, false, false, 0.0f, Vec2f(0, 0), &glyph.rect, c);

			pos.y -= (baseLineHeight * scale - size.y);
			pos.x += glyph.xadvance * scale;
		}
	}

	void Graphics::DrawText(const RectF& rect, const std::string& text, Font* font, const Color& c)
	{
		assert(drawMode == DrawMode::Sprite2d && "Attempt to draw in 2d in non 2d view");
		const auto& charData = font->GetCharData();
		char firstChar = font->GetFirstChar();
		char lastChar = font->GetLastChar();

		float width = 0.0f;
		float maxHeight = 0.0f;

		for (char ch : text)
		{
			if (ch < firstChar || ch >= lastChar) continue;
			const Font::Glyph& glyph = charData[size_t(ch - firstChar)];
			width += glyph.rect.GetWidth();
			maxHeight = std::max(maxHeight, -glyph.rect.GetHeight());
		}

		if (width <= 0.0f || maxHeight <= 0.0f) return;

		float scale = std::min(rect.GetWidth() / width, rect.GetHeight() / maxHeight);

		Vec2f scaledSize(width * scale, maxHeight * scale);
		Vec2f start(rect.left + (rect.GetWidth() - scaledSize.x) * 0.5f, rect.top + (rect.GetHeight() - scaledSize.y) * 0.5f);

		DrawText(start, text, font, scaledSize.y, c);
	}

	void Graphics::DrawText(Vec2f pos, Vec2f size, const std::string& text, Font* font, const Color& c)
	{
		DrawText(RectF(pos, size.x, size.y), text, font, c);
	}

	void Graphics::PutPixel(float x, float y, const Color& c)
	{
		assert(drawMode == DrawMode::Sprite2d && "Attempt to draw in 2d in non 2d view");
		Mat4f model(1.0f);
		model.Translate(Vec3f(x, y, curDrawDepth));
		auto renderable = std::make_unique<QuadRenderable>(model, blankTexture, RectF(0.0f, 1.0f, 0.0f, 1.0f), c, curDrawDepth);
		if (c.a == 1.0f) opaqueQuads[defaultShader].emplace_back(std::move(renderable));
		else transparentQuads[defaultShader].emplace_back(std::move(renderable));
	}

	Color Graphics::GetPixel(int x, int y)
	{
		unsigned char pixelData[4];
		glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
		int r = pixelData[0];
		int g = pixelData[1];
		int b = pixelData[2];
		int a = pixelData[3];
		Color c = Color::FromBytes(a, r, g, b);
		return c;
	}

	RectF Graphics::GetCanvasRect() const
	{
		return RectF(0.0f, canvasWidth, 0.0f, canvasHeight);
	}

	float Graphics::GetCanvasWidth() const
	{
		return canvasWidth;
	}

	float Graphics::GetCanvasHeight() const
	{
		return canvasHeight;
	}

	Vec2f Graphics::GetCanvasSize() const
	{
		return Vec2f(canvasWidth, canvasHeight);
	}

	void Graphics::UpdateCanvasSize(float width, float height)
	{
		if (width != canvasWidth || height != canvasHeight)
		{
			canvasWidth = width;
			canvasHeight = height;
			vpMat.projection = Ortho<float>(0.0f, canvasWidth, canvasHeight, 0.0f, -50.0f, 50.0f);
			BindUniformBuffer(vpMatUbo);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(vpMat), &vpMat);

			size_t size = size_t(canvasWidth) * size_t(canvasHeight) * 4;
			unsigned char* buffer = (unsigned char*)malloc(size);
			assert(buffer);
			if (buffer)
			{
				if (framebufferTexture) UnloadTexture(framebufferTexture);
				if (framebufferTextureSecondary) UnloadTexture(framebufferTextureSecondary);
				memset(buffer, 0, size);
				framebufferTexture = CreateTextureFromMemory(int(canvasWidth), int(canvasHeight), 4, buffer, TextureWrap::ClampToEdge, TextureFilter::Nearest, TextureFilter::Nearest);
				framebufferTextureSecondary = CreateTextureFromMemory(int(canvasWidth), int(canvasHeight), 4, buffer, TextureWrap::ClampToEdge, TextureFilter::Nearest, TextureFilter::Nearest);
				free(buffer);
			}
			assert(framebufferTexture);
			BindTexture(framebufferTexture);
			UseTexture(framebufferTexture);

			if (fbo != 0) glDeleteFramebuffers(1, &fbo);
			if (rbo != 0) glDeleteRenderbuffers(1, &rbo);

			glGenFramebuffers(1, &fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture->GetHandle(), 0);
			glGenRenderbuffers(1, &rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, int(canvasWidth), int(canvasHeight));
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				throw std::runtime_error("Frame buffer is not complete");
			}
		}
	}

	void Graphics::BindVertexArray(unsigned int vao)
	{
		if (boundVAO != vao)
		{
			glBindVertexArray(vao);
			boundVAO = vao;
		}
	}

	void Graphics::BindIndexBuffer(unsigned int ibo)
	{
		if (boundIBO != ibo)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			boundIBO = ibo;
		}
	}

	void Graphics::BindVertexBuffer(unsigned int vbo)
	{
		if (boundVBO != vbo)
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			boundVBO = vbo;
		}
	}

	void Graphics::BindShader(unsigned int shader)
	{
		if (boundShader != shader)
		{
			glUseProgram(shader);
			boundShader = shader;
		}
	}

	void Graphics::BindShaderStorageBuffer(unsigned int ssbo)
	{
		if (boundSSBO != ssbo)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
			boundSSBO = ssbo;
		}
	}

	void Graphics::BindUniformBuffer(unsigned int ubo)
	{
		if (boundUBO != ubo)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, ubo);
			boundUBO = ubo;
		}
	}

	void Graphics::ClearQuadBatchData()
	{
		quadInstanceDataBuffer.clear();
		opaqueQuads.clear();
		transparentQuads.clear();
	}

	void Graphics::RenderQuads()
	{
		Vec2f visibleSize(GetCanvasWidth() / cam2d.zoom, GetCanvasHeight() / cam2d.zoom);
		Vec2f cameraCenter = cam2d.pos + Vec2f(GetCanvasWidth() / 2, GetCanvasHeight() / 2);
		RectF visibleArea(cameraCenter - visibleSize / 2, visibleSize.x, visibleSize.y);

		const Texture* prevTexture = nullptr;

		if (!opaqueQuads.empty())
		{
			for (auto& [shader, renderables] : opaqueQuads)
			{
				std::sort(renderables.begin(), renderables.end(), [](const auto& a, const auto& b)
					{
						return a.get()->texture < b->texture;
					});
				currentShader = shader;
				BindShader(currentShader->GetHandle());
				for (auto& renderable : renderables)
				{
					if (prevTexture != renderable.get()->texture)
					{
						FlushQuadBatch();
						prevTexture = renderable.get()->texture;
						BindTexture(prevTexture);
						currentShader->SetUniform1i("uTexture", GetTextureSlot(prevTexture));
					}

					UploadRenderableQuad(renderable.get());
				}
				FlushQuadBatch();
			}
		}

		if (!transparentQuads.empty())
		{
			std::vector<std::pair<Shader*, QuadRenderable*>> sortedTransparent;
			for (auto& [shader, renderables] : transparentQuads)
			{
				assert(shader);
				for (auto& renderable : renderables)
				{
					sortedTransparent.emplace_back(shader, renderable.get());
				}
			}

			std::sort(sortedTransparent.begin(), sortedTransparent.end(),
				[&](const auto& a, const auto& b)
				{
					if(a.second->z == b.second->z) return a.second->texture < b.second->texture;
					return a.second->z < b.second->z;
				});

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_FALSE);

			for (auto& [shader, renderable] : sortedTransparent)
			{
				if (shader != currentShader)
				{
					FlushQuadBatch();
					currentShader = shader;
					BindShader(currentShader->GetHandle());
					currentShader->SetUniform1i("uTexture", GetTextureSlot(prevTexture));
				}
				if (prevTexture != renderable->texture)
				{
					FlushQuadBatch();
					prevTexture = renderable->texture;
					BindTexture(prevTexture);
					currentShader->SetUniform1i("uTexture", GetTextureSlot(prevTexture));
				}
				UploadRenderableQuad(renderable);
			}
			FlushQuadBatch();

			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);
		}

		ClearQuadBatchData();
	}

	void Graphics::FlushQuadBatch()
	{
		BindVertexArray(quadVao);
		BindVertexBuffer(quadInstanceVbo);

		glBindBuffer(GL_ARRAY_BUFFER, quadInstanceVbo);
		glBufferData(GL_ARRAY_BUFFER, maxQuadsInBatch * sizeof(QuadInstanceData), nullptr, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, int(quadInstanceDataBuffer.size() * sizeof(QuadInstanceData)), quadInstanceDataBuffer.data());

		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, int(quadInstanceDataBuffer.size()));
		quadInstanceDataBuffer.clear();
	}

	void Graphics::UploadRenderableQuad(QuadRenderable* renderable)
	{
		if (quadInstanceDataBuffer.size() == maxQuadsInBatch)
		{
			FlushQuadBatch();
		}

		const Texture* texture = renderable->texture;

		unsigned int vertStart = unsigned int(quadInstanceDataBuffer.size());
		quadInstanceDataBuffer.emplace_back(QuadInstanceData{ renderable->uv, renderable->color, renderable->model });

		UseTexture(texture);
	}

	Texture* Graphics::LoadTexture(const std::string& filepath, TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter)
	{
		if (!textures.contains(filepath))
		{
			std::unique_ptr<Texture> texture = std::make_unique<Texture>(filepath, wrap, minFilter, magFilter);
			Texture* rawPtr = texture.get();
			textureToSlot[rawPtr] = -1;
			textures[filepath] = std::move(texture);
		}
		return textures[filepath].get();
	}

	Texture* Graphics::CreateTextureFromMemory(int width, int height, int BPP, unsigned char* buffer, TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter)
	{
		std::string name = "__dynamic_" + std::to_string(totalDynamiclyCreatedTextures++);
		std::unique_ptr<Texture> texture = std::make_unique<Texture>(width, height, BPP, buffer, wrap, minFilter, magFilter);
		Texture* rawPtr = texture.get();
		textureToSlot[rawPtr] = -1;
		textures[name] = std::move(texture);
		return textures[name].get();
	}

	Font* Graphics::LoadFont(const std::string& filepath, char firstChar, char lastChar)
	{
		if (!fonts.contains(filepath))
		{
			assert(firstChar <= lastChar);
			int charCount = lastChar - firstChar + 1;
			std::vector<stbtt_bakedchar> charData(charCount);
		
			FILE* file = nullptr;
			errno_t err = fopen_s(&file, filepath.c_str(), "rb");
			assert(err == 0 && file);

			fseek(file, 0, SEEK_END);
			size_t size = ftell(file);
			fseek(file, 0, SEEK_SET);

			std::vector<unsigned char> ttfBuffer(size);
			fread(ttfBuffer.data(), size, 1, file);
			fclose(file);
			stbtt_fontinfo info;
			stbtt_InitFont(&info, ttfBuffer.data(), 0);

			int ascent, descent, lineGap;
			stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
			float scale = stbtt_ScaleForPixelHeight(&info, fontLineHeight);
			float realLineHeight = scale * (ascent - descent + lineGap);

			const int texWidth = 512;
			const int texHeight = 512;
			std::vector<unsigned char> bitmap(texWidth * texHeight, 0);
			stbtt_BakeFontBitmap(ttfBuffer.data(), 0, fontLineHeight, bitmap.data(), texWidth, texHeight, firstChar, charCount, charData.data());
			
			std::vector<Font::Glyph> glyphData;
			glyphData.resize(charCount, {});
			for (int c = int(firstChar); c <= int(lastChar); c++)
			{
				int index = c - firstChar;
				const stbtt_bakedchar& bc = charData[index];
				
				Font::Glyph glyph;
				glyph.rect = RectF(bc.x0, bc.x1, bc.y1, bc.y0);
				glyph.xadvance = bc.xadvance;

				glyphData[index] = glyph;
			}
			std::vector<unsigned char> buffer(texWidth * texHeight * 4);
			for (size_t i = 0; i < size_t(texWidth * texHeight); i++)
			{
				unsigned char a = bitmap[i];
				buffer[i * 4 + 0] = 255;
				buffer[i * 4 + 1] = 255;
				buffer[i * 4 + 2] = 255;
				buffer[i * 4 + 3] = a;
			}
			Texture* atlas = CreateTextureFromMemory(texWidth, texHeight, 4, buffer.data(), TextureWrap::ClampToEdge, TextureFilter::Nearest, TextureFilter::Nearest);
			fonts[filepath] = std::make_unique<Font>(atlas, std::move(glyphData), realLineHeight, ascent, firstChar, lastChar);
		}
		return fonts[filepath].get();
	}

	void Graphics::UnloadTexture(Texture* texture)
	{
		assert(texture && "Failed to unload texture. Texture is nullptr");
		lru.Erase(texture);
		auto texSlotIt = textureToSlot.find(texture);

		int slot = texSlotIt->second;
		if (slot != -1)
		{
			slotToTexture.erase(slot);
			availableSlots.insert(slot);
		}
		textureToSlot.erase(texSlotIt);

		for (auto it = textures.begin(); it != textures.end(); ++it)
		{
			if (it->second.get() == texture)
			{
				textures.erase(it);
				break;
			}
		}
	}

	Mesh* Graphics::LoadMesh(const std::string& filepath)
	{
		if (!meshes.contains(filepath)) 
		{
			unsigned int meshvao;
			unsigned int meshvbo;
			unsigned int meshibo;

			std::vector<MeshVertex> verts
			{
				{Vec3f(-0.5f,  0.5f, 0.0f), Vec2f(0.0f, 1.0f)}, // top-left
				{Vec3f(0.5f,  0.5f, 0.0f), Vec2f(1.0f, 1.0f)}, // top-right
				{Vec3f(0.5f, -0.5f, 0.0f), Vec2f(1.0f, 0.0f)}, // bottom-right
				{Vec3f(-0.5f, -0.5f, 0.0f), Vec2f(0.0f, 0.0f)}  // bottom-left
			};

			std::vector<unsigned int> indices
			{
				0, 1, 2,
				2, 3, 0
			};

			glGenVertexArrays(1, &meshvao);
			BindVertexArray(meshvao);
			
			glGenBuffers(1, &meshvbo);
			BindVertexBuffer(meshvbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex) * verts.size(), verts.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, pos));//pos
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, uv)); //uv
			glEnableVertexAttribArray(1);
			
			glGenBuffers(1, &meshibo);
			BindIndexBuffer(meshibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

			meshes[filepath] = std::make_unique<Mesh>(meshvao, meshvbo, meshibo);
		}
		return meshes[filepath].get();
	}

	void Graphics::UnloadFont(Font* font)
	{
		assert(font && "Failed to unload font. Font is nullptr");
		UnloadTexture(font->GetTextureAtlas());
		for (auto it = fonts.begin(); it != fonts.end(); ++it)
		{
			if (it->second.get() == font)
			{
				fonts.erase(it);
				break;
			}
		}
	}

	Shader* Graphics::LoadShader(const std::string& vertex, const std::string& fragment, bool isPath)
	{
		std::string name = vertex + '|' + fragment;
		if (!shaders.contains(name))
		{
			std::unique_ptr<Shader> shader = std::make_unique<Shader>(vertex, fragment, isPath);
			BindShader(shader->GetHandle());
			GLuint blockIndex = glGetUniformBlockIndex(shader.get()->GetHandle(), "CameraBuffer");
			if(blockIndex != GL_INVALID_INDEX) glUniformBlockBinding(shader.get()->GetHandle(), blockIndex, 0);
			
			Shader* rawPtr = shader.get();
			shaders[name] = std::move(shader);

		}
		return shaders[name].get();
	}

	void Graphics::UnloadShader(Shader* shader)
	{
		assert(shader && "Failed to unload shader. Shader is nullptr");
		for (auto it = shaders.begin(); it != shaders.end(); ++it)
		{
			if (it->second.get() == shader)
			{
				shaders.erase(it);
				break;
			}
		}
	}

	int Graphics::GetTextureSlot(const Texture* texture)
	{
		return textureToSlot[texture];
	}

	void Graphics::BindTexture(const Texture* texture)
	{
		assert(texture && "Failed to bind texture. Texture is nullptr");
		if(textureToSlot[texture] == -1)
		{
			if (availableSlots.empty())
			{
				const Texture* oldTexture = lru.GetLRU();
				assert(oldTexture);
				lru.PopLRU();
				int slot = GetTextureSlot(oldTexture);
				assert(slot != -1);
				textureToSlot[oldTexture] = -1;
				glBindTextureUnit(slot, texture->GetHandle());
				slotToTexture[slot] = texture;
				textureToSlot[texture] = slot;
			}
			else
			{
				int slot = *availableSlots.begin();
				glBindTextureUnit(slot, texture->GetHandle());
				textureToSlot[texture] = slot;
				slotToTexture[slot] = texture;
				availableSlots.erase(slot);
			}
		}
		assert(textureToSlot[texture] >= 0);
		lru.Push(texture);
	}

	void Graphics::UseTexture(const Texture* texture)
	{
		assert(texture && "Failed to use texture. Texture is nullptr");
		if (texture) lru.Push(texture);
	}

	void Graphics::ClearTextures()
	{
		slotToTexture.clear();
		textures.clear();
		availableSlots.clear();
		for (int i = 0; i < maxTextureSlots; i++) availableSlots.insert(i);
	}
	Graphics::QuadRenderable::QuadRenderable(Mat4f model, const Texture* texture, RectF uv, Color color, float z)
		: model(model), uv(uv), texture(texture), color(color), z(z) {}
}