#include <jni.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "soinfo.h"


#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

uint8_t * g_base = nullptr;
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    //手动注册
    JNIEnv * env = nullptr;
    vm->GetEnv((void**)&env, JNI_VERSION_1_6);

    jclass  cls =env->FindClass("com/kr/test/MainActivity");

    JNINativeMethod m[] = {
            {"stringFromJNI", "()Ljava/lang/String;", g_base+0x25250}
    };
    env->RegisterNatives(cls, m, 1);

    //调用真正so的JNI_OnLoad
    auto pfn = (jint(*)(JavaVM*, void*))(g_base+0x2543C);
    pfn(vm, reserved);

    return JNI_VERSION_1_6;
}

extern "C" void _init()
{
    LOGV("_init");

    //memfd_create(nullptr, 0);

    /*加载so
     0. 打开文件，读取段表
     1. 解析段表，计算所需内存大小，申请内存
     3. 映射文件到内存
     4. 解析此so的soin
     5. 重定位
     6. 使用解析的soinfo替换此自己的soinfo
     7. 调用初始化函数
     */

    //0. 打开文件，读取段表
    int so_file = open("/sdcard/libtest.so", O_EXCL | O_RDONLY);
    if (so_file < 0){
        LOGV("open 失败:%s", strerror(errno));
        return;
    }

    uint8_t buf[0x1000] = {0};
    int ret = read(so_file, buf, sizeof(buf));
    if (ret <= 0){
        LOGV("read 失败:%s", strerror(errno));
        return;
    }

    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)buf;
    Elf64_Phdr* phdr = reinterpret_cast<Elf64_Phdr *>(buf + ehdr->e_phoff);

    //1. 解析段表，计算所需内存大小，申请内存
    size_t  min_addr = 0;
    size_t  max_addr = 0;
    size_t  seg_size = 0;
    for (int i = 0; i < ehdr->e_phnum; ++i) {
        if(phdr[i].p_type != PT_LOAD){
            continue;
        }

        //最小地址
        if (min_addr >= phdr[i].p_vaddr){
            min_addr = phdr[i].p_vaddr;
        }

        //最大地址
        if (max_addr <= phdr[i].p_vaddr){
            max_addr = phdr[i].p_vaddr;
            seg_size = phdr[i].p_memsz;
        }
    }
    size_t load_size = max_addr - min_addr + seg_size; //所需内存大小

    uint8_t * load_base = (uint8_t*)mmap(nullptr, load_size,  PROT_EXEC|PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (load_base == nullptr){
        LOGV("mmap 失败：%s", strerror(errno));
        return;
    }

    //3. 映射文件到内存
    for (int i = 0; i < ehdr->e_phnum; ++i) {
        if(phdr[i].p_type != PT_LOAD){
            continue;
        }

        uint64_t map_begin = PAGE_START(phdr[i].p_vaddr);

        uint64_t file_begin = PAGE_START(phdr[i].p_offset);
        uint64_t file_end = phdr[i].p_offset+phdr[i].p_filesz;

        lseek(so_file, file_begin, SEEK_SET);
        if(read(so_file, load_base+map_begin, file_end-file_begin) <= 0){
            LOGV("read 失败：%d %s", i, strerror(errno));
        }
/*
        void* map_addr = mmap(load_base+map_begin,
                   file_end-file_begin,
                   PROT_EXEC|PROT_READ|PROT_WRITE,
                   MAP_FIXED|MAP_PRIVATE,
                   so_file,
                   file_begin);
        if (map_addr == MAP_FAILED){
            LOGV("mmap 失败：%d %s", i, strerror(errno));
            return;
        }*/
    }

    //4. 解析此so的soinfo
    soinfo* new_si = new soinfo;
    new_si->base = reinterpret_cast<Elf64_Addr>(load_base);
    new_si->size = load_size;
    new_si->phdr= reinterpret_cast<const Elf64_Phdr *>(load_base + sizeof(Elf64_Ehdr));
    new_si->phnum = ehdr->e_phnum;
    new_si->load_bias = reinterpret_cast<Elf64_Addr>(load_base);
    if(!new_si->prelink_image()) {
        LOGV("prelink_image 失败");
        return;
    }

    //5. 重定位
    if(!new_si->link_image()){
        LOGV("link_image 失败");
        return;
    }
    g_base = load_base;
    //6. 使用解析的soinfo替换此自己的soinfo
    /*
//    void* h = nullptr;
//    uint8_t * linker_base = (uint8_t *)GetLinkerBase();
//    solist_get_head_t get_head = (solist_get_head_t)(linker_base + 0x1C3E8);
//    soinfo* node = get_head();
//    while (node != nullptr){
//         Elf64_Addr init_addr = (Elf64_Addr)_init;
//        if (init_addr >= node->base && init_addr <= node->base+node->size){
//            LOGV("base:%p size:%lx phdr:%p phnum:%d dynamic:%p strtab:%p symtab:%p plt_real:%p plt_cnt:%d rela:%p rel_cnt:%d"
//                 " nbucekt:%d bucket:%d chain:%p maskword:%d shift:%d bloo:%p",
//                node->base, node->size, node->phdr, node->phnum, node->dynamic, node->strtab_,
//                node->symtab_, node->plt_rela_, node->plt_rela_count_, node->rela_, node->rela_count_,
//                node->gnu_nbucket_, node->gnu_bucket_, node->chain_, node->gnu_maskwords_, node->gnu_shift2_, node->gnu_bloom_filter_
//
//            );
//            node->load_bias = reinterpret_cast<Elf64_Addr>(load_base);
//            node->base = new_si->base;
//            node->size = new_si->size;
//            node->phdr = new_si->phdr;
//            node->phnum = new_si->phnum;
//            node->dynamic = new_si->dynamic;
//            node->strtab_= new_si->strtab_;
//            node->symtab_ = new_si->symtab_;
//            node->plt_rela_ = new_si->plt_rela_;
//            node->plt_rela_count_ = new_si->plt_rela_count_;
//            node->rela_ = new_si->rela_;
//            node->rela_count_ = new_si->rela_count_;
//            node->gnu_nbucket_ = new_si->gnu_nbucket_;
//            node->gnu_bucket_ = new_si->gnu_bucket_;
//            node->gnu_chain_ = new_si->gnu_chain_;
//            node->gnu_maskwords_ = new_si->gnu_maskwords_;
//            node->gnu_shift2_ = new_si->gnu_shift2_;
//            node->gnu_bloom_filter_ = new_si->gnu_bloom_filter_;
//            h = reinterpret_cast<void *>(node->handle_);
//
//            LOGV("base:%p size:%lx phdr:%p phnum:%d dynamic:%p strtab:%p symtab:%p plt_real:%p plt_cnt:%d rela:%p rel_cnt:%d"
//                 " nbucekt:%d bucket:%p chain:%p maskword:%d shift:%d bloo:%p",
//                 node->base, node->size, node->phdr, node->phnum, node->dynamic, node->strtab_,
//                 node->symtab_, node->plt_rela_, node->plt_rela_count_, node->rela_, node->rela_count_,
//                 node->gnu_nbucket_, node->gnu_bucket_, node->gnu_chain_, node->gnu_maskwords_, node->gnu_shift2_, node->gnu_bloom_filter_
//
//            );
//            break;
//        }
//        node = node->next;
//    }

    //7. 调用初始化函数
//    void* h2 =  dlopen("libtest.so", RTLD_NOW);
//    void* pfn2 = dlsym(h2, "Java_com_kr_test_MainActivity_stringFromJNI");
//    if(pfn2 == nullptr){
//        LOGV("dlsym 失败 %s", dlerror());
//    }
    //__asm("brk 0");
//    void* pfn = dlsym(h, "Java_com_kr_test_MainActivity_stringFromJNI");
//    if(pfn == nullptr){
//        LOGV("dlsym 失败 %s", dlerror());
//    }
//
//
//    void* pfnOnload = dlsym(h, "JNI_OnLoad");
//    if(pfn == nullptr){
//        LOGV("dlsym 失败 %s", dlerror());
//    }
//    LOGV("fromjni:%p onload:%p", pfn, pfnOnload);
    //__asm("brk 0");*/
    return ;
}
