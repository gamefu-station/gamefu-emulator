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
    u32 pc = vm->pc;
    vm->pc += 4;

    gfu_inst inst;
    inst.raw = *((u32*)&vm->icache_code[pc]);
    vm->code = inst.raw;

    switch (inst.opcode) {
        default: {
            fprintf(stderr, "Unimplemented opcode %02X.\n", inst.opcode);
        } break;

        case GFU_OPCODE_ORI: {
            // TODO(local): What if not gpr?
            vm->gpr.r[inst.rt] = vm->gpr.r[inst.rs] | inst.imm;
        } break;

        case GFU_OPCODE_SPECIAL: {
            switch (inst.funct) {
                default: {
                    fprintf(stderr, "Unimplemented SPECIAL funct %02X.\n", inst.funct);
                } break;

                case GFU_FUNCT_ADD: {
                    // TODO(local): What if not gpr?
                    vm->gpr.r[inst.rd] = vm->gpr.r[inst.rs] + vm->gpr.r[inst.rt];
                } break;
            }
        } break;
    }
}
