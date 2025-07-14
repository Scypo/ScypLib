#include<glm/glm.hpp>
#include<glm/gtc/type_ptr.hpp>

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
			#version 450 core
			
			layout(location = 0) in vec3 aPosition;
			layout(location = 1) in vec2 aTexCoord;
			layout(location = 2) in float aInstanceIndex;
			
			struct InstanceData 
			{
			    mat4 transform;
			    vec4 colorTint;
			    float textureSlot;
			    float padding[3];
			};
			
			layout(std140, binding = 0) uniform CameraBuffer 
			{
			    mat4 view;
			    mat4 projection;
			};
			
			layout(std430, binding = 1) readonly buffer instanceData
			{
			    InstanceData instances[];
			};
			
			out vec2 vTexCoord;
			out float vTexSlot;
			out vec4 vColorTint;
			
			void main()
			{
			    InstanceData data = instances[int(aInstanceIndex)];
			    gl_Position = projection * view * data.transform * vec4(aPosition, 1.0);
			
			    vTexCoord = aTexCoord;
			    vTexSlot = data.textureSlot;
			    vColorTint = data.colorTint;
			}
			)";
		const std::string fragmentShader = R"(
			#version 450 core
			
			in vec2 vTexCoord;
			in float vTexSlot;
			in vec4 vColorTint;
			
			out vec4 FragColor;
			uniform sampler2D uTextures[32];
			
			void main()
			{
			    int slot = int(vTexSlot);
			    vec4 texColor = texture(uTextures[slot], vTexCoord);
			    vec4 finalColor = texColor * vColorTint;
			
			    if (finalColor.a < 0.1) discard;
			
			    FragColor = finalColor;
			}
			)";

		builtInShader = LoadShader(vertexShader, fragmentShader, false);
		SetDefaultShader(builtInShader);

		glGenVertexArrays(1, &vao);
		BindVertexArray(vao);
		glGenBuffers(1, &vbo);
		BindVertexBuffer(vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(TextureVertex) * unsigned int(maxQuadsInBatch * 4), nullptr, GL_DYNAMIC_DRAW);
		unsigned int offset = 0;
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), (void*)offset);
		glEnableVertexAttribArray(0);
		offset += 3 * sizeof(float);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), (void*)offset);
		glEnableVertexAttribArray(1);
		offset += 2 * sizeof(float);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), (void*)offset);
		glEnableVertexAttribArray(2);
		glGenBuffers(1, &ibo);
		BindIndexBuffer(ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * unsigned int(6 * maxQuadsInBatch), nullptr, GL_DYNAMIC_DRAW);

		glGenBuffers(1, &instanceSSBO);
		BindShaderStorageBuffer(instanceSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(InstanceData) * unsigned int(maxQuadsInBatch), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, instanceSSBOBindingPoint, instanceSSBO);
		glGenBuffers(1, &vpMatUbo);
		BindUniformBuffer(vpMatUbo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(ViewProjMat), &vpMat, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, vpMatUboBindingPoint, vpMatUbo);

		SetCanvasSize(Vec2f(1.0f, 1.0f));//SMTHING BUGGER IF CALLED TWICE IT WORKS PROPERLY OR OUTSIDE OF CONSTRUCTOR
		SetCanvasSize(Vec2f(float(wnd->GetWidth()), float(wnd->GetHeight())));

		vertices.reserve(maxQuadsInBatch * 4);
		indices.reserve(6 * maxQuadsInBatch);
	}

	Graphics::TextureVertex::TextureVertex(float x, float y, float z, float u, float v, int instanceIndex)
		: x(x), y(y), z(z), u(u), v(v), instanceIndex(static_cast<float>(instanceIndex)) {}

	bool Graphics::TextureVertex::operator==(const TextureVertex& other) const
	{
		return x == other.x && y == other.y &&
			z == other.z &&
			u == other.u && v == other.v &&
			instanceIndex == other.instanceIndex;
	}

	size_t Graphics::TextureVertex::Hasher::operator()(const TextureVertex& vertex) const
	{
		size_t seed = 0;
		auto hash_combine = [&seed](size_t h)
			{
				seed ^= h + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			};

		hash_combine(std::hash<float>{}(vertex.x));
		hash_combine(std::hash<float>{}(vertex.y));
		hash_combine(std::hash<float>{}(vertex.z));
		hash_combine(std::hash<float>{}(vertex.u));
		hash_combine(std::hash<float>{}(vertex.v));
		hash_combine(std::hash<float>{}(vertex.instanceIndex));

		return seed;
	}

	Graphics::InstanceData::InstanceData(glm::mat4 transform, Color color, float textureSlot)
		: transform(transform), color(color), textureSlot(textureSlot) {}

	Graphics::Renderable::Renderable(float x, float y, float z, float width, float height, RectF uv, const Texture* texture, glm::mat4 transform, Color color)
		: x(x), y(y), z(z), width(width), height(height), uv(uv), texture(texture), data(transform, color, -1.0f) {}

	Graphics::~Graphics()
	{
		glDeleteFramebuffers(1, &fbo);
		glDeleteRenderbuffers(1, &rbo);
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ibo);
		glDeleteBuffers(1, &instanceSSBO);
		glDeleteBuffers(1, &vpMatUbo);
		ClearTextures();
	}

	void Graphics::BeginFrame()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, canvasWidth, canvasHeight);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Graphics::EndFrame(Shader* shader)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, window->GetWidth(), window->GetHeight());
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		if (!shader) shader = defaultShader;
		DrawTexture(GetCanvasRect(), framebufferTexture, shader);
		Render();
		glfwSwapBuffers(window->GetGLFWWindow());
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
		Render();
		glfwSwapBuffers(window->GetGLFWWindow());
		glEnable(GL_DEPTH_TEST);
	}

	void Graphics::BeginView(Vec2f cameraPosition, float zoom)
	{
		vpMat.view = glm::mat4(1.0f);
		vpMat.view = glm::scale(vpMat.view, glm::vec3(zoom, zoom, 1.0f));
		vpMat.view = glm::translate(vpMat.view, glm::vec3(-cameraPosition.x, -cameraPosition.y, 0.0f));
		BindUniformBuffer(vpMatUbo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(vpMat), &vpMat);
	}

	void Graphics::EndView(std::vector<Shader*>& shaders)
	{
		Render();
		ApplyPostProcessing(shaders);
	}

	void Graphics::EndView(Shader* shader)
	{
		Render();
		glDisable(GL_DEPTH_TEST);
		if (!shader) shader = defaultShader;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTextureSecondary->GetHandle(), 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture->GetHandle(), 0);
		DrawTexture(GetCanvasRect(), framebufferTexture, shader);
		glEnable(GL_DEPTH_TEST);
	}

	void Graphics::SetDrawLayer(float layer)
	{
		curDrawLayer = layer;
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
			Render();
			std::swap(currentTarget, otherTarget);
		}
		if (currentTarget != framebufferTexture) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture->GetHandle(), 0);
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
		assert(texture && "Failed to draw texture. Texture is nullptr");
		auto renderable = std::make_unique<Renderable>(x, y, curDrawLayer, float(texture->GetWidth()), float(texture->GetHeight()), RectF(0.0f, 1.0f, 0.0f, 1.0f),
			texture, glm::mat4(1.0f), Colors::White);
		if (texture->IsBinaryAlpha()) opaque[defaultShader].emplace_back(std::move(renderable));
		else transparent[defaultShader].emplace_back(std::move(renderable));
	}

	void Graphics::DrawTexture(Vec2f pos, Vec2f size, const Texture* texture, Shader* shader, bool flipX, bool flipY, float angle, Vec2f origin, const RectF* uv, const Color& tint)
	{
		assert(texture && "Failed to draw texture. Texture is nullptr");
		RectF finalUV(0.0f, 1.0f, 0.0f, 1.0f);
		glm::mat4 transform(1.0f);
		if (!shader) shader = defaultShader;
		if (uv) finalUV = *uv / Vec2f(float(texture->GetWidth()), float(texture->GetHeight()));
		if (flipX) std::swap(finalUV.left, finalUV.right);
		if (flipY) std::swap(finalUV.top, finalUV.bottom);
		if (angle != 0)
		{
			transform = glm::translate(transform, glm::vec3(origin.x + pos.x, origin.y + pos.y, 0.0f));
			transform = glm::rotate(transform, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
			transform = glm::translate(transform, glm::vec3(-origin.x - pos.x, -origin.y - pos.y, 0.0f));
		}
		auto renderable = std::make_unique<Renderable>(pos.x, pos.y, curDrawLayer, size.x, size.y, finalUV, texture, transform, tint);
		if (texture->IsBinaryAlpha() && (tint.a == 1.0f || tint.a == 0.0f)) opaque[shader].emplace_back(std::move(renderable));
		else transparent[shader].emplace_back(std::move(renderable));
	}

	void Graphics::DrawTexture(const RectF& targetRect, const Texture* texture, Shader* shader, bool flipX, bool flipY, float angle, Vec2f origin, const RectF* uv, const Color& tint)
	{
		DrawTexture({ targetRect.left, targetRect.top }, { targetRect.GetWidth(), targetRect.GetHeight() }, texture, shader, flipX, flipY, angle, origin, uv, tint);
	}

	void Graphics::DrawSprite(const Sprite& sprite)
	{
		assert(sprite.GetTexture() && "Failed to draw sprite. Texture is nullptr");
		glm::mat4 transform(1.0f);
		Vec2f pos = sprite.GetPos();
		Vec2f size = sprite.GetSize();
		Shader* shader = sprite.GetShader();
		if (!shader) shader = defaultShader;
		if (sprite.GetRotation() != 0)
		{
			Vec2f origin = sprite.GetOrigin();

			transform = glm::translate(transform, glm::vec3(origin.x + pos.x, origin.y + pos.y, 0.0f));
			transform = glm::rotate(transform, glm::radians(sprite.GetRotation()), glm::vec3(0.0f, 0.0f, 1.0f));
			transform = glm::translate(transform, glm::vec3(-origin.x - pos.x, -origin.y - pos.y, 0.0f));
		}
		auto renderable = std::make_unique<Renderable>(pos.x, pos.y, curDrawLayer, size.x, size.y, sprite.GetNDCUV(), sprite.GetTexture(), transform, sprite.GetColorTint());

		if (sprite.GetTexture()->IsBinaryAlpha() && (sprite.GetColorTint().a == 1.0f || sprite.GetColorTint().a == 0.0f)) opaque[shader].emplace_back(std::move(renderable));
		else transparent[shader].emplace_back(std::move(renderable));
	}

	void Graphics::DrawAnimatedSprite(const AnimatedSprite& animatedSprite)
	{
		assert(animatedSprite.GetTexture() && "Failed to draw sprite. Texture is nullptr");
		glm::mat4 transform(1.0f);
		Vec2f pos = animatedSprite.GetPos();
		Vec2f size = animatedSprite.GetSize();
		RectF uv = animatedSprite.GetNDCUV();
		Shader* shader = animatedSprite.GetShader();
		if (!shader) shader = defaultShader;
		if (animatedSprite.IsFlippedX()) std::swap(uv.left, uv.right);
		if (animatedSprite.IsFlippedY()) std::swap(uv.top, uv.bottom);

		if (animatedSprite.GetRotation() != 0)
		{
			Vec2f origin = animatedSprite.GetOrigin();

			transform = glm::translate(transform, glm::vec3(origin.x + pos.x, origin.y + pos.y, 0.0f));
			transform = glm::rotate(transform, glm::radians(animatedSprite.GetRotation()), glm::vec3(0.0f, 0.0f, 1.0f));
			transform = glm::translate(transform, glm::vec3(-origin.x - pos.x, -origin.y - pos.y, 0.0f));
		}
		auto renderable = std::make_unique<Renderable>(pos.x, pos.y, curDrawLayer, size.x, size.y, animatedSprite.GetNDCUV(), animatedSprite.GetTexture(), transform, animatedSprite.GetColorTint());

		if (animatedSprite.GetTexture()->IsBinaryAlpha() && (animatedSprite.GetColorTint().a == 1.0f || animatedSprite.GetColorTint().a == 0.0f)) opaque[shader].emplace_back(std::move(renderable));
		else transparent[shader].emplace_back(std::move(renderable));
	}

	void Graphics::DrawLine(float x1, float y1, float x2, float y2, float thickness, const Color& c, Shader* shader)
	{
		float dx = x2 - x1;
		float dy = y2 - y1;
		float length = std::sqrt(dx * dx + dy * dy);
		float angle = std::atan2(dy, dx);
		if (!shader) shader = defaultShader;

		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::rotate(transform, angle, glm::vec3(0.0f, 0.0f, 1.0f));

		auto renderable = std::make_unique<Renderable>(x1, y1 - thickness / 2.0f, curDrawLayer, length, thickness, RectF(0.0f, 1.0f, 0.0f, 1.0f), blankTexture, transform, c);

		if (c.a == 0.0f || c.a == 1.0f) opaque[shader].emplace_back(std::move(renderable));
		else transparent[shader].emplace_back(std::move(renderable));
	}

	void Graphics::DrawRect(const RectF& rect, const Color& c)
	{
		glm::mat4 transform(1.0f);
		auto renderable = std::make_unique<Renderable>(rect.left, rect.top, curDrawLayer, float(rect.GetWidth()), float(rect.GetHeight()),
			RectF(0.0f, 1.0f, 0.0f, 1.0f), blankTexture, transform, c);
		if (c.a == 0.0f || c.a == 1.0f) opaque[defaultShader].emplace_back(std::move(renderable));
		else transparent[defaultShader].emplace_back(std::move(renderable));
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
		glm::mat4 transform(1.0f);
		if (!shader) shader = defaultShader;

		Vec2f center = rect.GetCenter();
		transform = glm::translate(transform, glm::vec3(center.x, center.y, 0.0f));
		transform = glm::rotate(transform, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::translate(transform, glm::vec3(-center.x, -center.y, 0.0f));

		auto renderable = std::make_unique<Renderable>(rect.left, rect.top, curDrawLayer, float(rect.GetWidth()), float(rect.GetHeight()),
			RectF(0.0f, 1.0f, 0.0f, 1.0f), blankTexture, transform, c);
		if (c.a == 0.0f || c.a == 1.0f) opaque[shader].emplace_back(std::move(renderable));
		else transparent[shader].emplace_back(std::move(renderable));
	}

	void Graphics::DrawText(float x, float y, const std::string& text, Font* font, float height, const Color& c)
	{
		if (!font) font = defaultFont;
		assert(font->GetTextureAtlas() && "Failed to draw text. Font atlas is nullptr");
		assert(font && "Failed to draw text. Both font and default font are nullptrs");

		const std::vector<stbtt_bakedchar>& charData = font->GetCharData();
		Texture* atlas = font->GetTextureAtlas();
		float baseLineHeight = font->GetLineHeight();

		float scale = height / baseLineHeight;
		float xCursor = x;
		float yCursor = y + scale * float(font->GetAscent() / font->GetLineHeight()) - height * scale;

		for (char ch : text)
		{
			if (ch < font->GetFirstChar() || ch >= font->GetLastChar()) continue;
			stbtt_aligned_quad quad;
			stbtt_GetBakedQuad(charData.data(), atlas->GetWidth(), atlas->GetHeight(), ch - font->GetFirstChar(), &xCursor, &yCursor, &quad, 1);
			float quadWidth = (quad.x1 - quad.x0) * scale;
			float quadHeight = (quad.y1 - quad.y0) * scale;
			Vec2f drawPos = Vec2f(quad.x0 * scale, quad.y0 * scale);
			RectF uv(quad.s0 * atlas->GetWidth(), quad.s1 * atlas->GetWidth(), quad.t1 * atlas->GetHeight(), quad.t0 * atlas->GetHeight());
			DrawTexture(drawPos, Vec2f(quadWidth, quadHeight), atlas, nullptr, false, false, 0.0f, Vec2f(0, 0), &uv, c);
		}
	}

	void Graphics::PutPixel(float x, float y, const Color& c)
	{
		auto renderable = std::make_unique<Renderable>(x, y, curDrawLayer, 1.0f, 1.0f, RectF(0.0f, 1.0f, 0.0f, 1.0f), blankTexture, glm::mat4(1.0f), c);
		if (c.a == 1.0f) opaque[defaultShader].emplace_back(std::move(renderable));
		else transparent[defaultShader].emplace_back(std::move(renderable));
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

	void Graphics::UpdateCanvasSize(float width, float height)
	{
		if (width != canvasWidth || height != canvasHeight)
		{
			canvasWidth = width;
			canvasHeight = height;
			vpMat.projection = glm::ortho(0.0f, canvasWidth, canvasHeight, 0.0f, -50.0f, 50.0f);
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

	void Graphics::ClearBatchData()
	{
		vertices.clear();
		indices.clear();
		opaque.clear();
		transparent.clear();
		usedTextures.clear();
		instanceDataBuffer.clear();
	}

	void Graphics::Render()
	{
		if (!opaque.empty())
		{
			for (auto& [shader, renderables] : opaque)
			{
				currentShader = shader;
				for (auto& renderable : renderables)
				{
					UploadRenderable(renderable.get());
				}
				FlushBatch();
			}
		}

		if (!transparent.empty())
		{
			std::vector<std::pair<Shader*, Renderable*>> sortedTransparent;
			for (auto& [shader, renderables] : transparent)
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
					return a.second->z < b.second->z;
				});

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_FALSE);

			for (auto& [shader, renderable] : sortedTransparent)
			{
				if (shader != currentShader)
				{
					assert(shader);
					FlushBatch();
					currentShader = shader;
				}
				UploadRenderable(renderable);
			}
			FlushBatch();

			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);
		}

		ClearBatchData();
	}

	void Graphics::FlushBatch()
	{
		BindShaderStorageBuffer(instanceSSBO);
		BindVertexArray(vao);
		BindVertexBuffer(vbo);
		BindIndexBuffer(ibo);
		BindShader(currentShader->GetHandle());

		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(InstanceData) * int(instanceDataBuffer.size()), instanceDataBuffer.data());
		glBufferSubData(GL_ARRAY_BUFFER, 0, int(vertices.size() * sizeof(TextureVertex)), vertices.data());
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * int(indices.size()), indices.data());

		glDrawElements(GL_TRIANGLES, int(indices.size()), GL_UNSIGNED_INT, nullptr);
		vertices.clear();
		indices.clear();
		usedTextures.clear();
		instanceDataBuffer.clear();
	}

	void Graphics::UploadRenderable(Renderable* renderable)
	{
		if (vertices.size() == maxQuadsInBatch * 4)
		{
			FlushBatch();
		}

		const Texture* texture = renderable->texture;

		int slot = GetTextureSlot(texture);
		if (usedTextures.size() == maxTextureSlots && slot == -1)
		{
			FlushBatch();

			BindTexture(texture);
			slot = GetTextureSlot(texture);
		}
		else if (slot == -1)
		{
			BindTexture(texture);
			slot = GetTextureSlot(texture);
		}
		assert(slot != -1);
		float width = renderable->width;
		float height = renderable->height;
		int instanceIdx = int(instanceDataBuffer.size());

		unsigned int vertStart = unsigned int(vertices.size());

		RectF uv = renderable->uv;

		float x = renderable->x;
		float y = renderable->y;
		float z = renderable->z;
		vertices.emplace_back(TextureVertex(x, y, z, uv.left, uv.bottom, instanceIdx)); // bottom-left
		vertices.emplace_back(TextureVertex(x + width, y, z, uv.right, uv.bottom, instanceIdx)); // bottom-right
		vertices.emplace_back(TextureVertex(x + width, y + height, z, uv.right, uv.top, instanceIdx)); // top-right
		vertices.emplace_back(TextureVertex(x, y + height, z, uv.left, uv.top, instanceIdx)); // top-left

		indices.emplace_back(vertStart);
		indices.emplace_back(vertStart + 1);
		indices.emplace_back(vertStart + 2);

		indices.emplace_back(vertStart);
		indices.emplace_back(vertStart + 2);
		indices.emplace_back(vertStart + 3);

		UseTexture(texture);
		renderable->data.textureSlot = float(slot);

		instanceDataBuffer.push_back(renderable->data);
		if (!usedTextures.contains(texture)) usedTextures.insert(texture);
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
			charData.resize(charCount, {});
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
			std::vector<unsigned char> buffer(texWidth * texHeight * 4);
			for (size_t i = 0; i < size_t(texWidth * texHeight); i++)
			{
				unsigned char a = bitmap[i];
				buffer[i * 4 + 0] = 255;
				buffer[i * 4 + 1] = 255;
				buffer[i * 4 + 2] = 255;
				buffer[i * 4 + 3] = a;
			}
			Texture* atlas = CreateTextureFromMemory(texWidth, texHeight, 4, buffer.data(), TextureWrap::ClampToEdge, TextureFilter::LinearMipmapLinear, TextureFilter::Linear);
			fonts[filepath] = std::make_unique<Font>(atlas, std::move(charData), realLineHeight, ascent, firstChar, lastChar);
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
			int slots[32]{};
			for (int i = 0; i < 32; i++) slots[i] = i;
			BindShader(shader->GetHandle());
			shader->SetUniform1iv("uTextures", 32, slots);
			
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
}