#include "ShaderCache.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <regex>

ShaderCache::ShaderCache() {}

ShaderCache::~ShaderCache() {
    Clear();
}

void ShaderCache::Clear() {
    for (auto const& [key, program] : programCache) {
        if (program) glDeleteProgram(program);
    }
    programCache.clear();
}

std::string ShaderCache::ComputeKey(const std::string& vertSource, const std::string& fragSource) {
    std::hash<std::string> hasher;
    size_t h1 = hasher(vertSource);
    size_t h2 = hasher(fragSource);
    return std::to_string(h1) + "_" + std::to_string(h2);
}

std::string ShaderCache::OptimizeSource(const std::string& source) {
    std::string optimized = source;

    // 1. Inject RDNA2 / Mobile Optimizations defines
    // Find version string to insert after
    size_t versionPos = optimized.find("#version");
    std::string defines = "\n#define MOBILE_FAST_PATH 1\n#define RDNA2_OPTIMIZATION 1\n#define LOW_LATENCY 1\n";

    // Feature disabling defines for aggressive FPS
    defines += "#define NO_DYNAMIC_LIGHTS 1\n#define FAST_FOG 1\n#define SIMPLE_CLOUDS 1\n";

    if (versionPos != std::string::npos) {
        size_t nextLine = optimized.find('\n', versionPos);
        if (nextLine != std::string::npos) {
            optimized.insert(nextLine + 1, defines);
        } else {
            optimized += defines;
        }
    } else {
        optimized = "#version 300 es\n" + defines + optimized;
    }

    // 2. Aggressive Precision Downgrade (highp -> mediump)
    size_t pos = 0;
    while ((pos = optimized.find("precision highp float", pos)) != std::string::npos) {
        optimized.replace(pos, 21, "precision mediump float");
        pos += 23;
    }

    // 3. Dynamic Branch Elimination (RDNA2 Optimization)
    // Replace "if (u_Something > 0.5)" with precomputed logic if possible.
    // Ideally, we'd use specialization constants, but GLES 3.0 doesn't have them easily.
    // Instead, we try to force paths.
    // For example, if we see "if (u_Fancy > 0.5)", and we defined NO_FANCY, we can replace it.
    // But safely, let's just use the Defines we injected.
    // The shader code itself needs to assume these defines exist.
    // Since we are modding the "Glue", we assume the shader packs are compatible or we are providing a "Fast" shader.

    // Hack: Flatten common discard patterns if beneficial?
    // "if (alpha < 0.1) discard" is fine.

    return optimized;
}

GLuint ShaderCache::GetProgram(const std::string& vertSource, const std::string& fragSource) {
    std::string key = ComputeKey(vertSource, fragSource);
    if (programCache.find(key) != programCache.end()) {
        return programCache[key];
    }

    GLuint program = glCreateProgram();

    if (LoadBinary(key, program)) {
        programCache[key] = program;
        return program;
    }

    std::string optVertSource = OptimizeSource(vertSource);
    std::string optFragSource = OptimizeSource(fragSource);

    GLuint vShader = CompileShader(GL_VERTEX_SHADER, optVertSource.c_str());
    GLuint fShader = CompileShader(GL_FRAGMENT_SHADER, optFragSource.c_str());

    if (!vShader || !fShader) {
        if (vShader) glDeleteShader(vShader);
        if (fShader) glDeleteShader(fShader);

        vShader = CompileShader(GL_VERTEX_SHADER, vertSource.c_str());
        fShader = CompileShader(GL_FRAGMENT_SHADER, fragSource.c_str());

        if (!vShader || !fShader) return 0;
    }

    glAttachShader(program, vShader);
    glAttachShader(program, fShader);

    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        glDeleteProgram(program);
        glDeleteShader(vShader);
        glDeleteShader(fShader);
        return 0;
    }

    SaveBinary(key, program);

    glDeleteShader(vShader);
    glDeleteShader(fShader);

    programCache[key] = program;
    return program;
}

GLuint ShaderCache::CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool ShaderCache::LoadBinary(const std::string& key, GLuint program) {
    return false;
}

void ShaderCache::SaveBinary(const std::string& key, GLuint program) {
}
