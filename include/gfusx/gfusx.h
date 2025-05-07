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
/// Fancy Constant Values.                                                   ///
/// ======================================================================== ///

// TODO(local): This is C23 only, so if we want to support older standards we need a fallback implementation.
#if defined(__has_include) && __has_include(<stdbit.h>)
#    include <stdbit.h>
#    define GFUSX_BIG_ENDIAN __STDC_ENDIAN_BIG__
#    define GFUSX_LITTLE_ENDIAN __STDC_ENDIAN_LITTLE__
#    define GFUSX_NATIVE_ENDIAN __STDC_ENDIAN_NATIVE__
#else
// hope for the best, my guy
#    define GFUSX_BIG_ENDIAN 0
#    define GFUSX_LITTLE_ENDIAN 1
#    define GFUSX_NATIVE_ENDIAN 1
#endif

#define GFUSX_IS_NATIVE_LITTLE_ENDIAN (GFUSX_NATIVE_ENDIAN == GFUSX_LITTLE_ENDIAN)
#define GFUSX_IS_NATIVE_BIG_ENDIAN (GFUSX_NATIVE_ENDIAN == GFUSX_BIG_ENDIAN)
#define GFUSX_IS_NATIVE_MIXED_ENDIAN (!GFUSX_IS_NATIVE_LITTLE_ENDIAN && !GFUSX_IS_NATIVE_BIG_ENDIAN)

#define GFUSX_CYCLE_BIAS 2

/// ======================================================================== ///
/// Virtual Machine State.                                                   ///
/// ======================================================================== ///

typedef union gfusx_mips_gpregs {
    u32 r[34];
    struct {
        u32 r0, at, v0, v1, a0, a1, a2, a3;
        u32 t0, t1, t2, t3, t4, t5, t6, t7;
        u32 s0, s1, s2, s3, s4, s5, s6, s7;
        u32 t8, t9, k0, k1, gp, sp, fp, ra;
        u32 hi, lo;
    };
} gfusx_mips_gpregs;

typedef union gfusx_cop0_regs {
    u32 r[32];
    struct {
        u32 index, random, entry_lo0, bpc, context, bda, pid_mask, dcic;
        u32 bad_vaddr, bdam, entry_hi, bpcm, status, cause, epc, pr_id;
        u32 config, ll_addr, watch_lo, watch_hi, xcontext, reserved1, reserved2, reserved3;
        u32 reserved4, reserved5, ecc, cache_err, tag_lo, tag_hi, error_epc, reserved6;
    };
} gfusx_cop0_regs;

/*

typedef union gfusx_bytes32 {
#if GFUSX_IS_NATIVE_BIG_ENDIAN
    struct {
        u8 h3, h2, h, l;
    } b;
    struct {
        i8 h3, h2, h, l;
    } sb;
    struct {
        u16 h, l;
    } w;
    struct {
        i16 h, l;
    } sw;
#else
    struct {
        u8 l, h, h2, h3;
    } b;
    struct {
        i8 l, h, h2, h3;
    } sb;
    struct {
        u16 l, h;
    } w;
    struct {
        i16 l, h;
    } sw;
#endif
    u32 d;
    i32 sd;
} gfusx_bytes32;

typedef struct gfusx_cop2_svector2 {
    i16 x, y;
} gfusx_cop2_svector2;

typedef struct gfusx_cop2_svector2z {
    i16 z, unused;
} gfusx_cop2_svector2;

typedef struct gfusx_cop2_svector3 {
    i16 x, y, z, unused;
} gfusx_cop2_svector3;

typedef struct gfusx_cop2_cbgr {
    u8 r, g, b, c;
} gfusx_cop2_cbgr;

typedef struct gfusx_cop2_smatrix3 {
    i16 m11, m12, m13, m21, m22, m23, m31, m32, m33, unused;
} gfusx_cop2_smatrix3;

typedef union gfusx_cop2_data_regs {
    u32 r[32];
    gfusx_bytes32 p[32];
    struct {
        gfusx_cop2_svector3 v0, v1, v2;
        gfusx_cop2_cbgr rgb;
        i32 otz;
        i32 ir0, ir1, ir2, ir3;
        gfusx_cop2_svector2 sxy0, sxy1, sxy2, sxyp;
        gfusx_cop2_svector2z sz0, sz1, sz2, sz3;
        gfusx_cop2_cbgr rgb0, rgb1, rgb2;
        i32 reserved;
        i32 mac0, mac1, mac2, mac3;
        u32 irgb, orgb;
        i32 lzcs, lzcr;
    };
} gfusx_cop2_data_regs;

typedef union gfusx_cop2_data_ctrl {
    u32 r[32];
    gfusx_bytes32 p[32];
    struct {
        gfusx_cop2_smatrix3 rmatrix;
        i32 trx, try, trz;
        gfusx_cop2_smatrix3 lmatrix;
        i32 rbk, gbk, bbk;
        gfusx_cop2_smatrix3 cmatrix;
        i32 rfc, gfc, bfc;
        i32 ofx, ofy;
        i32 h;
        i32 dqa, dqb;
        i32 zsf3, zsf4;
        i32 flag;
    };
} gfusx_cop2_data_ctrl;

*/

typedef enum gfusx_exception_kind {
    GFUSX_EX_ARITHMETIC_OVERFLOW,
} gfusx_exception_kind;

typedef struct gfusx_settings {
    struct {
        bool debug;
    } debug;
} gfusx_settings;

typedef enum gfusx_log_class {
    GFUSX_LC_CPU,
} gfusx_log_class;

typedef struct gfusx_delayed_load_info {
    u32 index, value, mask, pc_value;
    bool active : 1;
    bool pc_active : 1;
    bool from_link : 1;
} gfusx_delayed_load_info;

typedef struct gfusx_vm {
    gfusx_mips_gpregs gpr;
    gfusx_cop0_regs cop0;
    //gfusx_cop2_data_regs cop2d;
    //gfusx_cop2_data_ctrl cop2c;
    u32 pc; // program counter
    u32 code; // current instruction
    u64 cycle;
    u64 previous_cycles;
    // TODO(local): etc...
    u8 icache_addr[0x1000];
    u8 icache_code[0x1000];

    gfusx_delayed_load_info delayed_load_info[2];
    u32 current_delayed_load : 1;
    bool next_is_delay_slot : 1;
    bool in_delay_slot : 1;

    gfusx_settings settings;
} gfusx_vm;

void gfusx_vm_power_on(gfusx_vm* vm);
void gfusx_vm_power_off(gfusx_vm* vm);
void gfusx_vm_dump_regs(gfusx_vm* vm, FILE* stream);
void gfusx_vm_step(gfusx_vm* vm);
void gfusx_vm_logf(gfusx_vm* vm, gfusx_log_class log_class, const char* format, ...);

#endif /* GFUSX_H_ */
