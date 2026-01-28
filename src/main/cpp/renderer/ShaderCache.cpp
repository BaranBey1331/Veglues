#include "ShaderCache.h"
#include <iostream>
#include <vector>
#include <sstream>

ShaderCache::ShaderCache() {}

ShaderCache::~ShaderCache() {
    Clear();
}

void ShaderCache::Clear() {
    for (auto const& [key, program] : programCache) {
        // glDeleteProgram(program); // In a real app, we'd delete. Keeping it simple.
    }
    programCache.clear();
}

std::string ShaderCache::ComputeKey(const std::string& vertSource, const std::string& fragSource) {
    // Simple hash combination
    std::hash<std::string> hasher;
    size_t h1 = hasher(vertSource);
    size_t h2 = hasher(fragSource);
    return std::to_string(h1) + "_" + std::to_string(h2);
}

// Simple heuristic to downgrade precision for performance on mobile
std::string ShaderCache::OptimizeSource(const std::string& source) {
    std::string optimized = source;
    // Replace "precision highp float" with "precision mediump float"
    // This is aggressive but fits the "Performance > Visuals" goal.
    // In a real scenario, we'd parse strictly.
    size_t pos = optimized.find("precision highp float");
    if (pos != std::string::npos) {
        optimized.replace(pos, 21, "precision mediump float");
    }
    return optimized;
}

GLuint ShaderCache::GetProgram(const std::string& vertSource, const std::string& fragSource) {
    std::string key = ComputeKey(vertSource, fragSource);
    if (programCache.find(key) != programCache.end()) {
        return programCache[key];
    }

    // Try creating program
    GLuint program = glCreateProgram();

    // Attempt to load binary
    if (LoadBinary(key, program)) {
        programCache[key] = program;
        return program;
    }

    // Compile from source
    // Optimize fragment shader source
    std::string optFragSource = OptimizeSource(fragSource);

    GLuint vShader = CompileShader(GL_VERTEX_SHADER, vertSource.c_str());
    GLuint fShader = CompileShader(GL_FRAGMENT_SHADER, optFragSource.c_str());

    if (!vShader || !fShader) {
        // Error handling
        return 0;
    }

    glAttachShader(program, vShader);
    glAttachShader(program, fShader);

    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        // Handle error
        glDeleteProgram(program);
        return 0;
    }

    // Save binary for next time
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
        // Print log
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool ShaderCache::LoadBinary(const std::string& key, GLuint program) {
    // Stub: In a real implementation, read from disk/file using 'key'
    // std::vector<uint8_t> data = FileSystem::Read(key);
    // if (data.empty()) return false;
    // GLenum format = ...;
    // glProgramBinary(program, format, data.data(), data.size());
    // return (glGetError() == GL_NO_ERROR);
    return false;
}

void ShaderCache::SaveBinary(const std::string& key, GLuint program) {
    GLint length = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &length);
    if (length > 0) {
        std::vector<char> buffer(length);
        GLenum format = 0;
        glGetProgramBinary(program, length, NULL, &format, buffer.data());

        // Stub: Write to disk
        // FileSystem::Write(key, format, buffer);
    }
}
