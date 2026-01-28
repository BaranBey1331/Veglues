#ifndef MOBILEGLUES_STATEMANAGER_H
#define MOBILEGLUES_STATEMANAGER_H

#include <GLES3/gl3.h>
#include <unordered_map>

class StateManager {
public:
    StateManager();
    ~StateManager();

    // Program
    void UseProgram(GLuint program);

    // Texture
    void BindTexture(GLenum target, GLuint texture);
    void ActiveTexture(GLenum texture);

    // Buffer
    void BindBuffer(GLenum target, GLuint buffer);

    // Capabilities
    void Enable(GLenum cap);
    void Disable(GLenum cap);

    // Blending
    void BlendFunc(GLenum sfactor, GLenum dfactor);

    // Depth
    void DepthMask(GLboolean flag);

    // Reset cached state (useful at frame start or when external code might have messed with GL)
    void Reset();

private:
    GLuint currentProgram;
    GLuint currentActiveTexture;
    // Map of texture unit -> bound texture ID for GL_TEXTURE_2D (most common)
    // For simplicity, we optimize for 2D textures on unit 0-31
    GLuint boundTextures[32];

    GLuint currentArrayBuffer;
    GLuint currentElementArrayBuffer;

    // Capability cache
    std::unordered_map<GLenum, bool> capabilityState;

    // Blend state
    GLenum blendSrc;
    GLenum blendDst;

    // Depth state
    GLboolean depthMask;
    bool depthMaskCached;
};

#endif // MOBILEGLUES_STATEMANAGER_H
