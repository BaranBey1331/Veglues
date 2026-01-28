#include "StateManager.h"
#include <limits>

#define INVALID_GL_UINT 0xFFFFFFFF
#define INVALID_GL_INT -1

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

    vpCached = false;
    scCached = false;
}

bool StateManager::UseProgram(GLuint program) {
    if (currentProgram != program) {
        currentProgram = program;
        glUseProgram(program);
        return true;
    }
    return false;
}

void StateManager::ActiveTexture(GLenum texture) {
    if (currentActiveTexture != texture) {
        currentActiveTexture = texture;
        glActiveTexture(texture);
    }
}

bool StateManager::BindTexture(GLenum target, GLuint texture) {
    if (target == GL_TEXTURE_2D) {
        int unitIndex = currentActiveTexture - GL_TEXTURE0;
        if (unitIndex >= 0 && unitIndex < 32) {
            if (boundTextures[unitIndex] != texture) {
                boundTextures[unitIndex] = texture;
                glBindTexture(target, texture);
                return true;
            }
            return false;
        }
    }
    // Fallback always binds but we return true to be safe
    glBindTexture(target, texture);
    return true;
}

bool StateManager::BindBuffer(GLenum target, GLuint buffer) {
    if (target == GL_ARRAY_BUFFER) {
        if (currentArrayBuffer != buffer) {
            currentArrayBuffer = buffer;
            glBindBuffer(target, buffer);
            return true;
        }
    } else if (target == GL_ELEMENT_ARRAY_BUFFER) {
        if (currentElementArrayBuffer != buffer) {
            currentElementArrayBuffer = buffer;
            glBindBuffer(target, buffer);
            return true;
        }
    } else {
        glBindBuffer(target, buffer);
        return true;
    }
    return false;
}

bool StateManager::Enable(GLenum cap) {
    auto it = capabilityState.find(cap);
    if (it == capabilityState.end() || !it->second) {
        capabilityState[cap] = true;
        glEnable(cap);
        return true;
    }
    return false;
}

bool StateManager::Disable(GLenum cap) {
    auto it = capabilityState.find(cap);
    if (it == capabilityState.end() || it->second) {
        capabilityState[cap] = false;
        glDisable(cap);
        return true;
    }
    return false;
}

bool StateManager::BlendFunc(GLenum sfactor, GLenum dfactor) {
    if (blendSrc != sfactor || blendDst != dfactor) {
        blendSrc = sfactor;
        blendDst = dfactor;
        glBlendFunc(sfactor, dfactor);
        return true;
    }
    return false;
}

bool StateManager::DepthMask(GLboolean flag) {
    if (!depthMaskCached || depthMask != flag) {
        depthMask = flag;
        depthMaskCached = true;
        glDepthMask(flag);
        return true;
    }
    return false;
}

bool StateManager::Viewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    if (!vpCached || vpX != x || vpY != y || vpW != width || vpH != height) {
        vpX = x; vpY = y; vpW = width; vpH = height;
        vpCached = true;
        glViewport(x, y, width, height);
        return true;
    }
    return false;
}

bool StateManager::Scissor(GLint x, GLint y, GLsizei width, GLsizei height) {
    if (!scCached || scX != x || scY != y || scW != width || scH != height) {
        scX = x; scY = y; scW = width; scH = height;
        scCached = true;
        glScissor(x, y, width, height);
        return true;
    }
    return false;
}
