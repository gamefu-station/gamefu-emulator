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

#define GFUBASE_IMPLEMENTATION
#include <gfubase/gfubase.h>

#define GFUARCH_IMPLEMENTATION
#include <gfuarch/gfuarch.h>

#include <gfusx/gfusx.h>

int main(int argc, char** argv) {
    fprintf(stderr, "Hello, GFUSX!\n");

    u32 program[] = {
        gfu_inst_ori(GFU_REG_T0, GFU_REG_R0, 34),
        gfu_inst_ori(GFU_REG_T1, GFU_REG_R0, 35),
        gfu_inst_add(GFU_REG_T2, GFU_REG_T0, GFU_REG_T1),
    };

    gfusx_vm vm = {0};
    gfusx_vm_power_on(&vm);

    memcpy(vm.icache_code, program, 3 * sizeof(u32));
    for (int i = 0; i < 3; i++) {
        gfusx_vm_step(&vm);
        gfusx_vm_dump_regs(&vm, stderr);
    }

    gfusx_vm_power_off(&vm);

    return 0;
}
