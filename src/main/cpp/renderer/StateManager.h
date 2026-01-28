#ifndef MOBILEGLUES_STATEMANAGER_H
#define MOBILEGLUES_STATEMANAGER_H

#include <GLES3/gl3.h>
#include <unordered_map>
#include <vector>

// Structure to track uniform state
struct UniformValue {
    GLint iVal[4];
    GLfloat fVal[4];
    GLfloat mVal[16];
    bool isInt;
    bool isMatrix;
    int count; // For vectors/matrices
    bool valid;

    UniformValue() : isInt(false), isMatrix(false), count(0), valid(false) {}
};

class StateManager {
public:
    StateManager();
    ~StateManager();

    // Program
    bool UseProgram(GLuint program);

    // Texture
    bool BindTexture(GLenum target, GLuint texture);
    void ActiveTexture(GLenum texture);

    // Buffer
    bool BindBuffer(GLenum target, GLuint buffer);

    // Capabilities
    bool Enable(GLenum cap);
    bool Disable(GLenum cap);

    // Blending
    bool BlendFunc(GLenum sfactor, GLenum dfactor);

    // Depth
    bool DepthMask(GLboolean flag);

    // Viewport/Scissor
    bool Viewport(GLint x, GLint y, GLsizei width, GLsizei height);
    bool Scissor(GLint x, GLint y, GLsizei width, GLsizei height);

    // Uniforms (New)
    // Returns true if cache missed and GL was called
    bool Uniform1i(GLint location, GLint v0);
    bool Uniform1f(GLint location, GLfloat v0);
    bool UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

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

    GLint vpX, vpY;
    GLsizei vpW, vpH;
    bool vpCached;

    GLint scX, scY;
    GLsizei scW, scH;
    bool scCached;

    // Uniform Cache: Map Location -> Value
    // Note: Locations are per-program. So we must clear or namespace by program.
    // For simplicity, we clear on Program change, OR use a map<Program, map<Loc, Val>>.
    // Given the high FPS requirement, clearing on Program switch is acceptable if switches are batched.
    // Or we assume the caller manages locations carefully.
    // Let's use a dense array if locations are small, or a map. Locations can be large.
    // Since we reset on UseProgram anyway (standard practice to avoid stale state issues across programs),
    // we just keep a current cache.
    std::unordered_map<GLint, UniformValue> uniformCache;
};

#endif // MOBILEGLUES_STATEMANAGER_H
