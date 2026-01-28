#include "Renderer.h"
#include <limits>
#include <cmath>

#define INVALID_GL_UINT 0xFFFFFFFF

Renderer::Renderer() : currentProgramId(INVALID_GL_UINT), currentTextureId(INVALID_GL_UINT), cullingEnabled(false) {
}

Renderer::~Renderer() {
    Shutdown();
}

Renderer& Renderer::GetInstance() {
    static Renderer instance;
    return instance;
}

void Renderer::Init() {
    stateManager.Reset();
    shaderCache.Clear();
}

void Renderer::Shutdown() {
    batcher.Flush();
    shaderCache.Clear();
}

void Renderer::BeginFrame() {
    batcher.NextFrame();
}

void Renderer::EndFrame() {
    batcher.Flush();
}

void Renderer::SetViewProj(const float* mat) {
    UpdateFrustum(mat);
    cullingEnabled = true;
}

void Renderer::UpdateFrustum(const float* m) {
    // Extract planes from ViewProj matrix (Column Major)
    // Left
    frustumPlanes[0].a = m[3] + m[0];
    frustumPlanes[0].b = m[7] + m[4];
    frustumPlanes[0].c = m[11] + m[8];
    frustumPlanes[0].d = m[15] + m[12];
    frustumPlanes[0].Normalize();

    // Right
    frustumPlanes[1].a = m[3] - m[0];
    frustumPlanes[1].b = m[7] - m[4];
    frustumPlanes[1].c = m[11] - m[8];
    frustumPlanes[1].d = m[15] - m[12];
    frustumPlanes[1].Normalize();

    // Bottom
    frustumPlanes[2].a = m[3] + m[1];
    frustumPlanes[2].b = m[7] + m[5];
    frustumPlanes[2].c = m[11] + m[9];
    frustumPlanes[2].d = m[15] + m[13];
    frustumPlanes[2].Normalize();

    // Top
    frustumPlanes[3].a = m[3] - m[1];
    frustumPlanes[3].b = m[7] - m[5];
    frustumPlanes[3].c = m[11] - m[9];
    frustumPlanes[3].d = m[15] - m[13];
    frustumPlanes[3].Normalize();

    // Near
    frustumPlanes[4].a = m[3] + m[2];
    frustumPlanes[4].b = m[7] + m[6];
    frustumPlanes[4].c = m[11] + m[10];
    frustumPlanes[4].d = m[15] + m[14];
    frustumPlanes[4].Normalize();

    // Far
    frustumPlanes[5].a = m[3] - m[2];
    frustumPlanes[5].b = m[7] - m[6];
    frustumPlanes[5].c = m[11] - m[10];
    frustumPlanes[5].d = m[15] - m[14];
    frustumPlanes[5].Normalize();
}

bool Renderer::IsVisible(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
    // AABB Plane check
    // If AABB is completely behind any plane, it's culled.
    for (int i = 0; i < 6; i++) {
        // Find point furthest in direction of plane normal
        float px = (frustumPlanes[i].a > 0) ? maxX : minX;
        float py = (frustumPlanes[i].b > 0) ? maxY : minY;
        float pz = (frustumPlanes[i].c > 0) ? maxZ : minZ;

        if (frustumPlanes[i].Distance(px, py, pz) < 0) {
            return false; // Culled
        }
    }
    return true;
}

void Renderer::DrawGeometry(const void* vertices, int vertexSizeBytes, int vertexCount,
                            const void* indices, int indexCount,
                            GLuint textureId, GLuint programId,
                            GLenum drawMode,
                            float minX, float minY, float minZ,
                            float maxX, float maxY, float maxZ) {

    // Draw Reduction / Culling
    if (vertexCount < 2 || indexCount < 2) return;

    // Frustum Culling
    // Only cull if valid AABB provided (not all zeros or inverted)
    if (cullingEnabled && maxX >= minX) {
        if (!IsVisible(minX, minY, minZ, maxX, maxY, maxZ)) {
            return; // Culled!
        }
    }

    bool stateChanged = false;

    if (programId != currentProgramId) stateChanged = true;
    else if (textureId != currentTextureId) stateChanged = true;

    if (stateChanged) {
        batcher.Flush();

        if (programId != currentProgramId) {
            if (stateManager.UseProgram(programId)) {}
            currentProgramId = programId;
        }

        if (textureId != currentTextureId) {
            if (stateManager.BindTexture(GL_TEXTURE_2D, textureId)) {}
            currentTextureId = textureId;
        }
    }

    // Pass mode
    batcher.AddGeometry(vertices, vertexSizeBytes, vertexCount, indices, indexCount, drawMode);
}

void Renderer::SetBlendFunc(GLenum sfactor, GLenum dfactor) {
    if (stateManager.BlendFunc(sfactor, dfactor)) {
        batcher.Flush();
    }
}

void Renderer::SetDepthMask(GLboolean flag) {
    if (stateManager.DepthMask(flag)) {
        batcher.Flush();
    }
}

void Renderer::Enable(GLenum cap) {
    if (stateManager.Enable(cap)) {
        batcher.Flush();
    }
}

void Renderer::Disable(GLenum cap) {
    if (stateManager.Disable(cap)) {
        batcher.Flush();
    }
}
