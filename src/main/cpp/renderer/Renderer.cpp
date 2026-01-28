#include "Renderer.h"
#include <limits>

#define INVALID_GL_UINT 0xFFFFFFFF

Renderer::Renderer() : currentProgramId(INVALID_GL_UINT), currentTextureId(INVALID_GL_UINT) {
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
    // Batcher inits itself
}

void Renderer::Shutdown() {
    batcher.Flush();
    shaderCache.Clear();
}

void Renderer::BeginFrame() {
    // Reset or prepare frame-level things
    // Note: We might NOT want to reset state manager fully if context persists,
    // but verifying state at start of frame is good practice.
    // For extreme performance, we trust our tracking.
}

void Renderer::EndFrame() {
    batcher.Flush();
}

void Renderer::DrawGeometry(const void* vertices, int vertexSizeBytes, int vertexCount,
                            const void* indices, int indexCount,
                            GLuint textureId, GLuint programId) {
    // Check if pipeline state changes require a flush
    bool stateChanged = false;

    if (programId != currentProgramId) {
        stateChanged = true;
    } else if (textureId != currentTextureId) {
        stateChanged = true;
    }

    // In a real engine, we'd also check if vertex format changed,
    // but Batcher::AddGeometry handles vertex stride changes by flushing.

    if (stateChanged) {
        batcher.Flush();

        // Apply new state
        if (programId != currentProgramId) {
            stateManager.UseProgram(programId);
            currentProgramId = programId;
        }

        if (textureId != currentTextureId) {
            stateManager.BindTexture(GL_TEXTURE_2D, textureId);
            currentTextureId = textureId;
        }
    }

    batcher.AddGeometry(vertices, vertexSizeBytes, vertexCount, indices, indexCount);
}

void Renderer::SetBlendFunc(GLenum sfactor, GLenum dfactor) {
    // Determine if change is needed (StateManager does this too, but we need to know to Flush)
    // Actually, we can just delegate to StateManager, but if StateManager SAYS it changed, we flushed too late?
    // No, we must Flush BEFORE changing state.
    // So we check our cached knowledge or ask StateManager (if it exposed getters).
    // For now, let's flush conservatively. "Minimize state changes" - StateManager handles the GL call skipping.
    // But Batcher must be flushed if we *intend* to change state that affects the next draw.
    // Optimization: Only flush if state IS going to change.

    // Since StateManager encapsulates current state, we might over-flush if we don't expose getters.
    // But over-flushing is better than incorrect rendering.
    // Ideally StateManager returns "true" if state changed.

    batcher.Flush();
    stateManager.BlendFunc(sfactor, dfactor);
}

void Renderer::SetDepthMask(GLboolean flag) {
    batcher.Flush();
    stateManager.DepthMask(flag);
}

void Renderer::Enable(GLenum cap) {
    batcher.Flush();
    stateManager.Enable(cap);
}

void Renderer::Disable(GLenum cap) {
    batcher.Flush();
    stateManager.Disable(cap);
}
