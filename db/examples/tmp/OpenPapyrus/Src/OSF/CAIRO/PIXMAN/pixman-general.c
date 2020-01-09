/*
 * Copyright © 2009 Red Hat, Inc.
 * Copyright © 2000 SuSE, Inc.
 * Copyright © 2007 Red Hat, Inc.
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *             2005 Lars Knoll & Zack Rusin, Trolltech
 *             2008 Aaron Plattner, NVIDIA Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Red Hat not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Red Hat makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
#include "cairoint.h"
#pragma hdrstop

static void general_iter_init(pixman_iter_t * iter, const pixman_iter_info_t * info)
{
	pixman_image_t * image = iter->image;
	switch(image->type) {
		case BITS:
		    if((iter->iter_flags & ITER_SRC) == ITER_SRC)
			    _pixman_bits_image_src_iter_init(image, iter);
		    else
			    _pixman_bits_image_dest_iter_init(image, iter);
		    break;
		case LINEAR:
		    _pixman_linear_gradient_iter_init(image, iter);
		    break;
		case RADIAL:
		    _pixman_radial_gradient_iter_init(image, iter);
		    break;
		case CONICAL:
		    _pixman_conical_gradient_iter_init(image, iter);
		    break;
		case SOLID:
		    _pixman_log_error(FUNC, "Solid image not handled by noop");
		    break;
		default:
		    _pixman_log_error(FUNC, "Pixman bug: unknown image type\n");
		    break;
	}
}

static const pixman_iter_info_t general_iters[] =
{
	{ PIXMAN_any, 0, (iter_flags_t)0, general_iter_init, NULL, NULL },
	{ PIXMAN_null },
};

typedef struct op_info_t op_info_t;
struct op_info_t {
	uint8 src, dst;
};

#define ITER_IGNORE_BOTH						\
	(ITER_IGNORE_ALPHA | ITER_IGNORE_RGB | ITER_LOCALIZED_ALPHA)

static const op_info_t op_flags[PIXMAN_N_OPERATORS] =
{
	/* Src                   Dst                   */
	{ ITER_IGNORE_BOTH,      ITER_IGNORE_BOTH      }, /* CLEAR */
	{ ITER_LOCALIZED_ALPHA,  ITER_IGNORE_BOTH      }, /* SRC */
	{ ITER_IGNORE_BOTH,      ITER_LOCALIZED_ALPHA  }, /* DST */
	{ 0,                     ITER_LOCALIZED_ALPHA  }, /* OVER */
	{ ITER_LOCALIZED_ALPHA,  0                     }, /* OVER_REVERSE */
	{ ITER_LOCALIZED_ALPHA,  ITER_IGNORE_RGB       }, /* IN */
	{ ITER_IGNORE_RGB,       ITER_LOCALIZED_ALPHA  }, /* IN_REVERSE */
	{ ITER_LOCALIZED_ALPHA,  ITER_IGNORE_RGB       }, /* OUT */
	{ ITER_IGNORE_RGB,       ITER_LOCALIZED_ALPHA  }, /* OUT_REVERSE */
	{ 0,                     0                     }, /* ATOP */
	{ 0,                     0                     }, /* ATOP_REVERSE */
	{ 0,                     0                     }, /* XOR */
	{ ITER_LOCALIZED_ALPHA,  ITER_LOCALIZED_ALPHA  }, /* ADD */
	{ 0,                     0                     }, /* SATURATE */
};

#define SCANLINE_BUFFER_LENGTH 8192

