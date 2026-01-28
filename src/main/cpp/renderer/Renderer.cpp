#include "Renderer.h"
#include <limits>
#include <cmath>
#include <sched.h>
#include <unistd.h>
#include <time.h>

#define INVALID_GL_UINT 0xFFFFFFFF

void Renderer_OnThermalStatusChanged(void *data, AThermalStatus status);

Renderer::Renderer() : currentProgramId(INVALID_GL_UINT), currentTextureId(INVALID_GL_UINT),
                       cullingEnabled(false), performanceMode(false), extremeMode(false), thermalControl(false),
                       thermalManager(nullptr), currentThermalStatus(ETHERMAL_STATUS_NONE),
                       lastFrameTimeNs(0), currentFPS(60.0f) {
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

    thermalManager = AThermal_acquireManager();
    if (thermalManager) {
        AThermal_registerThermalStatusListener(thermalManager, Renderer::OnThermalStatusChanged, this);
        currentThermalStatus = AThermal_getCurrentThermalStatus(thermalManager);
    }
}

void Renderer::Shutdown() {
    if (thermalManager) {
        AThermal_unregisterThermalStatusListener(thermalManager, Renderer::OnThermalStatusChanged, this);
        AThermal_releaseManager(thermalManager);
        thermalManager = nullptr;
    }
    batcher.Flush();
    shaderCache.Clear();
}

void Renderer::OnThermalStatusChanged(void *data, AThermalStatus status) {
    Renderer* self = (Renderer*)data;
    if (self) {
        self->currentThermalStatus = status;
    }
}

void Renderer::BeginFrame() {
    PinThreadToPerformanceCore();
    UpdatePerformanceMetrics();
    batcher.NextFrame();
}

void Renderer::EndFrame() {
    batcher.Flush();
}

void Renderer::UpdatePerformanceMetrics() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long long nowNs = ts.tv_sec * 1000000000LL + ts.tv_nsec;

    if (lastFrameTimeNs > 0) {
        long long delta = nowNs - lastFrameTimeNs;
        if (delta > 0) {
            currentFPS = 1000000000.0f / delta;
        }
    }
    lastFrameTimeNs = nowNs;
}

void Renderer::PinThreadToPerformanceCore() {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(9, &cpuset);
    CPU_SET(8, &cpuset);
    CPU_SET(7, &cpuset);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
}

void Renderer::SetPerformanceMode(bool enabled) { performanceMode = enabled; }
void Renderer::SetExtremeMode(bool enabled) { extremeMode = enabled; }
void Renderer::SetThermalControl(bool enabled) { thermalControl = enabled; }

void Renderer::SetViewProj(const float* mat) {
    UpdateFrustum(mat);
    cullingEnabled = true;
}

void Renderer::UpdateFrustum(const float* m) {
    frustumPlanes[0].a = m[3] + m[0]; frustumPlanes[0].b = m[7] + m[4]; frustumPlanes[0].c = m[11] + m[8]; frustumPlanes[0].d = m[15] + m[12];
    frustumPlanes[0].Normalize();
    frustumPlanes[1].a = m[3] - m[0]; frustumPlanes[1].b = m[7] - m[4]; frustumPlanes[1].c = m[11] - m[8]; frustumPlanes[1].d = m[15] - m[12];
    frustumPlanes[1].Normalize();
    frustumPlanes[2].a = m[3] + m[1]; frustumPlanes[2].b = m[7] + m[5]; frustumPlanes[2].c = m[11] + m[9]; frustumPlanes[2].d = m[15] + m[13];
    frustumPlanes[2].Normalize();
    frustumPlanes[3].a = m[3] - m[1]; frustumPlanes[3].b = m[7] - m[5]; frustumPlanes[3].c = m[11] - m[9]; frustumPlanes[3].d = m[15] - m[13];
    frustumPlanes[3].Normalize();
    frustumPlanes[4].a = m[3] + m[2]; frustumPlanes[4].b = m[7] + m[6]; frustumPlanes[4].c = m[11] + m[10]; frustumPlanes[4].d = m[15] + m[14];
    frustumPlanes[4].Normalize();
    frustumPlanes[5].a = m[3] - m[2]; frustumPlanes[5].b = m[7] - m[6]; frustumPlanes[5].c = m[11] - m[10]; frustumPlanes[5].d = m[15] - m[14];
    frustumPlanes[5].Normalize();
}

bool Renderer::IsVisible(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
    if (extremeMode) {
        float volume = (maxX - minX) * (maxY - minY) * (maxZ - minZ);
        if (volume < 0.1f) return false;
    }

    if (thermalControl && currentThermalStatus >= ETHERMAL_STATUS_SEVERE) {
        float volume = (maxX - minX) * (maxY - minY) * (maxZ - minZ);
        if (volume < 1.0f) return false;
    }

    if (performanceMode) {
        float volume = (maxX - minX) * (maxY - minY) * (maxZ - minZ);
        if (volume < 0.008f) return false;
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
            stateManager.UseProgram(programId);
            currentProgramId = programId;
        }

        if (textureId != currentTextureId) {
            stateManager.BindTexture(GL_TEXTURE_2D, textureId);
            currentTextureId = textureId;
        }
    }

    batcher.AddGeometry(vertices, vertexSizeBytes, vertexCount, indices, indexCount, drawMode);
}

// Optimization Fix: Must flush BEFORE changing state to ensure pending geometry uses OLD state.
void Renderer::SetBlendFunc(GLenum sfactor, GLenum dfactor) {
    batcher.Flush();
    stateManager.BlendFunc(sfactor, dfactor);
}
void Renderer::SetDepthMask(GLboolean flag) {
    batcher.Flush();
    stateManager.DepthMask(flag);
}
void Renderer::SetColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a) {
    batcher.Flush();
    stateManager.ColorMask(r, g, b, a);
}
void Renderer::Enable(GLenum cap) {
    batcher.Flush();
    stateManager.Enable(cap);
}
void Renderer::Disable(GLenum cap) {
    batcher.Flush();
    stateManager.Disable(cap);
}
void Renderer::SetUniform1i(GLint location, GLint v0) {
    // Uniforms update "Next Draw". If current batch pending, it uses OLD uniform.
    batcher.Flush();
    stateManager.Uniform1i(location, v0);
}
void Renderer::SetUniform1f(GLint location, GLfloat v0) {
    batcher.Flush();
    stateManager.Uniform1f(location, v0);
}
void Renderer::SetUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    batcher.Flush();
    stateManager.UniformMatrix4fv(location, count, transpose, value);
}
