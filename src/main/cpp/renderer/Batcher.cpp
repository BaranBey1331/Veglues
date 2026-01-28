#include "Batcher.h"
#include <iostream>

Batcher::Batcher() : currentVertexCount(0), currentIndexCount(0), currentVertexSizeBytes(0),
                     frameIndex(0), currentMode(GL_TRIANGLES) {
    vertexBuffer.reserve(VERTEX_BUFFER_SIZE);
    indexBuffer.reserve(MAX_INDICES);
    InitBuffers();
}

Batcher::~Batcher() {
    glDeleteBuffers(BUFFER_COUNT, vbos);
    glDeleteBuffers(BUFFER_COUNT, ibos);
}

void Batcher::InitBuffers() {
    glGenBuffers(BUFFER_COUNT, vbos);
    glGenBuffers(BUFFER_COUNT, ibos);

    for (int i = 0; i < BUFFER_COUNT; i++) {
        glBindBuffer(GL_ARRAY_BUFFER, vbos[i]);
        glBufferData(GL_ARRAY_BUFFER, VERTEX_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibos[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(unsigned short), NULL, GL_DYNAMIC_DRAW);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Batcher::NextFrame() {
    frameIndex = (frameIndex + 1) % BUFFER_COUNT;
}

long long Batcher::GetAllocatedBytes() const {
    return vertexBuffer.capacity() * sizeof(unsigned char) + indexBuffer.capacity() * sizeof(unsigned short);
}

void Batcher::CheckMemoryPressure() {
    // If usage > threshold (e.g. 16MB per batcher seems high given limits),
    // we might want to shrink_to_fit if empty?
    // But keeping it reserved prevents fragmentation.
    // Logic: If capacity is WAY larger than usage consistently, shrink.
    // For now, just logging or limiting growth.
    // Since we use reserve(MAX...), it shouldn't grow beyond unless changed.
}

void Batcher::Flush() {
    if (currentVertexCount == 0 || currentIndexCount == 0) return;

    DrawBatch();

    currentVertexCount = 0;
    currentIndexCount = 0;
    vertexBuffer.clear();
    indexBuffer.clear();
}

void Batcher::AddGeometry(const void* vertices, int vertexSize, int vertexCount,
                          const void* indices, int indexCount, GLenum mode) {
    if (currentVertexCount + vertexCount > MAX_VERTICES ||
        currentIndexCount + indexCount > MAX_INDICES) {
        Flush();
    }

    if (currentVertexCount > 0) {
        if (currentVertexSizeBytes != vertexSize) Flush();
        else if (currentMode != mode) Flush();
    }

    currentVertexSizeBytes = vertexSize;
    currentMode = mode;

    const unsigned char* vPtr = static_cast<const unsigned char*>(vertices);
    int bytesToAdd = vertexCount * vertexSize;
    vertexBuffer.insert(vertexBuffer.end(), vPtr, vPtr + bytesToAdd);

    const unsigned short* iPtr = static_cast<const unsigned short*>(indices);
    int baseVertex = currentVertexCount;
    for (int i = 0; i < indexCount; i++) {
        indexBuffer.push_back(iPtr[i] + baseVertex);
    }

    currentVertexCount += vertexCount;
    currentIndexCount += indexCount;

    // Check memory after insert?
    // If vector reallocated, it might have doubled.
    // We already reserved, so it shouldn't realloc unless we exceed MAX.
    // But AddGeometry logic ensures we Flush before exceeding MAX.
}

void Batcher::DrawBatch() {
    GLuint vbo = vbos[frameIndex];
    GLuint ibo = ibos[frameIndex];

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size(), vertexBuffer.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(unsigned short), indexBuffer.data(), GL_DYNAMIC_DRAW);

    // Setup Attributes based on stride guess
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, currentVertexSizeBytes, (void*)0);

    if (currentVertexSizeBytes >= 20) {
         glEnableVertexAttribArray(1);
         glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, currentVertexSizeBytes, (void*)12);
    } else {
         glDisableVertexAttribArray(1);
    }

    if (currentVertexSizeBytes >= 24) {
         glEnableVertexAttribArray(2);
         glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, currentVertexSizeBytes, (void*)20);
    } else {
         glDisableVertexAttribArray(2);
    }

    glDrawElements(currentMode, currentIndexCount, GL_UNSIGNED_SHORT, 0);
}
