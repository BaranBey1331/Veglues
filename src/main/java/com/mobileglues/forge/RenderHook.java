package com.mobileglues.forge;

import java.nio.ByteBuffer;
import java.nio.ShortBuffer;

/**
 * Interface to the native optimized renderer.
 * This class hooks into Minecraft's rendering pipeline (e.g. via Mixins in Sodium/Iris).
 */
public class RenderHook {

    static {
        // Load the optimized native library
        System.loadLibrary("mobileglues-renderer");
    }

    public static native void init();
    public static native void beginFrame();
    public static native void endFrame();

    /**
     * Submit geometry to the batcher.
     * @param vertices Direct ByteBuffer containing vertex data.
     * @param vertexSizeBytes Size of a single vertex in bytes.
     * @param vertexCount Number of vertices.
     * @param indices Direct ShortBuffer containing indices.
     * @param indexCount Number of indices.
     * @param textureId OpenGL texture ID.
     * @param shaderId OpenGL program ID.
     * @param drawMode OpenGL draw mode (GL_TRIANGLES=4, GL_LINES=1, etc.)
     * @param minX AABB Min X (for culling)
     * @param minY AABB Min Y
     * @param minZ AABB Min Z
     * @param maxX AABB Max X
     * @param maxY AABB Max Y
     * @param maxZ AABB Max Z
     */
    public static native void drawGeometry(ByteBuffer vertices, int vertexSizeBytes, int vertexCount,
                                           ShortBuffer indices, int indexCount,
                                           int textureId, int shaderId,
                                           int drawMode,
                                           float minX, float minY, float minZ,
                                           float maxX, float maxY, float maxZ);

    public static native void setViewProjection(float[] matrix);

    public static native void setBlendFunc(int sfactor, int dfactor);
    public static native void setDepthMask(boolean flag);
    public static native void enable(int cap);
    public static native void disable(int cap);

    public static native int getProgram(String vertSource, String fragSource);

    // New Optimizations
    public static native void setPerformanceMode(boolean enabled);

    public static native void setUniform1i(int location, int v0);
    public static native void setUniform1f(int location, float v0);
    public static native void setUniformMatrix4fv(int location, int count, boolean transpose, float[] value);
}
