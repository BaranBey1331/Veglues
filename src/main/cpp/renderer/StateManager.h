#ifndef MOBILEGLUES_STATEMANAGER_H
#define MOBILEGLUES_STATEMANAGER_H

#include <GLES3/gl3.h>
#include <unordered_map>

class StateManager {
public:
    StateManager();
    ~StateManager();

    // Program
    // Returns true if state changed
    bool UseProgram(GLuint program);

    // Texture
    // Returns true if state changed
    bool BindTexture(GLenum target, GLuint texture);
    void ActiveTexture(GLenum texture);

    // Buffer
    bool BindBuffer(GLenum target, GLuint buffer);

    // Capabilities
    // Returns true if state changed
    bool Enable(GLenum cap);
    bool Disable(GLenum cap);

    // Blending
    // Returns true if state changed
    bool BlendFunc(GLenum sfactor, GLenum dfactor);

    // Depth
    // Returns true if state changed
    bool DepthMask(GLboolean flag);

    // Viewport/Scissor (New for ANGLE optimization)
    bool Viewport(GLint x, GLint y, GLsizei width, GLsizei height);
    bool Scissor(GLint x, GLint y, GLsizei width, GLsizei height);

    // Reset cached state
    void Reset();

private:
    GLuint currentProgram;
    GLuint currentActiveTexture;
    GLuint boundTextures[32];

    GLuint currentArrayBuffer;
    GLuint currentElementArrayBuffer;

    std::unordered_map<GLenum, bool> capabilityState;

    GLenum blendSrc;
    GLenum blendDst;

    GLboolean depthMask;
    bool depthMaskCached;

    // Viewport/Scissor cache
    GLint vpX, vpY;
    GLsizei vpW, vpH;
    bool vpCached;

    GLint scX, scY;
    GLsizei scW, scH;
    bool scCached;
};

#endif // MOBILEGLUES_STATEMANAGER_H
