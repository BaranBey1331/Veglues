#ifndef MOBILEGLUES_SHADERCACHE_H
#define MOBILEGLUES_SHADERCACHE_H

#include <GLES3/gl3.h>
#include <string>
#include <vector>
#include <map>

class ShaderCache {
public:
    ShaderCache();
    ~ShaderCache();

    // Compiles a program or loads from cache if available.
    // Returns the program ID.
    GLuint GetProgram(const std::string& vertSource, const std::string& fragSource);

    // Clears the runtime cache.
    void Clear();

private:
    std::map<std::string, GLuint> programCache;

    GLuint CompileShader(GLenum type, const char* source);
    GLuint LinkProgram(GLuint vertShader, GLuint fragShader);

    // Helpers for binary cache (stubbed implementation for disk IO)
    bool LoadBinary(const std::string& key, GLuint program);
    void SaveBinary(const std::string& key, GLuint program);
    std::string ComputeKey(const std::string& vertSource, const std::string& fragSource);

    // String processing for RDNA2 optimization
    std::string OptimizeSource(const std::string& source);
};

#endif // MOBILEGLUES_SHADERCACHE_H
