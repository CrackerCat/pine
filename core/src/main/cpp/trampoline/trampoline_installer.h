//
// Created by canyie on 2020/3/19.
//

#ifndef PINE_TRAMPOLINE_INSTALLER_H
#define PINE_TRAMPOLINE_INSTALLER_H

#include "../utils/macros.h"
#include "../art/art_method.h"
#include "arch/trampolines.h"

#define INST_CASE(mask, op) \
if (UNLIKELY(((inst) & (mask)) == op)) return true

#define AS_SIZE_T(value) (reinterpret_cast<size_t>(value))
#define AS_VOID_PTR(value) (reinterpret_cast<void *>(value))
#define AS_PTR_NUM(value) (reinterpret_cast<uintptr_t>(value))
#define PTR_SIZE (sizeof(void *))

namespace pine {
    class TrampolineInstaller {
    public:
        static void InitDefault();

        static TrampolineInstaller* GetDefault() {
            return default_;
        }

        TrampolineInstaller() {};

        void Init() {
            InitTrampolines();
            kBridgeJumpTrampolineSize = SubAsSize(kCallOriginTrampoline, kBridgeJumpTrampoline);
            kCallOriginTrampolineSize = SubAsSize(kBackupTrampoline, kCallOriginTrampoline);
            kBackupTrampolineSize = SubAsSize(kTrampolinesEnd, kBackupTrampoline);
        }

        bool CannotSafeInlineHook(art::ArtMethod* target) {
            size_t target_code_size = target->GetCompiledCodeSize();
            size_t direct_jump_size = kDirectJumpTrampolineSize;
            if (UNLIKELY(target_code_size < direct_jump_size)) {
                LOGW("Cannot safe inline hook method: code size of target method too small (size %u)!",
                     target_code_size);
                return true;
            }
            if (UNLIKELY(CannotBackup(target))) {
                LOGW("Cannot safe inline hook method: code of target method has pc register related instruction!");
                return true;
            }
            return false;
        }

        void* InstallReplacementTrampoline(art::ArtMethod* target, art::ArtMethod* bridge);

        void* InstallInlineTrampoline(art::ArtMethod* target, art::ArtMethod* bridge);

        virtual bool NativeHookNoBackup(void* target, void* to);

    protected:
        static inline size_t SubAsSize(void* a, void* b) {
            return AS_SIZE_T(reinterpret_cast<uintptr_t>(a) - reinterpret_cast<uintptr_t>(b));
        }

        inline size_t DirectJumpTrampolineOffset(void* ptr) {
            return SubAsSize(ptr, kDirectJumpTrampoline);
        }

        inline size_t BridgeJumpTrampolineOffset(void* ptr) {
            return SubAsSize(ptr, kBridgeJumpTrampoline);
        }

        inline size_t CallOriginTrampolineOffset(void* ptr) {
            return SubAsSize(ptr, kCallOriginTrampoline);
        }

        inline size_t BackupTrampolineOffset(void* ptr) {
            return SubAsSize(ptr, kBackupTrampoline);
        }

        virtual void InitTrampolines() = 0;

        virtual void* CreateDirectJumpTrampoline(void* to);

        void WriteDirectJumpTrampolineTo(void* mem, void* jump_to);

        virtual void* CreateBridgeJumpTrampoline(art::ArtMethod* target, art::ArtMethod* bridge,
                                                 void* origin_code_entry);

        virtual void* CreateCallOriginTrampoline(art::ArtMethod* origin, void* original_code_entry);

        virtual bool CannotBackup(art::ArtMethod* target) = 0;

        virtual void* Backup(art::ArtMethod* target);

        static TrampolineInstaller* default_;

        void* kDirectJumpTrampoline;
        size_t kDirectJumpTrampolineEntryOffset;
        size_t kDirectJumpTrampolineSize;

        void* kBridgeJumpTrampoline;
        size_t kBridgeJumpTrampolineTargetMethodOffset;
        size_t kBridgeJumpTrampolineExtrasOffset;
        size_t kBridgeJumpTrampolineBridgeMethodOffset;
        size_t kBridgeJumpTrampolineBridgeEntryOffset;
        size_t kBridgeJumpTrampolineOriginCodeEntryOffset;
        size_t kBridgeJumpTrampolineSize;

        void* kCallOriginTrampoline;
        size_t kCallOriginTrampolineOriginMethodOffset;
        size_t kCallOriginTrampolineOriginalEntryOffset;
        size_t kCallOriginTrampolineSize;

        void* kBackupTrampoline;
        size_t kBackupTrampolineOverrideSpaceOffset;
        size_t kBackupTrampolineOriginMethodOffset;
        size_t kBackupTrampolineRemainingCodeEntryOffset;
        size_t kBackupTrampolineSize;

        void* kTrampolinesEnd;
    private:
        DISALLOW_COPY_AND_ASSIGN(TrampolineInstaller);
    };
}


#endif //PINE_TRAMPOLINE_INSTALLER_H
