#ifndef MOBILEGLUES_RENDERER_H
#define MOBILEGLUES_RENDERER_H

#include "StateManager.h"
#include "ShaderCache.h"
#include "Batcher.h"
#include <mutex>
#include <vector>
#include <cmath>

struct FrustumPlane {
    float a, b, c, d;

    void Normalize() {
        float len = sqrtf(a*a + b*b + c*c);
        if (len > 0) {
            float invLen = 1.0f / len;
            a *= invLen; b *= invLen; c *= invLen; d *= invLen;
        }
    }

    float Distance(float x, float y, float z) const {
        return a*x + b*y + c*z + d;
    }
};

class Renderer {
public:
    static Renderer& GetInstance();

    void Init();
    void Shutdown();

    // The main draw call from Java/Engine
    void DrawGeometry(const void* vertices, int vertexSizeBytes, int vertexCount,
                      const void* indices, int indexCount,
                      GLuint textureId, GLuint programId,
                      GLenum drawMode,
                      float minX, float minY, float minZ,
                      float maxX, float maxY, float maxZ);

    void SetViewProj(const float* mat);

    void SetBlendFunc(GLenum sfactor, GLenum dfactor);
    void SetDepthMask(GLboolean flag);
    void Enable(GLenum cap);
    void Disable(GLenum cap);

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

    std::mutex renderMutex;

    // Culling
    FrustumPlane frustumPlanes[6];
    bool cullingEnabled;

    void UpdateFrustum(const float* vp);
    bool IsVisible(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
};

#endif // MOBILEGLUES_RENDERER_H
