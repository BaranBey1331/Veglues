#ifndef MOBILEGLUES_BATCHER_H
#define MOBILEGLUES_BATCHER_H

#include <GLES3/gl3.h>
#include <vector>
#include <cstring>

#define MAX_VERTICES 65536
#define MAX_INDICES 98304
#define VERTEX_BUFFER_SIZE (MAX_VERTICES * 64) // Reserve ample space (e.g. 64 bytes per vertex)

class Batcher {
public:
    Batcher();
    ~Batcher();

    // Ensures any pending geometry is drawn.
    void Flush();

    // Adds geometry to the current batch.
    // If the batch is full, it automatically Flushes.
    // Note: Caller must ensure State is consistent for this batch.
    void AddGeometry(const void* vertices, int vertexSizeBytes, int vertexCount,
                     const void* indices, int indexCount);

private:
    // CPU-side buffers
    std::vector<unsigned char> vertexBuffer;
    std::vector<unsigned short> indexBuffer;

    // GPU resources
    GLuint vbo;
    GLuint ibo;
    GLuint vao; // We might need a VAO per format, but here we assume a unified format or bind attributes before draw.
                // Actually, for simplicity/stub, we'll assume a standard layout (Pos, UV, Color, etc.)
                // or just bind the VBOs and let the caller set up attrib pointers (which is hard if we batch multiple formats).
                // Let's assume a single Vertex Layout for the main world rendering.

    int currentVertexCount;
    int currentIndexCount;
    int currentVertexSizeBytes;

    void InitBuffers();
    void DrawBatch();
};

#endif // MOBILEGLUES_BATCHER_H
