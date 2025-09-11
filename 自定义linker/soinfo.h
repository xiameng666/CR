//
// Created by USER on 2025/9/8.
//

#ifndef TEST_SOINFO_H
#define TEST_SOINFO_H
#include <elf.h>
#include <linux/elf.h>
#include <vector>
#include <android/log.h>
#include <link.h>
#include <sys/user.h>
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "kr", __VA_ARGS__)

#define ElfW(type) Elf64_ ## type
#define PAGE_START(x) ((x) & PAGE_MASK)
#define PAGE_OFFSET(x) ((x) & ~PAGE_MASK)
#define PAGE_END(x) PAGE_START((x) + (PAGE_SIZE-1))

typedef void (*linker_dtor_function_t)();
typedef void (*linker_ctor_function_t)(int, char**, char**);

struct soinfo;
template<typename T, typename Allocator>
class LinkedList {
    void* head_;
    void* tail_;
};


typedef LinkedList<soinfo, nullptr_t> soinfo_list_t;
typedef LinkedList<nullptr_t , nullptr_t> android_namespace_list_t;

struct soinfo {
public:
    bool prelink_image();
    bool link_image();
    bool relocate(Elf64_Rela* rela, size_t num);

public:
    const ElfW(Phdr)* phdr;
    size_t phnum;

    ElfW(Addr) base;
    size_t size;

    ElfW(Dyn)* dynamic;

    soinfo* next;
    uint32_t flags_;

    const char* strtab_;
    ElfW(Sym)* symtab_;

    size_t nbucket_;
    size_t nchain_;
    uint32_t* bucket_;
    uint32_t* chain_;


    ElfW(Rela)* plt_rela_;
    size_t plt_rela_count_;

    ElfW(Rela)* rela_;
    size_t rela_count_;


    linker_ctor_function_t* preinit_array_;
    size_t preinit_array_count_;

    linker_ctor_function_t* init_array_;
    size_t init_array_count_;
    linker_dtor_function_t* fini_array_;
    size_t fini_array_count_;

    linker_ctor_function_t init_func_;
    linker_dtor_function_t fini_func_;


    size_t ref_count_;

    link_map link_map_head;

    bool constructors_called;

    ElfW(Addr) load_bias;
    bool has_DT_SYMBOLIC;

    // This part of the structure is only available
    // when FLAG_NEW_SOINFO is set in this->flags.
    uint32_t version_;

    // version >= 0
    dev_t st_dev_;
    ino_t st_ino_;

    // dependency graph
    soinfo_list_t children_;
    soinfo_list_t parents_;

    // version >= 1
    off64_t file_offset_;
    uint32_t rtld_flags_;
    uint32_t dt_flags_1_;
    size_t strtab_size_;

    // version >= 2

    size_t gnu_nbucket_;
    uint32_t* gnu_bucket_;
    uint32_t* gnu_chain_;
    uint32_t gnu_maskwords_;
    uint32_t gnu_shift2_;
    ElfW(Addr)* gnu_bloom_filter_;

    soinfo* local_group_root_;

    uint8_t* android_relocs_;
    size_t android_relocs_size_;

    const char* soname_;
    std::string realpath_;

    const ElfW(Versym)* versym_;

    ElfW(Addr) verdef_ptr_;
    size_t verdef_cnt_;

    ElfW(Addr) verneed_ptr_;
    size_t verneed_cnt_;

    uint32_t target_sdk_version_;

    // version >= 3
    std::vector<std::string> dt_runpath_;
    void* primary_namespace_;
    android_namespace_list_t secondary_namespaces_;
    uintptr_t handle_;

    std::vector<std::string> needs;
};


typedef struct soinfo* (*solist_get_head_t)();

inline void* GetLinkerBase() {
    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) {
        LOGV("fopen");
        return NULL;
    }

    char line[1024];
    void* base = NULL;

    while (fgets(line, sizeof(line), maps)) {
        if (strstr(line, "/linker64")) {
            void* start;
            sscanf(line, "%p-", &start);
            base = start;
            break;
        }
    }

    fclose(maps);
    return base;
}

#endif //TEST_SOINFO_H
