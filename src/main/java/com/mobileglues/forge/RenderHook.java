package com.mobileglues.forge;

import java.nio.ByteBuffer;
import java.nio.ShortBuffer;

/**
 * Interface to the native optimized renderer.
 */
public class RenderHook {

    static {
        System.loadLibrary("mobileglues-renderer");
    }

    public static native void init();
    public static native void beginFrame();
    public static native void endFrame();

    public static native void drawGeometry(ByteBuffer vertices, int vertexSizeBytes, int vertexCount,
                                           ShortBuffer indices, int indexCount,
                                           int textureId, int shaderId,
                                           int drawMode,
                                           float minX, float minY, float minZ,
                                           float maxX, float maxY, float maxZ);

    public static native void setViewProjection(float[] matrix);

    public static native void setBlendFunc(int sfactor, int dfactor);
    public static native void setDepthMask(boolean flag);
    public static native void setColorMask(boolean r, boolean g, boolean b, boolean a);
    public static native void enable(int cap);
    public static native void disable(int cap);

    public static native int getProgram(String vertSource, String fragSource);

    // Optimizations
    public static native void setPerformanceMode(boolean enabled);
    public static native void setExtremeMode(boolean enabled);
    public static native void setThermalControl(boolean enabled);

    public static native void setUniform1i(int location, int v0);
    public static native void setUniform1f(int location, float v0);
    public static native void setUniformMatrix4fv(int location, int count, boolean transpose, float[] value);
}
