#pragma once
#include<unordered_map>
#include<unordered_set>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include"Rect.h"
#include"Color.h"
#include"Window.h"
#include"LRU.h"
#include"Sprite.h"
#include"Shader.h"
#include"Texture.h"
#include"Font.h"
#undef DrawText

namespace sl
{
    class Graphics
    {
    private:
        struct ViewProjMat
        {
            alignas(16) glm::mat4 view;
            alignas(16) glm::mat4 projection;
        };
        struct TextureVertex
        {
            float x, y, z;
            float u, v;
            float instanceIndex;

            TextureVertex(float x, float y, float z, float u, float v, int instanceIndex);

            bool operator==(const TextureVertex& other) const;

            struct Hasher
            {
                size_t operator()(const TextureVertex& vertex) const;
            };
        };
        struct InstanceData
        {
        public:
            InstanceData(glm::mat4 transform, Color color, float textureSlot);
        public:
            alignas(16) glm::mat4 transform;
            alignas(16) Color color;
            alignas(16) float textureSlot;
        };
        struct Renderable
        {
        public:
            Renderable(float x, float y, float z, float width, float height, RectF uv, const Texture* texture, glm::mat4 transform, Color color);
        public:
            float x, y, z = 0;
            float width, height;
            const Texture* texture;
            InstanceData data;
            RectF uv;
        };
    public:
        Graphics(Window* wnd);
        Graphics(Window* wnd, float canvasWidth, float canvasHeight);
        ~Graphics();

        void BeginFrame();
        void EndFrame(Shader* shader = nullptr);
        void EndFrame(std::vector<Shader*>& shaders);
        void BeginView(Vec2f cameraPosition = { 0.0f, 0.0f }, float zoom = 1.0f);
        void EndView(std::vector<Shader*>& shaders);
        void EndView(Shader* shader = nullptr);
        void SetDrawLayer(float layer);
        void SetCanvasSize(Vec2f size);
        void SetCanvasWidth(float width);
        void SetCanvasHeight(float height);
        void SetVSyncInterval(int interval);
        void ApplyPostProcessing(std::vector<Shader*>& shaders);
        void SetDefaultFont(Font* font);;
        void SetDefaultShader(Shader* shader);

        Texture* LoadTexture(const std::string& filepath, TextureWrap wrap = TextureWrap::ClampToEdge, TextureFilter minFilter = TextureFilter::Nearest, TextureFilter magFilter = TextureFilter::Nearest);
        Texture* CreateTextureFromMemory(int width, int height, int BPP, unsigned char* buffer, TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter);
        Font* LoadFont(const std::string& filepath, char firstChar, char lastChar);
        void UnloadTexture(Texture* texture);
        void UnloadFont(Font* font);
        Shader* LoadShader(const std::string& vertex, const std::string& fragment, bool isPath);
        void UnloadShader(Shader* shader);

        void DrawTexture(float x, float y, const Texture* texture);
        void DrawTexture(Vec2f pos, Vec2f size, const Texture* texture, Shader* shader = nullptr, bool flipX = false, bool flipY = false, float angle = 0.0f, Vec2f origin = Vec2f(0.0f, 0.0f), const RectF* uv = nullptr, const Color& tint = Colors::White);
        void DrawTexture(const RectF& targetRect, const Texture* texture, Shader* shader = nullptr, bool flipX = false, bool flipY = false, float angle = 0.0f, Vec2f origin = Vec2f(0.0f, 0.0f), const RectF* uv = nullptr, const Color& tint = Colors::White);
        void DrawSprite(const Sprite& sprite);
        void DrawAnimatedSprite(const AnimatedSprite& animatedSprite);
        void DrawLine(float x1, float y1, float x2, float y2, float thickness, const Color& c, Shader* shader = nullptr);
        void DrawRect(const RectF& rect, const Color& c);
        void DrawRect(Vec2f pos, Vec2f size, const Color& c);
        void DrawRect(Vec2f pos, Vec2f size, const Color& c, float angle, Shader* shader = nullptr);
        void DrawRect(const RectF& rect, const Color& c, float angle, Shader* shader = nullptr);
        void DrawText(float x, float y, const std::string& text, Font* font, float height, const Color& c);
        void PutPixel(float x, float y, const Color& c);

        Color GetPixel(int x, int y);
        RectF GetCanvasRect()const;
        float GetCanvasWidth()const;
        float GetCanvasHeight()const;
    public:
        void BindShader(unsigned int shader);
        void BindShaderStorageBuffer(unsigned int ssbo);
        void BindUniformBuffer(unsigned int ubo);
        void BindVertexArray(unsigned int vao);
        void BindIndexBuffer(unsigned int ibo);
        void BindVertexBuffer(unsigned int vbo);
    private:
        void UpdateCanvasSize(float width, float height);
        void ClearBatchData();
        void Render();
        void FlushBatch();
        void UploadRenderable(Renderable* renderable);
        int GetTextureSlot(const Texture* texture);
        const int GetTextureSlotLimit() const { return maxTextureSlots; };
        void BindTexture(const Texture* texture);
        void UseTexture(const Texture* texture);
        void ClearTextures();
    private:
        //window and canvasdata
        Window* window = nullptr;
        float canvasWidth = -1.0f;
        float canvasHeight = -1.0f;
        //framebuffer
        unsigned int fbo;
        unsigned int rbo;
        Texture* framebufferTexture = nullptr;
        Texture* framebufferTextureSecondary = nullptr;
        //others
        float curDrawLayer = 0;
        Texture* blankTexture = nullptr;
        unsigned int vpMatUbo = 0;
        unsigned int vpMatUboBindingPoint = 0;
        int totalDynamiclyCreatedTextures = 0;
        float fontLineHeight = 32;
        Shader* builtInShader = nullptr;
        Shader* defaultShader = nullptr;
        Font* defaultFont = nullptr;
        ViewProjMat vpMat{};
        //opengl current binds
        unsigned int boundVAO = 0;
        unsigned int boundIBO = 0;
        unsigned int boundVBO = 0;
        unsigned int boundShader = 0;
        unsigned int boundSSBO = 0;
        unsigned int boundUBO = 0;
        //batch components
        Shader* currentShader = nullptr;
        unsigned int vao = 0;
        unsigned int vbo = 0;
        unsigned int ibo = 0;
        unsigned int instanceSSBO = 0;
        unsigned int instanceSSBOBindingPoint = 1;
        std::vector<unsigned int> indices;
        std::vector<TextureVertex> vertices;
        size_t maxQuadsInBatch = 10000;
        std::vector<InstanceData> instanceDataBuffer;
        std::unordered_set<const Texture*> usedTextures;
        // renderables containers
        std::unordered_map<Shader*, std::vector<std::unique_ptr<Renderable>>> opaque;
        std::unordered_map<Shader*, std::vector<std::unique_ptr<Renderable>>> transparent;
        //texture manager
        int maxTextureSlots = 0;
        LRU<const Texture*> lru;
        std::unordered_map<std::string, std::unique_ptr<Texture>> textures;
        std::unordered_map<const Texture*, int> textureToSlot;
        std::unordered_map<int, const Texture*> slotToTexture;
        std::unordered_set<int> availableSlots;
        //fonts
        std::unordered_map<std::string, std::unique_ptr<Font>> fonts;
        //shaders
        std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
    };
}