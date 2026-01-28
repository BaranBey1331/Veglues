#include "StateManager.h"
#include <limits>

#define INVALID_GL_UINT 0xFFFFFFFF

StateManager::StateManager() {
    Reset();
}

StateManager::~StateManager() {
}

void StateManager::Reset() {
    currentProgram = INVALID_GL_UINT;
    currentActiveTexture = INVALID_GL_UINT;
    for (int i = 0; i < 32; i++) {
        boundTextures[i] = INVALID_GL_UINT;
    }
    currentArrayBuffer = INVALID_GL_UINT;
    currentElementArrayBuffer = INVALID_GL_UINT;

    capabilityState.clear();

    blendSrc = INVALID_GL_UINT;
    blendDst = INVALID_GL_UINT;

    depthMaskCached = false;
}

void StateManager::UseProgram(GLuint program) {
    if (currentProgram != program) {
        currentProgram = program;
        glUseProgram(program);
    }
}

void StateManager::ActiveTexture(GLenum texture) {
    if (currentActiveTexture != texture) {
        currentActiveTexture = texture;
        glActiveTexture(texture);
    }
}

void StateManager::BindTexture(GLenum target, GLuint texture) {
    // We strictly optimize GL_TEXTURE_2D as it's the most common in Minecraft
    if (target == GL_TEXTURE_2D) {
        // Calculate unit index from current active texture
        // GL_TEXTURE0 is 0x84C0
        int unitIndex = currentActiveTexture - GL_TEXTURE0;
        if (unitIndex >= 0 && unitIndex < 32) {
            if (boundTextures[unitIndex] != texture) {
                boundTextures[unitIndex] = texture;
                glBindTexture(target, texture);
            }
            return;
        }
    }

    // Fallback for other targets or units out of range
    glBindTexture(target, texture);
}

void StateManager::BindBuffer(GLenum target, GLuint buffer) {
    if (target == GL_ARRAY_BUFFER) {
        if (currentArrayBuffer != buffer) {
            currentArrayBuffer = buffer;
            glBindBuffer(target, buffer);
        }
    } else if (target == GL_ELEMENT_ARRAY_BUFFER) {
        if (currentElementArrayBuffer != buffer) {
            currentElementArrayBuffer = buffer;
            glBindBuffer(target, buffer);
        }
    } else {
        glBindBuffer(target, buffer);
    }
}

void StateManager::Enable(GLenum cap) {
    auto it = capabilityState.find(cap);
    if (it == capabilityState.end() || !it->second) {
        capabilityState[cap] = true;
        glEnable(cap);
    }
}

void StateManager::Disable(GLenum cap) {
    auto it = capabilityState.find(cap);
    if (it == capabilityState.end() || it->second) {
        capabilityState[cap] = false;
        glDisable(cap);
    }
}

void StateManager::BlendFunc(GLenum sfactor, GLenum dfactor) {
    if (blendSrc != sfactor || blendDst != dfactor) {
        blendSrc = sfactor;
        blendDst = dfactor;
        glBlendFunc(sfactor, dfactor);
    }
}

void StateManager::DepthMask(GLboolean flag) {
    if (!depthMaskCached || depthMask != flag) {
        depthMask = flag;
        depthMaskCached = true;
        glDepthMask(flag);
    }
}
