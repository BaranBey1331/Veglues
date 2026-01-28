#include "Batcher.h"
#include <iostream>

Batcher::Batcher() : currentVertexCount(0), currentIndexCount(0), currentVertexSizeBytes(0), vbo(0), ibo(0), vao(0) {
    // Pre-allocate CPU buffers to avoid runtime reallocations
    vertexBuffer.reserve(VERTEX_BUFFER_SIZE);
    indexBuffer.reserve(MAX_INDICES);
    InitBuffers();
}

Batcher::~Batcher() {
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);
    if (vao) {
        // glDeleteVertexArrays(1, &vao); // Not in our GLES3 stub, so commented out to avoid link error if not present
    }
}

void Batcher::InitBuffers() {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, VERTEX_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(unsigned short), NULL, GL_DYNAMIC_DRAW);

    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Batcher::Flush() {
    if (currentVertexCount == 0 || currentIndexCount == 0) return;

    DrawBatch();

    // Reset counters
    currentVertexCount = 0;
    currentIndexCount = 0;
    // We don't clear vectors, we just overwrite them next time,
    // but std::vector doesn't support "overwrite from 0" without clear/resize.
    // So we clear them. Since we reserved, memory is kept.
    vertexBuffer.clear();
    indexBuffer.clear();
}

void Batcher::AddGeometry(const void* vertices, int vertexSize, int vertexCount,
                          const void* indices, int indexCount) {
    // Check limits
    if (currentVertexCount + vertexCount > MAX_VERTICES ||
        currentIndexCount + indexCount > MAX_INDICES) {
        Flush();
    }

    // If vertex size changed (and we have data), we must flush because stride mismatch would occur
    // (Assuming we are drawing strictly one type of primitive/stride at a time per batch)
    if (currentVertexCount > 0 && currentVertexSizeBytes != vertexSize) {
        Flush();
    }
    currentVertexSizeBytes = vertexSize;

    // Append Vertices
    const unsigned char* vPtr = static_cast<const unsigned char*>(vertices);
    int bytesToAdd = vertexCount * vertexSize;
    vertexBuffer.insert(vertexBuffer.end(), vPtr, vPtr + bytesToAdd);

    // Append Indices
    // We need to re-index them based on currentVertexCount
    const unsigned short* iPtr = static_cast<const unsigned short*>(indices);
    int baseVertex = currentVertexCount;
    for (int i = 0; i < indexCount; i++) {
        indexBuffer.push_back(iPtr[i] + baseVertex);
    }

    currentVertexCount += vertexCount;
    currentIndexCount += indexCount;
}

void Batcher::DrawBatch() {
    // Upload Data
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Orphan the buffer (likely unnecessary if we use a ring buffer, but for now simple orphaning)
    // Actually, simple glBufferSubData might cause stalls if implicit sync happens.
    // Best practice on ANGLE/Vulkan: New buffer or orphan.
    glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size(), vertexBuffer.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(unsigned short), indexBuffer.data(), GL_DYNAMIC_DRAW);

    // We assume the caller has set up VertexAttribPointers bound to this VBO (or generic)
    // But wait, the pointers need to be set *after* binding the buffer.
    // Since we just bound them, if the VAO remembers the buffer ID, it might be old?
    // In Core GL, VAO stores buffer bindings.
    // So we might need to re-setup pointers here?
    // Or we assume the Renderer calls SetupPointers() just before this?
    // The safest is if Renderer calls `Flush` -> `Batcher::Flush` -> binds buffers -> Renderer sets pointers -> Batcher draws?
    // No, `Batcher::DrawBatch` does the draw.

    // Issue: Vertex Attributes are specific to the shader.
    // Solution: The Batcher just ensures data is in `vbo`.
    // The Caller must have called glVertexAttribPointer with correct stride/offset relative to 0 *before* this?
    // No, glVertexAttribPointer binds the currently bound ARRAY_BUFFER to the attribute.
    // So we must re-declare pointers here OR use a VAO that we update.

    // For this optimization, let's assume standard layout and re-enable attributes here?
    // Or simpler: The Batcher exposes `Bind()` which binds the VBOs, and the Renderer calls `SetupAttributes()` then `Batcher::Draw()`.
    // But `Flush()` is automatic.

    // Let's assume the Renderer has configured attributes to point to `Batcher::vbo` at offset 0 *once* (if format is constant)
    // or does it every frame.
    // If we re-upload data to the same VBO ID, the VAO links to that ID, so it should be fine.

    glDrawElements(GL_TRIANGLES, currentIndexCount, GL_UNSIGNED_SHORT, 0);
}
