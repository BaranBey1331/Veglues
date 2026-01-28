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
}

void Batcher::DrawBatch() {
    GLuint vbo = vbos[frameIndex];
    GLuint ibo = ibos[frameIndex];

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size(), vertexBuffer.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(unsigned short), indexBuffer.data(), GL_DYNAMIC_DRAW);

    // Setup Attributes based on stride guess (Fragile but optimized for this context)
    // Always Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, currentVertexSizeBytes, (void*)0);

    // UV
    if (currentVertexSizeBytes >= 20) {
         glEnableVertexAttribArray(1);
         glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, currentVertexSizeBytes, (void*)12);
    } else {
         glDisableVertexAttribArray(1); // Fix corruption
    }

    // Color
    if (currentVertexSizeBytes >= 24) {
         glEnableVertexAttribArray(2);
         glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, currentVertexSizeBytes, (void*)20);
    } else {
         glDisableVertexAttribArray(2); // Fix corruption
    }

    glDrawElements(currentMode, currentIndexCount, GL_UNSIGNED_SHORT, 0);
}
