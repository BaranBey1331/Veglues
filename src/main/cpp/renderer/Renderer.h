#ifndef MOBILEGLUES_RENDERER_H
#define MOBILEGLUES_RENDERER_H

#include "StateManager.h"
#include "ShaderCache.h"
#include "Batcher.h"
#include <mutex>

class Renderer {
public:
    static Renderer& GetInstance();

    void Init();
    void Shutdown();

    // The main draw call from Java/Engine
    // Assumes standard vertex format for this optimization pass
    void DrawGeometry(const void* vertices, int vertexSizeBytes, int vertexCount,
                      const void* indices, int indexCount,
                      GLuint textureId, GLuint programId);

    // State setters that trigger batch flushes
    void SetBlendFunc(GLenum sfactor, GLenum dfactor);
    void SetDepthMask(GLboolean flag);
    void Enable(GLenum cap);
    void Disable(GLenum cap);

    // Frame lifecycle
    void BeginFrame();
    void EndFrame();

    StateManager& GetStateManager() { return stateManager; }
    ShaderCache& GetShaderCache() { return shaderCache; }

private:
    Renderer();
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    StateManager stateManager;
    ShaderCache shaderCache;
    Batcher batcher;

    GLuint currentProgramId;
    GLuint currentTextureId;

    // Thread safety if needed (though rendering is usually single threaded)
    std::mutex renderMutex;
};

#endif // MOBILEGLUES_RENDERER_H
