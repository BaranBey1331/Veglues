#include "StateManager.h"
#include <limits>
#include <cstring>

#define INVALID_GL_UINT 0xFFFFFFFF
#define INVALID_GL_INT -1

// Define wrapped functions if not in stub
extern "C" {
void glUniform1i(GLint location, GLint v0);
void glUniform1f(GLint location, GLfloat v0);
void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
}

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

    uniformCache.clear();
}

bool StateManager::UseProgram(GLuint program) {
    if (currentProgram != program) {
        currentProgram = program;
        glUseProgram(program);
        // Uniforms are per-program state.
        // When switching programs, the "current" uniform locations refer to the new program.
        // The values in our cache for location X in old program are irrelevant for location X in new program.
        // So we MUST clear the cache.
        uniformCache.clear();
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

bool StateManager::Uniform1i(GLint location, GLint v0) {
    if (location == -1) return false;
    UniformValue& val = uniformCache[location];
    if (!val.valid || !val.isInt || val.count != 1 || val.iVal[0] != v0) {
        val.valid = true;
        val.isInt = true;
        val.isMatrix = false;
        val.count = 1;
        val.iVal[0] = v0;
        glUniform1i(location, v0);
        return true;
    }
    return false;
}

bool StateManager::Uniform1f(GLint location, GLfloat v0) {
    if (location == -1) return false;
    UniformValue& val = uniformCache[location];
    if (!val.valid || val.isInt || val.count != 1 || val.fVal[0] != v0) {
        val.valid = true;
        val.isInt = false;
        val.isMatrix = false;
        val.count = 1;
        val.fVal[0] = v0;
        glUniform1f(location, v0);
        return true;
    }
    return false;
}

bool StateManager::UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
    if (location == -1) return false;
    UniformValue& val = uniformCache[location];
    // Check if changed
    bool changed = !val.valid || val.isInt || !val.isMatrix || val.count != count;
    if (!changed) {
        // Deep compare
        if (std::memcmp(val.mVal, value, count * 16 * sizeof(float)) != 0) {
            changed = true;
        }
    }

    if (changed) {
        val.valid = true;
        val.isInt = false;
        val.isMatrix = true;
        val.count = count;
        // Only cache first 16 floats (1 matrix) for simplicity/speed if count=1?
        // If count > 1 (array), we might overflow our fixed buffer if we aren't careful.
        // `mVal` is 16 floats.
        if (count == 1) {
            std::memcpy(val.mVal, value, 16 * sizeof(float));
        } else {
             // For arrays, we don't cache (or need larger storage).
             // Fallback: invalidate cache for arrays > 1 to be safe, or just always update.
             // We'll mark invalid to force update next time too, or just update and not cache data.
             val.valid = false;
        }
        glUniformMatrix4fv(location, count, transpose, value);
        return true;
    }
    return false;
}
