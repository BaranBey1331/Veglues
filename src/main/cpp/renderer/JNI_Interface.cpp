#include <jni.h>
#include "Renderer.h"

extern "C" {

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_init(JNIEnv* env, jclass clazz) {
    Renderer::GetInstance().Init();
}

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_beginFrame(JNIEnv* env, jclass clazz) {
    Renderer::GetInstance().BeginFrame();
}

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_endFrame(JNIEnv* env, jclass clazz) {
    Renderer::GetInstance().EndFrame();
}

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_drawGeometry(JNIEnv* env, jclass clazz,
        jobject vertices, jint vertexSizeBytes, jint vertexCount,
        jobject indices, jint indexCount,
        jint textureId, jint shaderId,
        jint drawMode,
        jfloat minX, jfloat minY, jfloat minZ,
        jfloat maxX, jfloat maxY, jfloat maxZ) {

    void* vPtr = env->GetDirectBufferAddress(vertices);
    void* iPtr = env->GetDirectBufferAddress(indices);

    if (vPtr && iPtr) {
        Renderer::GetInstance().DrawGeometry(vPtr, vertexSizeBytes, vertexCount, iPtr, indexCount,
                                             (GLuint)textureId, (GLuint)shaderId, (GLenum)drawMode,
                                             minX, minY, minZ, maxX, maxY, maxZ);
    }
}

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_setViewProjection(JNIEnv* env, jclass clazz, jfloatArray matrix) {
    jfloat* mat = env->GetFloatArrayElements(matrix, NULL);
    if (mat) {
        Renderer::GetInstance().SetViewProj(mat);
        env->ReleaseFloatArrayElements(matrix, mat, JNI_ABORT);
    }
}

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_setBlendFunc(JNIEnv* env, jclass clazz, jint sfactor, jint dfactor) {
    Renderer::GetInstance().SetBlendFunc((GLenum)sfactor, (GLenum)dfactor);
}

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_setDepthMask(JNIEnv* env, jclass clazz, jboolean flag) {
    Renderer::GetInstance().SetDepthMask((GLboolean)flag);
}

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_setColorMask(JNIEnv* env, jclass clazz, jboolean r, jboolean g, jboolean b, jboolean a) {
    Renderer::GetInstance().SetColorMask((GLboolean)r, (GLboolean)g, (GLboolean)b, (GLboolean)a);
}

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_enable(JNIEnv* env, jclass clazz, jint cap) {
    Renderer::GetInstance().Enable((GLenum)cap);
}

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_disable(JNIEnv* env, jclass clazz, jint cap) {
    Renderer::GetInstance().Disable((GLenum)cap);
}

JNIEXPORT jint JNICALL Java_com_mobileglues_forge_RenderHook_getProgram(JNIEnv* env, jclass clazz, jstring vertSource, jstring fragSource) {
    const char* vStr = env->GetStringUTFChars(vertSource, NULL);
    const char* fStr = env->GetStringUTFChars(fragSource, NULL);

    GLuint program = Renderer::GetInstance().GetShaderCache().GetProgram(vStr ? vStr : "", fStr ? fStr : "");

    if (vStr) env->ReleaseStringUTFChars(vertSource, vStr);
    if (fStr) env->ReleaseStringUTFChars(fragSource, fStr);

    return (jint)program;
}

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_setPerformanceMode(JNIEnv* env, jclass clazz, jboolean enabled) {
    Renderer::GetInstance().SetPerformanceMode(enabled);
}

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_setExtremeMode(JNIEnv* env, jclass clazz, jboolean enabled) {
    Renderer::GetInstance().SetExtremeMode(enabled);
}

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_setThermalControl(JNIEnv* env, jclass clazz, jboolean enabled) {
    Renderer::GetInstance().SetThermalControl(enabled);
}

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_setUniform1i(JNIEnv* env, jclass clazz, jint location, jint v0) {
    Renderer::GetInstance().SetUniform1i(location, v0);
}

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_setUniform1f(JNIEnv* env, jclass clazz, jint location, jfloat v0) {
    Renderer::GetInstance().SetUniform1f(location, v0);
}

JNIEXPORT void JNICALL Java_com_mobileglues_forge_RenderHook_setUniformMatrix4fv(JNIEnv* env, jclass clazz, jint location, jint count, jboolean transpose, jfloatArray value) {
    jfloat* mat = env->GetFloatArrayElements(value, NULL);
    if (mat) {
        Renderer::GetInstance().SetUniformMatrix4fv(location, count, transpose, mat);
        env->ReleaseFloatArrayElements(value, mat, JNI_ABORT);
    }
}

}
