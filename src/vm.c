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
static GFUSX_ALWAYS_INLINE void gfusx_vm_delayed_load(gfusx_vm* vm, gfu_register reg, u32 value, u32 mask);
static GFUSX_ALWAYS_INLINE void gfusx_vm_delayed_pc_load(gfusx_vm* vm, u32 value, bool from_link);
static GFUSX_ALWAYS_INLINE void gfusx_vm_do_branch(gfusx_vm* vm, u32 target, bool from_link);
static GFUSX_ALWAYS_INLINE void gfusx_vm_potential_return_addr(gfusx_vm* vm, u32 return_addr, u32 sp);

static void gfusx_vm_debug_process(u32 old_pc, u32 new_pc, u32 old_code, u32 new_code, bool linked);

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
        if (vm->next_is_delay_slot) {
            vm->in_delay_slot = true;
            vm->next_is_delay_slot = false;
        }

        // TODO(local): gfusx_read_icache(vm->pc);
        vm->code = *((u32*)&vm->icache_code[vm->pc]);
        vm->pc += 4;
        vm->cycle += GFUSX_CYCLE_BIAS;

        gfusx_vm_exec_code(vm);

        vm->current_delayed_load ^= 1;
        gfusx_delayed_load_info* delayed_load = &vm->delayed_load_info[vm->current_delayed_load];
        bool from_link = false;

        if (delayed_load->pc_active) {
            vm->pc = delayed_load->pc_value;
            from_link = delayed_load->from_link;
            delayed_load->pc_active = false;
            delayed_load->from_link = false;
        }

        if (vm->in_delay_slot) {
            vm->in_delay_slot = false;
            ran_delay_slot = true;
            // TODO(local): intercept bios
            // TODO(local): branch test
        }

        gfusx_vm_dump_regs(vm, stderr);
    } while (!ran_delay_slot); // TODO(local): && !debug
}

static GFUSX_ALWAYS_INLINE void gfusx_vm_exception(gfusx_vm* vm, gfusx_exception_kind kind, bool bd, bool cop0) {
    // TODO(local): Exception handling.
}

static GFUSX_ALWAYS_INLINE void gfusx_vm_maybe_cancel_delayed_load(gfusx_vm* vm, gfu_register reg) {
    u32 other = vm->current_delayed_load ^ 1;
    if (vm->delayed_load_info[other].index == (u32)reg) {
        vm->delayed_load_info[other].active = false;
    }
}

static GFUSX_ALWAYS_INLINE void gfusx_vm_call_stack_set_sp(gfusx_vm* vm, u32 old_sp, u32 new_sp) {
    // TODO(local): Set call stack sp.
}

static GFUSX_ALWAYS_INLINE void gfusx_vm_delayed_load(gfusx_vm* vm, gfu_register reg, u32 value, u32 mask) {
    gfu_assert(reg < 32);
    gfusx_delayed_load_info* delayed_load = &vm->delayed_load_info[vm->current_delayed_load];
    delayed_load->active = true;
    delayed_load->index = reg;
    delayed_load->mask = mask;
    delayed_load->value = value;
}

static GFUSX_ALWAYS_INLINE void gfusx_vm_delayed_pc_load(gfusx_vm* vm, u32 value, bool from_link) {
    gfusx_delayed_load_info* delayed_load = &vm->delayed_load_info[vm->current_delayed_load];
    delayed_load->pc_active = true;
    delayed_load->pc_value = value;
    delayed_load->from_link = from_link;
}

static GFUSX_ALWAYS_INLINE void gfusx_vm_do_branch(gfusx_vm* vm, u32 target, bool from_link) {
    vm->next_is_delay_slot = true;
    gfusx_vm_delayed_pc_load(vm, target, from_link);
}

static GFUSX_ALWAYS_INLINE void gfusx_vm_potential_return_addr(gfusx_vm* vm, u32 return_addr, u32 sp) {
    // TODO(local): Potential return address.
}

#define _RS_ vm->gpr.r[inst.rs]
#define _RT_ vm->gpr.r[inst.rt]
#define _RD_ vm->gpr.r[inst.rd]
#define _JMP_TARG_ ((vm->pc & 0xF0000000) | (inst.addr << 2))
#define _BR_TARG_ (u32)((i64)vm->pc + ((i16)inst.imm << 2))

static GFUSX_ALWAYS_INLINE void gfusx_vm_exec_code(gfusx_vm* vm) {
    gfu_inst inst;
    inst.raw = vm->code;

    // shortcut the NOP
    if (inst.raw == 0) return;

    switch (inst.opcode) {
        default: {
            gfusx_vm_logf(vm, GFUSX_LC_CPU, "Unimplemented opcode %02X.", inst.opcode);
        } break;

        case GFU_OPCODE_JAL: {
            gfusx_vm_maybe_cancel_delayed_load(vm, GFU_REG_RA);
            u32 return_addr = vm->pc + 4; // +8, but the previous +4 was in the caller
            vm->gpr.ra = return_addr;
            gfusx_vm_do_branch(vm, _JMP_TARG_, true);
            gfusx_vm_potential_return_addr(vm, return_addr, vm->gpr.sp);
        } break;

        case GFU_OPCODE_BEQ: {
            if (_RS_ == _RT_) {
                gfusx_vm_do_branch(vm, _BR_TARG_, false);
            }
        } break;

        case GFU_OPCODE_BNE: {
            if (_RS_ != _RT_) {
                gfusx_vm_do_branch(vm, _BR_TARG_, false);
            }
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

                // rd <- rt << shamt
                case GFU_FUNCT_SLL: {
                    if (0 == inst.rt) return;
                    _RD_ = (u32)(_RT_ << inst.shamt);
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

static void gfusx_vm_debug_process(u32 old_pc, u32 new_pc, u32 old_code, u32 new_code, bool linked) {
}
