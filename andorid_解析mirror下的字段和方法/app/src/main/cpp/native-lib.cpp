#include <jni.h>
#include <string>
#include <shadowhook.h>

#include "art.h"
extern "C" JNIEXPORT jstring JNICALL
Java_com_kr_test_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

__attribute__((constructor())) void lib_init()
{
    shadowhook_init(SHADOWHOOK_MODE_UNIQUE, true);
    if(!InitArt()){
        LOGV("初始化失败");
        return;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_kr_test_MainActivity_showFieldsMethods(JNIEnv *env, jobject thiz) {

    JNIEnvExt* extenv = static_cast<JNIEnvExt *>(env);

    //获取Class
    jclass cls = env->FindClass("com/kr/test/CFoo");
    MyClass* c = (MyClass*)extenv->self->DecodeJObject(cls);
    LOGV("class:%p", c);

    LOGV("************静态字段*********");
    auto sfields = c->sfields_;
    for (int i = 0; i <sfields->size_; ++i) {
        auto name = sfields->data[i].PrettyField();
        LOGV("class:%08X flag:%08X idx:%d offset:%08X name:%s",
             sfields->data[i].declaring_class_.root_.reference_,
             sfields->data[i].access_flags_,
             sfields->data[i].field_dex_idx_,
             sfields->data[i].offset_,
             name.c_str()
             );
    }

    LOGV("************实例字段*********");
    auto ifields = c->ifields_;
    for (int i = 0; i <sfields->size_; ++i) {
        auto name = ifields->data[i].PrettyField();
        LOGV("class:%08X flag:%08X idx:%d offset:%08X name:%s",
             ifields->data[i].declaring_class_.root_.reference_,
             ifields->data[i].access_flags_,
             ifields->data[i].field_dex_idx_,
             ifields->data[i].offset_,
             name.c_str()
        );
    }

    LOGV("************遍历方法*********");
    auto methods = c->methods_;
    for (int i = 0; i < methods->size_; ++i) {
        auto name = methods->data[i].PrettyMethod();
        LOGV("calss:%08X flag:%08X offset:%08X dex_index:%d index:%d hot:%d entry:%p name:%s",
             methods->data[i].declaring_class_.root_.reference_,
             methods->data[i].access_flags_.load(),
             methods->data[i].dex_code_item_offset_,
             methods->data[i].dex_method_index_,
             methods->data[i].method_index_,
             methods->data[i].hotness_count_,
             methods->data[i].ptr_sized_fields_.entry_point_from_quick_compiled_code_,
             name.c_str());
    }

    env->GetMethodID(cls, "show", "()V");

}
extern "C"
JNIEXPORT void JNICALL
Java_com_kr_test_MainActivity_updateFields(JNIEnv *env, jobject thiz, jobject obj) {

    JNIEnvExt* extenv = static_cast<JNIEnvExt *>(env);

    //获取Class
    jclass cls = env->FindClass("com/kr/test/CFoo");
    MyClass* c = (MyClass*)extenv->self->DecodeJObject(cls);
    LOGV("class:%p", c);

    //获取Object
    Object* o = (Object*)extenv->self->DecodeJObject(obj);
    LOGV("Object:%p", o);

    uint32_t f1_off = 0xEC;
    uint32_t n1_off = 0xE8;
    uint32_t n_off = 0x8;
    uint32_t f_off = 0xc;

    *(uint32_t*)((uint8_t*)c+n1_off) = 9999;
    *(float *)((uint8_t*)c+f1_off) = 99.99;

    *(uint32_t*)((uint8_t*)o+n_off) = 8888;
    *(float *)((uint8_t*)o+f_off) = 88.88;
}

// 演示偏移量访问方式
extern "C"
JNIEXPORT void JNICALL
Java_com_kr_test_MainActivity_demonstrateOffsetAccess(JNIEnv *env, jobject thiz) {
    LOGV("=== 偏移量访问演示 ===");
    
    JNIEnvExt* extenv = static_cast<JNIEnvExt *>(env);
    
    // 获取CFoo类
    jclass cls = env->FindClass("com/kr/test/CFoo");
    MyClass* c = (MyClass*)extenv->self->DecodeJObject(cls);
    
    if (c == nullptr) {
        LOGV("Failed to get CFoo class");
        return;
    }
    
    LOGV("=== 使用偏移量访问类成员 ===");
    
    // 计算methods_字段的偏移量
    size_t methods_offset = OFFSET_OF_OBJECT_MEMBER(MyClass, methods_);
    LOGV("methods_字段偏移量: 0x%zx", methods_offset);
    
    // 通过偏移量访问methods_字段
    LengthPrefixedArray<ArtMethod>* methods = 
        GetFieldByOffset<LengthPrefixedArray<ArtMethod>>(c, methods_offset);
    
    if (methods != nullptr) {
        LOGV("通过偏移量访问成功，方法数量: %d", methods->size_);
        
        // 遍历方法
        for (int i = 0; i < methods->size_; ++i) {
            auto name = methods->data[i].PrettyMethod();
            LOGV("方法[%d]: %s", i, name.c_str());
        }
    }
    
    // 计算sfields_字段的偏移量
    size_t sfields_offset = OFFSET_OF_OBJECT_MEMBER(MyClass, sfields_);
    LOGV("sfields_字段偏移量: 0x%zx", sfields_offset);
    
    // 通过偏移量访问sfields_字段
    LengthPrefixedArray<ArtField>* sfields = 
        GetFieldByOffset<LengthPrefixedArray<ArtField>>(c, sfields_offset);
    
    if (sfields != nullptr) {
        LOGV("通过偏移量访问成功，静态字段数量: %d", sfields->size_);
        
        // 遍历静态字段
        for (int i = 0; i < sfields->size_; ++i) {
            auto name = sfields->data[i].PrettyField();
            LOGV("静态字段[%d]: %s", i, name.c_str());
        }
    }
    
    LOGV("=== 偏移量访问演示完成 ===");
}