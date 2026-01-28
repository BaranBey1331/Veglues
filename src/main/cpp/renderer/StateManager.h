#ifndef MOBILEGLUES_STATEMANAGER_H
#define MOBILEGLUES_STATEMANAGER_H

#include <GLES3/gl3.h>
#include <unordered_map>
#include <vector>

struct UniformValue {
    GLint iVal[4];
    GLfloat fVal[4];
    GLfloat mVal[16];
    bool isInt;
    bool isMatrix;
    int count;
    bool valid;

    UniformValue() : isInt(false), isMatrix(false), count(0), valid(false) {}
};

class StateManager {
public:
    StateManager();
    ~StateManager();

    bool UseProgram(GLuint program);

    bool BindTexture(GLenum target, GLuint texture);
    void ActiveTexture(GLenum texture);

    bool BindBuffer(GLenum target, GLuint buffer);

    bool Enable(GLenum cap);
    bool Disable(GLenum cap);

    bool BlendFunc(GLenum sfactor, GLenum dfactor);

    bool DepthMask(GLboolean flag);

    // New Pipeline State
    bool ColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a);
    bool StencilMask(GLuint mask);

    bool Viewport(GLint x, GLint y, GLsizei width, GLsizei height);
    bool Scissor(GLint x, GLint y, GLsizei width, GLsizei height);

    bool Uniform1i(GLint location, GLint v0);
    bool Uniform1f(GLint location, GLfloat v0);
    bool UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

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

    // New State Cache
    GLboolean colorMaskR, colorMaskG, colorMaskB, colorMaskA;
    bool colorMaskCached;
    GLuint stencilMask;
    bool stencilMaskCached;

    GLint vpX, vpY;
    GLsizei vpW, vpH;
    bool vpCached;

    GLint scX, scY;
    GLsizei scW, scH;
    bool scCached;

    std::unordered_map<GLint, UniformValue> uniformCache;
};

#endif // MOBILEGLUES_STATEMANAGER_H
