//
// Created by USER on 2025/9/13.
//

#ifndef TEST_ART_H
#define TEST_ART_H
#include <stdint.h>
#include <atomic>
#include <jni.h>
#include <android/log.h>
#include <string>
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "art", __VA_ARGS__)

struct MyClass;
struct Object;
struct ClassLoader;
struct DexCache;
struct ClassExt;
struct IfTable;
struct String;
struct PointerArray;
struct JavaVMExt;
struct Thread;

template<bool kPoisonReferences, class MirrorType>
struct ObjectReference{ uint32_t reference_;};
template<class MirrorType>
class HeapReference : public ObjectReference<false, MirrorType> {};
template<class MirrorType>
struct  CompressedReference : public ObjectReference<false, MirrorType>{};
template<class MirrorType>
struct GcRoot {CompressedReference<Object> root_;};

template<typename T>
struct LengthPrefixedArray {
    uint32_t size_;
    T data[100];
};

struct ArtField  {
        GcRoot<MyClass> declaring_class_;
        uint32_t access_flags_ = 0;
        uint32_t field_dex_idx_ = 0;
        uint32_t offset_ = 0;

        std::string PrettyField(bool with_type = true){
            return PrettyField_raw(this, with_type);
        }

        using PrettyField_t = std::string(*)(ArtField*, bool);
        static PrettyField_t PrettyField_raw;
};
struct ArtMethod {
    GcRoot<MyClass> declaring_class_;
    std::atomic<std::uint32_t> access_flags_;
    uint32_t dex_code_item_offset_;
    uint32_t dex_method_index_;
    uint16_t method_index_;
    uint16_t hotness_count_;
    struct PtrSizedFields {
        ArtMethod** dex_cache_resolved_methods_;
        void* data_;
        void* entry_point_from_quick_compiled_code_;
    } ptr_sized_fields_;


    std::string PrettyMethod(bool with_type = true){
        return PrettyMethod_raw(this, with_type);
    }

    using PrettyMethod_t = std::string(*)(ArtMethod*, bool);
    static PrettyMethod_t PrettyMethod_raw;
};

struct  Object {
    HeapReference<MyClass> klass_;
    uint32_t monitor_;
};

struct MyClass : public Object {
    HeapReference<ClassLoader> class_loader_;
    HeapReference<MyClass> component_type_;
    HeapReference<DexCache> dex_cache_;
    HeapReference<ClassExt> ext_data_;
    HeapReference<IfTable> iftable_;
    HeapReference<String> name_;
    HeapReference<MyClass> super_class_;
    HeapReference<PointerArray> vtable_;
    LengthPrefixedArray<ArtField>* ifields_;
    LengthPrefixedArray<ArtMethod>* methods_;
    LengthPrefixedArray<ArtField>* sfields_;
    uint32_t access_flags_;
    uint32_t class_flags_;
    uint32_t class_size_;
    pid_t clinit_thread_id_;
    int32_t dex_class_def_idx_;
    int32_t dex_type_idx_;
    uint32_t num_reference_instance_fields_;
    uint32_t num_reference_static_fields_;
    uint32_t object_size_;
    uint32_t object_size_alloc_fast_path_;
    uint32_t primitive_type_;
    uint32_t reference_instance_offsets_;
    enum Status {
        kStatusRetired = -3,  // Retired, should not be used. Use the newly cloned one instead.
        kStatusErrorResolved = -2,
        kStatusErrorUnresolved = -1,
        kStatusNotReady = 0,
        kStatusIdx = 1,  // Loaded, DEX idx in super_class_type_idx_ and interfaces_type_idx_.
        kStatusLoaded = 2,  // DEX idx values resolved.
        kStatusResolving = 3,  // Just cloned from temporary class object.
        kStatusResolved = 4,  // Part of linking.
        kStatusVerifying = 5,  // In the process of being verified.
        kStatusRetryVerificationAtRuntime = 6,  // Compile time verification failed, retry at runtime.
        kStatusVerifyingAtRuntime = 7,  // Retrying verification at runtime.
        kStatusVerified = 8,  // Logically part of linking; done pre-init.
        kStatusInitializing = 9,  // Class init in progress.
        kStatusInitialized = 10,  // Ready to go.
        kStatusMax = 11,
    };
    Status status_;
    uint16_t copied_methods_offset_;
    uint16_t virtual_methods_offset_;
};

struct Thread {
    Object* DecodeJObject(jobject obj)
    {
        return DecodeJObject_art(this, obj);
    }

    using DecodeJObject_t = Object* (*)(Thread*, jobject);
    static DecodeJObject_t DecodeJObject_art;
};

struct JNIEnvExt : public JNIEnv {
    Thread* const self;
    JavaVMExt* const vm;
};

// ClassLoader结构体
struct ClassLoader {
    HeapReference<MyClass> parent_;
    HeapReference<ClassLoader> parent_class_loader_;
    void* class_table_;  // 指向ClassTable的指针
    
    // 获取ClassTable的方法
    void* GetClassTable() {
        return class_table_;
    }
};

// ClassTable结构体
struct ClassTable {
    // 这里可以定义ClassTable的成员
    // 实际结构取决于ART版本
};

// 偏移量访问宏
#define OFFSET_OF_OBJECT_MEMBER(type, member) \
    (reinterpret_cast<uintptr_t>(&(reinterpret_cast<type*>(0)->member)))

// 通过偏移量访问成员的工具函数
template<typename T>
T* GetFieldByOffset(void* obj, size_t offset) {
    return reinterpret_cast<T*>(*reinterpret_cast<void**>((char*)obj + offset));
}

bool InitArt();

#endif //TEST_ART_H
