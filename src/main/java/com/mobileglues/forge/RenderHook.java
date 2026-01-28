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

    /**
     * Initialize the renderer backend.
     */
    public static native void init();

    /**
     * Mark the start of a frame.
     */
    public static native void beginFrame();

    /**
     * Mark the end of a frame. Flushes any remaining batches.
     */
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
     */
    public static native void drawGeometry(ByteBuffer vertices, int vertexSizeBytes, int vertexCount,
                                           ShortBuffer indices, int indexCount,
                                           int textureId, int shaderId);

    // Optimized State Management Wrappers

    public static native void setBlendFunc(int sfactor, int dfactor);

    public static native void setDepthMask(boolean flag);

    public static native void enable(int cap);

    public static native void disable(int cap);

    // Shader Management

    /**
     * Compile or retrieve a cached shader program.
     */
    public static native int getProgram(String vertSource, String fragSource);

}
