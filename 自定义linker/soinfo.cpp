//
// Created by USER on 2025/9/8.
//
#include <dlfcn.h>
#include "soinfo.h"

void phdr_table_get_dynamic_section(const ElfW(Phdr)* phdr_table, size_t phdr_count,
                                    ElfW(Addr) load_bias, ElfW(Dyn)** dynamic,
                                    ElfW(Word)* dynamic_flags) {
    *dynamic = nullptr;
    for (size_t i = 0; i<phdr_count; ++i) {
        const ElfW(Phdr)& phdr = phdr_table[i];
        if (phdr.p_type == PT_DYNAMIC) {
            *dynamic = reinterpret_cast<ElfW(Dyn)*>(load_bias + phdr.p_vaddr);
            if (dynamic_flags) {
                *dynamic_flags = phdr.p_flags;
            }
            return;
        }
    }
}
bool soinfo::prelink_image() {
    ElfW(Word) dynamic_flags = 0;
    phdr_table_get_dynamic_section(phdr, phnum, load_bias, &dynamic, &dynamic_flags);

    if (dynamic == nullptr) {
        LOGV("缺少动态段");
        return false;
    }

    uint32_t needed_count = 0;
    for (ElfW(Dyn)* d = dynamic; d->d_tag != DT_NULL; ++d) {
        LOGV("d = %p, d[0](tag) = %p d[1](val) = %p",
              d, reinterpret_cast<void*>(d->d_tag), reinterpret_cast<void*>(d->d_un.d_val));
        switch (d->d_tag) {
            case DT_SONAME:
                // this is parsed after we have strtab initialized (see below).
                break;

            case DT_GNU_HASH:
                gnu_nbucket_ = reinterpret_cast<uint32_t*>(load_bias + d->d_un.d_ptr)[0];
                gnu_maskwords_ = reinterpret_cast<uint32_t*>(load_bias + d->d_un.d_ptr)[2];
                gnu_shift2_ = reinterpret_cast<uint32_t*>(load_bias + d->d_un.d_ptr)[3];
                gnu_bloom_filter_ = reinterpret_cast<ElfW(Addr)*>(load_bias + d->d_un.d_ptr + 16);
                gnu_bucket_ = reinterpret_cast<uint32_t*>(gnu_bloom_filter_ + gnu_maskwords_);
                gnu_chain_ = gnu_bucket_ + gnu_nbucket_ -
                             reinterpret_cast<uint32_t*>(load_bias + d->d_un.d_ptr)[1];

                --gnu_maskwords_;
                break;

            case DT_STRTAB:
                strtab_ = reinterpret_cast<const char*>(load_bias + d->d_un.d_ptr);
                break;

            case DT_STRSZ:
                strtab_size_ = d->d_un.d_val;
                break;

            case DT_SYMTAB:
                symtab_ = reinterpret_cast<ElfW(Sym)*>(load_bias + d->d_un.d_ptr);
                break;


            case DT_JMPREL:
                plt_rela_ = reinterpret_cast<ElfW(Rela)*>(load_bias + d->d_un.d_ptr);
                break;


            case DT_PLTRELSZ:
                plt_rela_count_ = d->d_un.d_val / sizeof(ElfW(Rela));
                break;

        case DT_RELA:
            rela_ = reinterpret_cast<ElfW(Rela)*>(load_bias + d->d_un.d_ptr);
            break;

          case DT_RELASZ:
            rela_count_ = d->d_un.d_val / sizeof(ElfW(Rela));
            break;

          case DT_ANDROID_RELA:
            android_relocs_ = reinterpret_cast<uint8_t*>(load_bias + d->d_un.d_ptr);
            break;

          case DT_ANDROID_RELASZ:
            android_relocs_size_ = d->d_un.d_val;
            break;


        case DT_INIT:
            init_func_ = reinterpret_cast<linker_ctor_function_t>(load_bias + d->d_un.d_ptr);
            LOGV("%s constructors (DT_INIT) found at %p", init_func_);
            break;

        case DT_FINI:
            fini_func_ = reinterpret_cast<linker_dtor_function_t>(load_bias + d->d_un.d_ptr);
            LOGV(" destructors (DT_FINI) found at %p", fini_func_);
            break;

        case DT_INIT_ARRAY:
            init_array_ = reinterpret_cast<linker_ctor_function_t*>(load_bias + d->d_un.d_ptr);
            LOGV(" constructors (DT_INIT_ARRAY) found at %p", init_array_);
            break;

        case DT_INIT_ARRAYSZ:
            init_array_count_ = static_cast<uint32_t>(d->d_un.d_val) / sizeof(ElfW(Addr));
            break;

        case DT_FINI_ARRAY:
            fini_array_ = reinterpret_cast<linker_dtor_function_t*>(load_bias + d->d_un.d_ptr);
            LOGV(" destructors (DT_FINI_ARRAY) found at %p",  fini_array_);
            break;

        case DT_FINI_ARRAYSZ:
            fini_array_count_ = static_cast<uint32_t>(d->d_un.d_val) / sizeof(ElfW(Addr));
            break;

        case DT_PREINIT_ARRAY:
            preinit_array_ = reinterpret_cast<linker_ctor_function_t*>(load_bias + d->d_un.d_ptr);
            LOGV(" constructors (DT_PREINIT_ARRAY) found at %p",  preinit_array_);
            break;

        case DT_PREINIT_ARRAYSZ:
            preinit_array_count_ = static_cast<uint32_t>(d->d_un.d_val) / sizeof(ElfW(Addr));
            break;


        case DT_NEEDED:
            ++needed_count;
            break;


        case DT_VERSYM:
            versym_ = reinterpret_cast<ElfW(Versym)*>(load_bias + d->d_un.d_ptr);
            break;

        case DT_VERDEF:
            verdef_ptr_ = load_bias + d->d_un.d_ptr;
            break;
        case DT_VERDEFNUM:
            verdef_cnt_ = d->d_un.d_val;
            break;

        case DT_VERNEED:
            verneed_ptr_ = load_bias + d->d_un.d_ptr;
            break;

        case DT_VERNEEDNUM:
            verneed_cnt_ = d->d_un.d_val;
            break;

        case DT_RUNPATH:
            // this is parsed after we have strtab initialized (see below).
            break;

        default:
            LOGV("错误的动态段标志 %d", d->d_tag);
            break;
        }
    }

    // second pass - parse entries relying on strtab
    for (ElfW(Dyn)* d = dynamic; d->d_tag != DT_NULL; ++d) {
        switch (d->d_tag) {
            case DT_SONAME:
                soname_ = strtab_+d->d_un.d_val;
                break;
            case DT_RUNPATH:
                dt_runpath_.push_back(strtab_+d->d_un.d_val);
                break;
            case DT_NEEDED:
                needs.push_back(strtab_+d->d_un.d_val);
                break;
        }
    }

    return true;
}

