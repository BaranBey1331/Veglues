#include "Renderer.h"
#include <limits>
#include <cmath>
#include <sched.h>
#include <unistd.h>

#define INVALID_GL_UINT 0xFFFFFFFF

Renderer::Renderer() : currentProgramId(INVALID_GL_UINT), currentTextureId(INVALID_GL_UINT), cullingEnabled(false), performanceMode(false) {
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
    // Attempt to pin thread on every frame start? No, just once ideally.
    // But JNI threads might change? Assuming typical game loop.
    // Pinning repeatedly is low cost syscall.
    PinThreadToPerformanceCore();
    batcher.NextFrame();
}

void Renderer::EndFrame() {
    batcher.Flush();
}

void Renderer::PinThreadToPerformanceCore() {
    // Exynos 2400:
    // Core 0-3: A520 (Little)
    // Core 4-8: A720 (Mid)
    // Core 9: X4 (Big) -> Index 9?
    // Wait, Exynos 2400 is 10 cores (1x X4 + 2x A720 + 3x A720 + 4x A520) -> 1+5+4 = 10.
    // CPU IDs: usually 0-9.
    // The prime core is usually the last index or specifically identified.
    // Let's guess Core 9 is the X4. Or pin to 4-9 (Big/Mid).
    // Safest is to allow OS scheduler but hint high priority?
    // `sched_setaffinity` allows pinning.

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    // Pin to the Prime Core (X4) or the high-performance cluster.
    // Let's try to pin to the biggest core.
    // Assuming 10 cores, index 9.
    CPU_SET(9, &cpuset);

    // Also include big cores 7,8 just in case
    CPU_SET(8, &cpuset);
    CPU_SET(7, &cpuset);

    sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
}

void Renderer::SetPerformanceMode(bool enabled) {
    performanceMode = enabled;
}

void Renderer::SetViewProj(const float* mat) {
    UpdateFrustum(mat);
    cullingEnabled = true;
}

void Renderer::UpdateFrustum(const float* m) {
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
    // Size Culling Logic (Aggressive)
    if (performanceMode) {
        float dx = maxX - minX;
        float dy = maxY - minY;
        float dz = maxZ - minZ;
        // Approximation of bounding volume
        float volume = dx * dy * dz;
        // If volume is tiny, cull it?
        // Better: Project to screen space. But that requires MVP multiplication.
        // Heuristic: Just distance check?
        // Let's rely on simple "Small Object" check.
        // e.g. < 0.2 block size
        if (volume < 0.008f) return false; // 0.2^3
    }

    for (int i = 0; i < 6; i++) {
        float px = (frustumPlanes[i].a > 0) ? maxX : minX;
        float py = (frustumPlanes[i].b > 0) ? maxY : minY;
        float pz = (frustumPlanes[i].c > 0) ? maxZ : minZ;

        if (frustumPlanes[i].Distance(px, py, pz) < 0) {
            return false;
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

    if (vertexCount < 2 || indexCount < 2) return;

    if (cullingEnabled && maxX >= minX) {
        if (!IsVisible(minX, minY, minZ, maxX, maxY, maxZ)) {
            return;
        }
    }

    bool stateChanged = false;

    if (programId != currentProgramId) stateChanged = true;
    else if (textureId != currentTextureId) stateChanged = true;

    if (stateChanged) {
        batcher.Flush();

        if (programId != currentProgramId) {
            stateManager.UseProgram(programId); // Returns bool, but we flush anyway
            currentProgramId = programId;
        }

        if (textureId != currentTextureId) {
            stateManager.BindTexture(GL_TEXTURE_2D, textureId);
            currentTextureId = textureId;
        }
    }

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

void Renderer::SetUniform1i(GLint location, GLint v0) {
    // Uniform updates do NOT require batch flush (usually).
    // They update state for NEXT draw calls.
    // BUT if we have pending batch with OLD uniform value?
    // Batcher aggregates geometry. It does NOT store uniform state per vertex.
    // So if uniform changes, we MUST flush the batch so previous geometry is drawn with old uniform.
    if (stateManager.Uniform1i(location, v0)) {
        batcher.Flush();
    }
}

void Renderer::SetUniform1f(GLint location, GLfloat v0) {
    if (stateManager.Uniform1f(location, v0)) {
        batcher.Flush();
    }
}

void Renderer::SetUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    if (stateManager.UniformMatrix4fv(location, count, transpose, value)) {
        batcher.Flush();
    }
}
