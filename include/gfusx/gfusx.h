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

#ifndef GFUSX_H_
#define GFUSX_H_

#include <gfubase/gfubase.h>
#include <gfuarch/gfuarch.h>

/// ======================================================================== ///
/// Virtual Machine State.                                                   ///
/// ======================================================================== ///

typedef struct gfusx_cop {
    u32 r[GFU_REG_COUNT];
} gfusx_cop;

typedef struct gfusx_vm {
    gfusx_cop cop0;
    gfusx_cop cop2;
} gfusx_vm;

void gfusx_vm_power_on(gfusx_vm* vm);
void gfusx_vm_power_off(gfusx_vm* vm);
void gfusx_vm_step(gfusx_vm* vm);

#endif /* GFUSX_H_ */