bool soinfo::relocate(Elf64_Rela *rela, size_t num) {
    for (int i = 0; i < num; ++i) {

        //adden 不为0
        if (rela[i].r_addend != 0){
            uint64_t* addr = reinterpret_cast<uint64_t *>(rela[i].r_offset + load_bias);
            *addr = load_bias+rela[i].r_addend;
            continue;
        }

        //adden为0， 看符号
        uint32_t sym = ELF64_R_SYM(rela[i].r_info);
        //导出
        if (symtab_[sym].st_value != 0){
            uint64_t* addr = reinterpret_cast<uint64_t *>(rela[i].r_offset + load_bias);
            *addr = load_bias+symtab_[sym].st_value;
            continue;
        }

        //导入
        for (auto& need:needs) {
            void* h = dlopen(need.c_str(), RTLD_NOW);
            if (h == nullptr){
                LOGV("dlopen 失败 :%s", dlerror());
                return false;
            }

            const char* sym_name = strtab_+symtab_[sym].st_name;
            void* sym_addr = dlsym(h, sym_name);
            if (sym_addr == nullptr){
                continue;
            }

            //赋值
            uint64_t* addr = reinterpret_cast<uint64_t *>(rela[i].r_offset + load_bias);
            *addr = reinterpret_cast<uint64_t>(sym_addr);
        }


        uint64_t* addr = reinterpret_cast<uint64_t *>(rela[i].r_offset + load_bias);
        if (*addr == 0){
            LOGV("重定位失败 %d ", i);
        }
    }

    return true;
}

bool soinfo::link_image() {
    if (!relocate(rela_, rela_count_)){
        return false;
    }

    if (!relocate(plt_rela_, plt_rela_count_)){
        return false;
    }
    return true;
}