static void general_composite_rect(pixman_implementation_t * imp, pixman_composite_info_t * info)
{
	PIXMAN_COMPOSITE_ARGS(info);
	uint8 stack_scanline_buffer[3 * SCANLINE_BUFFER_LENGTH];
	uint8 * scanline_buffer = (uint8*)stack_scanline_buffer;
	uint8 * src_buffer, * mask_buffer, * dest_buffer;
	pixman_iter_t src_iter;
	pixman_iter_t mask_iter;
	pixman_iter_t dest_iter; // !
	pixman_combine_32_func_t compose;
	pixman_bool_t component_alpha;
	iter_flags_t width_flag, src_iter_flags;
	int Bpp;
	int i;
	if((src_image->common.flags & FAST_PATH_NARROW_FORMAT) && (!mask_image || mask_image->common.flags & FAST_PATH_NARROW_FORMAT) &&
	    (dest_image->common.flags & FAST_PATH_NARROW_FORMAT)) {
		width_flag = ITER_NARROW;
		Bpp = 4;
	}
	else {
		width_flag = ITER_WIDE;
		Bpp = 16;
	}

#define ALIGN(addr) ((uint8*)((((uintptr_t)(addr)) + 15) & (~15)))

	src_buffer = ALIGN(scanline_buffer);
	mask_buffer = ALIGN(src_buffer + width * Bpp);
	dest_buffer = ALIGN(mask_buffer + width * Bpp);
	if(ALIGN(dest_buffer + width * Bpp) > scanline_buffer + sizeof(stack_scanline_buffer)) {
		scanline_buffer = (uint8 *)pixman_malloc_ab_plus_c(width, Bpp * 3, 32 * 3);
		if(!scanline_buffer)
			return;
		src_buffer = ALIGN(scanline_buffer);
		mask_buffer = ALIGN(src_buffer + width * Bpp);
		dest_buffer = ALIGN(mask_buffer + width * Bpp);
	}
	if(width_flag == ITER_WIDE) {
		/* To make sure there aren't any NANs in the buffers */
		memzero(src_buffer, width * Bpp);
		memzero(mask_buffer, width * Bpp);
		memzero(dest_buffer, width * Bpp);
	}
	// src iter 
	src_iter_flags = (iter_flags_t)(width_flag | op_flags[op].src | ITER_SRC);
	_pixman_implementation_iter_init(imp->toplevel, &src_iter, src_image,
	    src_x, src_y, width, height, src_buffer, src_iter_flags, info->src_flags);
	// mask iter 
	if((src_iter_flags & (ITER_IGNORE_ALPHA | ITER_IGNORE_RGB)) == (ITER_IGNORE_ALPHA | ITER_IGNORE_RGB)) {
		// If it doesn't matter what the source is, then it doesn't matter what the mask is
		mask_image = NULL;
	}
	component_alpha = mask_image && mask_image->common.type == BITS && 
		mask_image->common.component_alpha && PIXMAN_FORMAT_RGB(mask_image->bits.format);
	_pixman_implementation_iter_init(imp->toplevel, &mask_iter,
	    mask_image, mask_x, mask_y, width, height, mask_buffer,
	    (iter_flags_t)(ITER_SRC | width_flag | (component_alpha ? 0 : ITER_IGNORE_RGB)),
	    info->mask_flags);
	// dest iter 
	_pixman_implementation_iter_init(imp->toplevel, &dest_iter, dest_image, dest_x, dest_y, width, height,
	    dest_buffer, (iter_flags_t)(ITER_DEST | width_flag | op_flags[op].dst), info->dest_flags);
	compose = _pixman_implementation_lookup_combiner(imp->toplevel, op, component_alpha, width_flag != ITER_WIDE);
	for(i = 0; i < height; ++i) {
		const uint32_t * m = mask_iter.get_scanline(&mask_iter, 0);
		const uint32_t * s = src_iter.get_scanline(&src_iter, m);
		uint32_t * d = dest_iter.get_scanline(&dest_iter, 0);
		compose(imp->toplevel, op, d, s, m, width);
		dest_iter.write_back(&dest_iter);
	}
	if(src_iter.fini)
		src_iter.fini(&src_iter);
	if(mask_iter.fini)
		mask_iter.fini(&mask_iter);
	if(dest_iter.fini)
		dest_iter.fini(&dest_iter);
	if(scanline_buffer != (uint8*)stack_scanline_buffer)
		SAlloc::F(scanline_buffer);
}

static const pixman_fast_path_t general_fast_path[] =
{
	{ PIXMAN_OP_any, PIXMAN_any, 0, PIXMAN_any, 0, PIXMAN_any, 0, general_composite_rect },
	{ PIXMAN_OP_NONE }
};

pixman_implementation_t * _pixman_implementation_create_general(void)
{
	pixman_implementation_t * imp = _pixman_implementation_create(NULL, general_fast_path);
	_pixman_setup_combiner_functions_32(imp);
	_pixman_setup_combiner_functions_float(imp);
	imp->iter_info = general_iters;
	return imp;
}

