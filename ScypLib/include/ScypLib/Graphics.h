#pragma once
#include<unordered_map>
#include<unordered_set>

#include"Rect.h"
#include"Color.h"
#include"Window.h"
#include"LRU.h"
#include"Sprite.h"
#include"Mesh.h"
#include"Shader.h"
#include"Texture.h"
#include"Font.h"
#include"Matrix.h"
#undef DrawText

namespace sl
{
    enum class DrawMode
    {
        Sprite2d,
        Mesh3d,
        Quad3d
    };
    struct Camera2d
    {
        Vec2f pos;
        float zoom;
    };
    struct Camera3d
    {

    };//TODO
    class Graphics
    {
    private:
        struct ViewProjMat
        {
            alignas(16) Mat4f view;
            alignas(16) Mat4f projection;
        };
        struct QuadInstanceData
        {
            RectF uv;
            Color color;
            Mat4f model;
        };
        struct QuadVertex
        {
            Vec3f pos;

            QuadVertex(Vec3f pos);

            bool operator==(const QuadVertex& other) const;
        };
        struct MeshVertex
        {
            Vec3f pos;
            Vec2f uv;
        };//TODO
        struct QuadRenderable
        {
            QuadRenderable(Mat4f model, const Texture* texture, RectF uv, Color color, float z);
            Mat4f model;
            const Texture* texture = nullptr;
            RectF uv;
            Color color;
            float z;
        };
        struct MeshRenderable
        {
            MeshRenderable(Vec3f pos, const Texture* texture, Mat4f transform, Color color) {};
            Vec3f pos{};
            const Texture* texture;
            Mat4f transform;
            Color color;
        };
    public:
        Graphics(Window* wnd);
        Graphics(Window* wnd, float canvasWidth, float canvasHeight);
        ~Graphics();

        void BeginFrame();
        void EndFrame(Shader* shader = nullptr);
        void EndFrame(std::vector<Shader*>& shaders);
        void BeginView(Vec2f cameraPosition = { 0.0f, 0.0f }, float zoom = 1.0f);
        //void BeginView(Vec3f cameraPosition = { 0.0f, 0.0f, 0.0f }, Vec3f rotation);//TODO
        void EndView(std::vector<Shader*>& shaders);
        void EndView(Shader* shader = nullptr);
        void SetDrawDepth(float depth);
        void SetCanvasSize(Vec2f size);
        void SetCanvasWidth(float width);
        void SetCanvasHeight(float height);
        void SetVSyncInterval(int interval);
        void ApplyPostProcessing(std::vector<Shader*>& shaders);
        void SetDefaultFont(Font* font);;
        void SetDefaultShader(Shader* shader);

        Texture* LoadTexture(const std::string& filepath, TextureWrap wrap = TextureWrap::ClampToEdge, TextureFilter minFilter = TextureFilter::Nearest, TextureFilter magFilter = TextureFilter::Nearest);
        Texture* CreateTextureFromMemory(int width, int height, int BPP, unsigned char* buffer, TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter);
        Mesh* LoadMesh(const std::string& filepath);
        Font* LoadFont(const std::string& filepath, char firstChar, char lastChar);
        void UnloadTexture(Texture* texture);
        void UnloadFont(Font* font);
        Shader* LoadShader(const std::string& vertex, const std::string& fragment, bool isPath);
        void UnloadShader(Shader* shader);

        void DrawTexture(float x, float y, const Texture* texture);
        void DrawTexture(Vec2f pos, Vec2f size, const Texture* texture, Shader* shader = nullptr, bool flipX = false, bool flipY = false, float angle = 0.0f, Vec2f* pivot = nullptr, const RectF* pixelUV = nullptr, const Color& tint = Colors::White);
        void DrawTexture(const RectF& targetRect, const Texture* texture, Shader* shader = nullptr, bool flipX = false, bool flipY = false, float angle = 0.0f, Vec2f* pivot = nullptr, const RectF* pixelUV = nullptr, const Color& tint = Colors::White);
        void DrawSprite(const Sprite& sprite);
        void DrawAnimatedSprite(const AnimatedSprite& animatedSprite);
        void DrawLine(float x1, float y1, float x2, float y2, float thickness, const Color& c, Shader* shader = nullptr);
        void DrawRect(const RectF& rect, const Color& c);
        void DrawRect(Vec2f pos, Vec2f size, const Color& c);
        void DrawRect(Vec2f pos, Vec2f size, const Color& c, float angle, Shader* shader = nullptr);
        void DrawRect(const RectF& rect, const Color& c, float angle, Shader* shader = nullptr);
        void DrawText(Vec2f pos, const std::string& text, Font* font, float height, const Color& c);
        void DrawText(const RectF& rect, const std::string& text, Font* font, const Color& c);
        void DrawText(Vec2f pos, Vec2f size, const std::string& text, Font* font, const Color& c);
        void PutPixel(float x, float y, const Color& c);

        Color GetPixel(int x, int y);
        RectF GetCanvasRect()const;
        float GetCanvasWidth()const;
        float GetCanvasHeight()const;
        Vec2f GetCanvasSize()const;
    public:
        void BindShader(unsigned int shader);
        void BindShaderStorageBuffer(unsigned int ssbo);
        void BindUniformBuffer(unsigned int ubo);
        void BindVertexArray(unsigned int vao);
        void BindIndexBuffer(unsigned int ibo);
        void BindVertexBuffer(unsigned int vbo);
    private:
        void UpdateCanvasSize(float width, float height);
        void ClearQuadBatchData();
        void RenderQuads();
        void FlushQuadBatch();
        bool IsQuadOffscreen(const RectF& rect) const;
        void UploadRenderableQuad(QuadRenderable* renderable);
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
        //framebuffer and camera
        Camera2d cam2d = { Vec2f(0.0f,0.0f),1.0f };
        unsigned int fbo;
        unsigned int rbo;
        Texture* framebufferTexture = nullptr;
        Texture* framebufferTextureSecondary = nullptr;
        //others
        DrawMode drawMode = DrawMode::Sprite2d;
        float curDrawDepth = 0;
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
        size_t maxQuadsInBatch = 10000;
        // renderables containers
        //2d
        unsigned int quadVao= 0;
        unsigned int quadVbo = 0;
        unsigned int quadInstanceVbo = 0;
        std::vector<QuadInstanceData> quadInstanceDataBuffer;
        std::unordered_map<Shader*, std::vector<std::unique_ptr<QuadRenderable>>> opaqueQuads;
        std::unordered_map<Shader*, std::vector<std::unique_ptr<QuadRenderable>>> transparentQuads;
        //3d
        std::unordered_map<Shader*, std::vector<std::unique_ptr<MeshRenderable>>> opaqueMeshes;
        std::unordered_map<Shader*, std::vector<std::unique_ptr<MeshRenderable>>> transparentMeshes;
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
        //meshes
        std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
    };
}