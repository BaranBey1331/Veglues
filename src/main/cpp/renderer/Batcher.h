#ifndef MOBILEGLUES_BATCHER_H
#define MOBILEGLUES_BATCHER_H

#include <GLES3/gl3.h>
#include <vector>
#include <cstring>

#define MAX_VERTICES 65536
#define MAX_INDICES 98304
#define VERTEX_BUFFER_SIZE (MAX_VERTICES * 64)

#define BUFFER_COUNT 3

class Batcher {
public:
    Batcher();
    ~Batcher();

    void Flush();
    void AddGeometry(const void* vertices, int vertexSizeBytes, int vertexCount,
                     const void* indices, int indexCount, GLenum mode);

    void NextFrame();

    // Memory Control
    long long GetAllocatedBytes() const;
    void CheckMemoryPressure();

private:
    std::vector<unsigned char> vertexBuffer;
    std::vector<unsigned short> indexBuffer;

    GLuint vbos[BUFFER_COUNT];
    GLuint ibos[BUFFER_COUNT];
    int frameIndex;

    int currentVertexCount;
    int currentIndexCount;
    int currentVertexSizeBytes;
    GLenum currentMode;

    void InitBuffers();
    void DrawBatch();
};

#endif // MOBILEGLUES_BATCHER_H
