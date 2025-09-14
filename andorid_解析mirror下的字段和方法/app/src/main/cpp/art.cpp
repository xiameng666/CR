//
// Created by USER on 2025/9/13.
//
#include "art.h"
#include <shadowhook.h>

Thread::DecodeJObject_t Thread::DecodeJObject_art = nullptr;
ArtField::PrettyField_t ArtField::PrettyField_raw = nullptr;

ArtMethod::PrettyMethod_t ArtMethod::PrettyMethod_raw = nullptr;

bool InitArt()
{
    void* hArt = shadowhook_dlopen("libart.so");
    if (hArt == nullptr){
        LOGV("打开art失败 %s", shadowhook_to_errmsg(shadowhook_get_errno()));
        return false;
    }

    Thread::DecodeJObject_art = (Thread::DecodeJObject_t)shadowhook_dlsym(hArt, "_ZNK3art6Thread13DecodeJObjectEP8_jobject");
    if (Thread::DecodeJObject_art == nullptr){
        LOGV("DecodeJObject_art失败 %s", shadowhook_to_errmsg(shadowhook_get_errno()));
        return false;
    }

    ArtField::PrettyField_raw = (ArtField::PrettyField_t) shadowhook_dlsym(hArt, "_ZN3art8ArtField11PrettyFieldEb");
    if (ArtField::PrettyField_raw == nullptr){
        LOGV("PrettyField_raw %s", shadowhook_to_errmsg(shadowhook_get_errno()));
        return false;
    }


    ArtMethod::PrettyMethod_raw = (ArtMethod::PrettyMethod_t) shadowhook_dlsym(hArt, "_ZN3art9ArtMethod12PrettyMethodEb");
    if (ArtMethod::PrettyMethod_raw == nullptr){
        LOGV("PrettyMethod_raw %s", shadowhook_to_errmsg(shadowhook_get_errno()));
        return false;
    }
    return true;
}