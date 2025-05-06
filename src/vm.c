/// ======================================================================== ///
/// This file is part of the GameFU Station fantasy console project.         ///
///   GFUSX - GameFU Station Emulator                                        ///
/// ------------------------------------------------------------------------ ///
/// Copyright (C) 2025  L. C. Atticus <contact@nashiora.com>                 ///
///                                                                          ///
/// This program is free software: you can redistribute it and/or modify     ///
/// it under the terms of the GNU Affero General Public License as published ///
/// by the Free Software Foundation, either version 3 of the License, or     ///
/// (at your option) any later version.                                      ///
///                                                                          ///
/// This program is distributed in the hope that it will be useful,          ///
/// but WITHOUT ANY WARRANTY; without even the implied warranty of           ///
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            ///
/// GNU General Public License for more details.                             ///
///                                                                          ///
/// You should have received a copy of the GNU General Public License        ///
/// along with this program.  If not, see <https://www.gnu.org/licenses/>.   ///
/// ======================================================================== ///

#include <gfusx/gfusx.h>

#if defined(__clang__) || defined(__GNUC__)
#    define GFUSX_ALWAYS_INLINE inline __attribute__((__always_inline__))
#else
#    define GFUSX_ALWAYS_INLINE inline
#endif

static GFUSX_ALWAYS_INLINE void gfusx_vm_exec_code(gfusx_vm* vm);
static GFUSX_ALWAYS_INLINE void gfusx_vm_exception(gfusx_vm* vm, gfusx_exception_kind kind, bool bd, bool cop0);
static GFUSX_ALWAYS_INLINE void gfusx_vm_maybe_cancel_delayed_load(gfusx_vm* vm, gfu_register reg);
static GFUSX_ALWAYS_INLINE void gfusx_vm_call_stack_set_sp(gfusx_vm* vm, u32 old_sp, u32 new_sp);

void gfusx_vm_logf(gfusx_vm* vm, gfusx_log_class log_class, const char* format, ...) {
    gfu_string message = {0};

    va_list v;
    va_start(v, format);
    gfu_string_vsprintf(&message, format, v);
    va_end(v);

    fprintf(stderr, GFU_STR_FMT"\n", GFU_STR_ARG(message));
    gfu_da_dealloc(&message);
}

void gfusx_vm_power_on(gfusx_vm* vm) {
    *vm = (gfusx_vm) {0};
}

void gfusx_vm_power_off(gfusx_vm* vm) {
    *vm = (gfusx_vm) {0};
}

void gfusx_vm_dump_regs(gfusx_vm* vm, FILE* stream) {
    fprintf(stream, "code: %08X\n", vm->code);
    fprintf(stream, "pc: %u\n", vm->pc);
    fprintf(stream, "gpr:\n");
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            fprintf(stream, "  %08X", vm->gpr.r[j + i * 8]);
        }

        fprintf(stream, "\n");
    }

    fprintf(stream, "  %08X  %08X\n", vm->gpr.hi, vm->gpr.lo);

    fprintf(stream, "\n");
}

void gfusx_vm_step(gfusx_vm* vm) {
    bool ran_delay_slot = false;
    do {
        // TODO(local): gfusx_read_icache(vm->pc);
        vm->code = *((u32*)&vm->icache_code[vm->pc]);
        vm->pc += 4;
        vm->cycle += GFUSX_CYCLE_BIAS;

        gfusx_vm_exec_code(vm);

        if (vm->in_delay_slot) {
            vm->in_delay_slot = false;
            ran_delay_slot = true;
            // TODO(local): intercept bios
            // TODO(local): branch test
        }

        // TODO(local): implement delay slot stuff
        ran_delay_slot = true;
    } while (!ran_delay_slot); // TODO(local): && !debug
}

static GFUSX_ALWAYS_INLINE void gfusx_vm_exception(gfusx_vm* vm, gfusx_exception_kind kind, bool bd, bool cop0) {
    // TODO(local): Exception handling.
}

static GFUSX_ALWAYS_INLINE void gfusx_vm_maybe_cancel_delayed_load(gfusx_vm* vm, gfu_register reg) {
    // TODO(local): Maybe cancel delayed load.
}

static GFUSX_ALWAYS_INLINE void gfusx_vm_call_stack_set_sp(gfusx_vm* vm, u32 old_sp, u32 new_sp) {
    // TODO(local): Set call stack sp.
}

#define _RS_ vm->gpr.r[inst.rs]
#define _RT_ vm->gpr.r[inst.rt]
#define _RD_ vm->gpr.r[inst.rd]

static GFUSX_ALWAYS_INLINE void gfusx_vm_exec_code(gfusx_vm* vm) {
    gfu_inst inst;
    inst.raw = vm->code;

    switch (inst.opcode) {
        default: {
            gfusx_vm_logf(vm, GFUSX_LC_CPU, "Unimplemented opcode %02X.", inst.opcode);
        } break;

        // rt <- rs OR imm
        case GFU_OPCODE_ORI: {
            if (0 == inst.rt) return;
            gfusx_vm_maybe_cancel_delayed_load(vm, inst.rt);
            u32 new_value = _RS_ | inst.imm;
            if (inst.rd == GFU_REG_SP) gfusx_vm_call_stack_set_sp(vm, _RT_, new_value);
            _RT_ = new_value;
        } break;

        case GFU_OPCODE_SPECIAL: {
            switch (inst.funct) {
                default: {
                    gfusx_vm_logf(vm, GFUSX_LC_CPU, "Unimplemented SPECIAL funct %02X.", inst.funct);
                } break;

                // rd <- rs + rt
                case GFU_FUNCT_ADD: {
                    u32 rs = _RS_, rt = _RT_;
                    u32 new_value = rs + rt;

                    if (inst.rd == GFU_REG_SP) gfusx_vm_call_stack_set_sp(vm, _RD_, new_value);
                    if (vm->settings.debug.debug) {
                        bool overflow = ((rs ^ new_value) & (rt ^ new_value)) >> 31;
                        if (overflow) {
                            vm->pc -= 4;
                            gfusx_vm_logf(vm, GFUSX_LC_CPU, "Signed overflow in ADD instruction from 0x%08X.", vm->pc);
                            gfusx_vm_exception(vm, GFUSX_EX_ARITHMETIC_OVERFLOW, vm->in_delay_slot, false);
                            return;
                        }
                    }

                    if (inst.rd != 0) {
                        gfusx_vm_maybe_cancel_delayed_load(vm, inst.rd);
                        _RD_ = new_value;
                    }
                } break;

                // rd <- rs + rt
                case GFU_FUNCT_ADDU: {
                    if (0 == inst.rd) return;
                    gfusx_vm_maybe_cancel_delayed_load(vm, inst.rd);
                    u32 new_value = _RS_ + _RT_;
                    if (inst.rd == GFU_REG_SP) gfusx_vm_call_stack_set_sp(vm, _RD_, new_value);
                    _RD_ = new_value;
                } break;
            }
        } break;
    }
}
