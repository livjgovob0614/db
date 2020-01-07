// LAYOUT-2D.H
// Layout - Simple 2D stacking boxes calculations
// Copyright (c) 2016 Andrew Richards <randrew@gmail.com>
//
#ifndef LAY_INCLUDE_HEADER
#define LAY_INCLUDE_HEADER

// Do this:
//
//   #define LAY_IMPLEMENTATION
//
// in exactly one C or C++ file in your project before you include layout.h.
// Your includes should look like this:
//
//   #include ...
//   #include ...
//   #define LAY_IMPLEMENTATION
//   #include "layout.h"
//
// All other files in your project should not define LAY_IMPLEMENTATION.
//#include <stdint.h>

#ifndef LAY_EXPORT
	#define LAY_EXPORT extern
#endif

// Users of this library can define LAY_ASSERT if they would like to use an
// assert other than the one from assert.h.
/*#ifndef LAY_ASSERT
	#include <assert.h>
	#define LAY_ASSERT assert
#endif*/
//
// 'static inline' for things we always want inlined -- the compiler should not
// even have to consider not inlining these.
#if defined(__GNUC__) || defined(__clang__)
	#define LAY_STATIC_INLINE __attribute__((always_inline)) static inline
#elif defined(_MSC_VER)
	#define LAY_STATIC_INLINE __forceinline static
#else
	#define LAY_STATIC_INLINE inline static
#endif
/*
typedef struct TLayout::Item {
	uint32 flags;
	lay_id   first_child;
	lay_id   next_sibling;
	lay_vec4 margins;
	lay_vec2 size;
} TLayout::Item;

typedef struct TLayout::Context {
	TLayout::Item * items;
	lay_vec4 * rects;
	lay_id capacity;
	lay_id count;
} TLayout::Context;
*/
//
// Container flags to pass to lay_set_container()
//
typedef enum lay_box_flags {
	//
	// flex-direction (bit 0+1)
	//
	LAY_ROW     = 0x002, // left to right
	LAY_COLUMN  = 0x003, // top to bottom
	//
	// model (bit 1)
	//
	LAY_LAYOUT  = 0x000, // free layout
	LAY_FLEX    = 0x002, // flex model
	//
	// flex-wrap (bit 2)
	//
	LAY_NOWRAP  = 0x000, // single-line
	LAY_WRAP    = 0x004, // multi-line, wrap left to right
	//
	// justify-content (start, end, center, space-between)
	//
	LAY_START   = 0x008, // at start of row/column
	LAY_MIDDLE  = 0x000, // at center of row/column
	LAY_END     = 0x010, // at end of row/column
	LAY_JUSTIFY = 0x018, // insert spacing to stretch across whole row/column
	//
	// align-items
	// can be implemented by putting a flex container in a layout container,
	// then using LAY_TOP, LAY_BOTTOM, LAY_VFILL, LAY_VCENTER, etc.
	// FILL is equivalent to stretch/grow
	//
	// align-content (start, end, center, stretch)
	// can be implemented by putting a flex container in a layout container,
	// then using LAY_TOP, LAY_BOTTOM, LAY_VFILL, LAY_VCENTER, etc.
	// FILL is equivalent to stretch; space-between is not supported.
	//
} lay_box_flags;
//
// child layout flags to pass to lay_set_behave()
//
typedef enum lay_layout_flags {
	//
	// attachments (bit 5-8)
	// fully valid when parent uses LAY_LAYOUT model
	// partially valid when in LAY_FLEX model
	//
	LAY_LEFT    = 0x020, // anchor to left item or left side of parent
	LAY_TOP     = 0x040, // anchor to top item or top side of parent
	LAY_RIGHT   = 0x080, // anchor to right item or right side of parent
	LAY_BOTTOM  = 0x100, // anchor to bottom item or bottom side of parent
	LAY_HFILL   = 0x0a0, // anchor to both left and right item or parent borders
	LAY_VFILL   = 0x140, // anchor to both top and bottom item or parent borders
	LAY_HCENTER = 0x000, // center horizontally, with left margin as offset
	LAY_VCENTER = 0x000, // center vertically, with top margin as offset
	LAY_CENTER  = 0x000, // center in both directions, with left/top margin as offset
	LAY_FILL    = 0x1e0, // anchor to all four directions
	//
	// When in a wrapping container, put this element on a new line. Wrapping
	// layout code auto-inserts LAY_BREAK flags as needed. See GitHub issues for
	// @todo related to this.
	//
	// Drawing routines can read this via item pointers as needed after
	// performing layout calculations.
	LAY_BREAK   = 0x200
} lay_layout_flags;

enum {
	// these bits, starting at bit 16, can be safely assigned by the
	// application, e.g. as item types, other event types, drop targets, etc.
	// this is not yet exposed via API functions, you'll need to get/set these
	// by directly accessing item pointers.
	//
	// (In reality we have more free bits than this, TODO)
	//
	// @todo fix int/unsigned size mismatch (clang issues warning for this),
	// should be all bits as 1 instead of INT_MAX
	LAY_USERMASK = 0x7fff0000,
	//
	// a special mask passed to lay_find_item() (currently does not exist, was not ported from oui)
	//
	LAY_ANY = 0x7fffffff,
};

enum {
	// extra item flags
	LAY_ITEM_BOX_MODEL_MASK = 0x000007, // bit 0-2
	LAY_ITEM_BOX_MASK       = 0x00001F, // bit 0-4
	LAY_ITEM_LAYOUT_MASK    = 0x0003E0, // bit 5-9
	LAY_ITEM_INSERTED       = 0x400, // item has been inserted (bit 10)
	LAY_ITEM_HFIXED         = 0x800, // horizontal size has been explicitly set (bit 11)
	LAY_ITEM_VFIXED         = 0x1000, // vertical size has been explicitly set (bit 12)
	LAY_ITEM_FIXED_MASK     = LAY_ITEM_HFIXED | LAY_ITEM_VFIXED, // bit 11-12
	LAY_ITEM_COMPARE_MASK   = LAY_ITEM_BOX_MODEL_MASK | (LAY_ITEM_LAYOUT_MASK & ~LAY_BREAK) | LAY_USERMASK, // which flag bits will be compared
};

LAY_STATIC_INLINE lay_vec4 lay_vec4_xyzw(lay_scalar x, lay_scalar y, lay_scalar z, lay_scalar w)
{
#if defined(__GNUC__) || defined(__clang__)
	return (lay_vec4){x, y, z, w};
#elif defined(_MSC_VER)
	lay_vec4 result;
	result[0] = x;
	result[1] = y;
	result[2] = z;
	result[3] = w;
	return result;
#endif
}
//
// Call this on a context before using it. You must also call this on a context
// if you would like to use it again after calling lay_destroy_context() on it.
//
//LAY_EXPORT void lay_init_context(TLayout::Context * ctx);
//
// Reserve enough heap memory to contain `count` items without needing to
// reallocate. The initial lay_init_context() call does not allocate any heap
// memory, so if you init a context and then call this once with a large enough
// number for the number of items you'll create, there will not be any further reallocations.
//
LAY_EXPORT void lay_reserve_items_capacity(TLayout::Context * ctx, lay_id count);
//
// Frees any heap allocated memory used by a context. Don't call this on a
// context that did not have lay_init_context() call on it. To reuse a context
// after destroying it, you will need to call lay_init_context() on it again.
//
//LAY_EXPORT void lay_destroy_context(TLayout::Context * ctx);
//
// Clears all of the items in a context, setting its count to 0. Use this when
// you want to re-declare your layout starting from the root item. This does not
// free any memory or perform allocations. It's safe to use the context again
// after calling this. You should probably use this instead of init/destroy if
// you are recalculating your layouts in a loop.
//
LAY_EXPORT void lay_reset_context(TLayout::Context * ctx);
//
// Performs the layout calculations, starting at the root item (id 0). After
// calling this, you can use lay_get_rect() to query for an item's calculated
// rectangle. If you use procedures such as lay_append() or lay_insert() after
// calling this, your calculated data may become invalid if a reallocation occurs.
//
// You should prefer to recreate your items starting from the root instead of
// doing fine-grained updates to the existing context.
//
// However, it's safe to use lay_set_size on an item, and then re-run
// lay_run_context. This might be useful if you are doing a resizing animation
// on items in a layout without any contents changing.
//
//LAY_EXPORT void lay_run_context(TLayout::Context * ctx);
//
// Like lay_run_context(), this procedure will run layout calculations --
// however, it lets you specify which item you want to start from.
// lay_run_context() always starts with item 0, the first item, as the root.
// Running the layout calculations from a specific item is useful if you want
// need to iteratively re-run parts of your layout hierarchy, or if you are only
// interested in updating certain subsets of it. Be careful when using this --
// it's easy to generated bad output if the parent items haven't yet had their
// output rectangles calculated, or if they've been invalidated (e.g. due to re-allocation).
//
//LAY_EXPORT void lay_run_item(TLayout::Context * ctx, lay_id item);
//
// Performing a layout on items where wrapping is enabled in the parent
// container can cause flags to be modified during the calculations. If you plan
// to call lay_run_context or lay_run_item multiple times without calling
// lay_reset, and if you have a container that uses wrapping, and if the width
// or height of the container may have changed, you should call
// lay_clear_item_break on all of the children of a container before calling
// lay_run_context or lay_run_item again. If you don't, the layout calculations
// may perform unnecessary wrapping.
//
// This requirement may be changed in the future.
//
// Calling this will also reset any manually-specified breaking. You will need
// to set the manual breaking again, or simply not call this on any items that
// you know you wanted to break manually.
//
// If you clear your context every time you calculate your layout, or if you
// don't use wrapping, you don't need to call this.
//
LAY_EXPORT void lay_clear_item_break(TLayout::Context * ctx, lay_id item);
//
// Returns the number of items that have been created in a context.
//
LAY_EXPORT lay_id lay_items_count(TLayout::Context * ctx);
//
// Returns the number of items the context can hold without performing a reallocation.
//
LAY_EXPORT lay_id lay_items_capacity(TLayout::Context * ctx);
//
// Create a new item, which can just be thought of as a rectangle. Returns the
// id (handle) used to identify the item.
//
//LAY_EXPORT lay_id lay_item(TLayout::Context * ctx);
//
// Inserts an item into another item, forming a parent - child relationship. An
// item can contain any number of child items. Items inserted into a parent are
// put at the end of the ordering, after any existing siblings.
//
LAY_EXPORT void lay_insert(TLayout::Context * ctx, lay_id parent, lay_id child);
//
// lay_append inserts an item as a sibling after another item. This allows
// inserting an item into the middle of an existing list of items within a
// parent. It's also more efficient than repeatedly using lay_insert(ctx,
// parent, new_child) in a loop to create a list of items in a parent, because
// it does not need to traverse the parent's children each time. So if you're
// creating a long list of children inside of a parent, you might prefer to use
// this after using lay_insert to insert the first child.
//
LAY_EXPORT void lay_append(TLayout::Context * ctx, lay_id earlier, lay_id later);
//
// Like lay_insert, but puts the new item as the first child in a parent instead of as the last.
//
LAY_EXPORT void lay_push(TLayout::Context * ctx, lay_id parent, lay_id child);
//
// Gets the size that was set with lay_set_size or lay_set_size_xy. The _xy
// version writes the output values to the specified addresses instead of
// returning the values in a lay_vec2.
//
//LAY_EXPORT lay_vec2 lay_get_size(TLayout::Context * ctx, lay_id item);
LAY_EXPORT void lay_get_size_xy(TLayout::Context * ctx, lay_id item, lay_scalar * x, lay_scalar * y);
//
// Sets the size of an item. The _xy version passes the width and height as
// separate arguments, but functions the same.
//
//LAY_EXPORT void lay_set_size(TLayout::Context * ctx, lay_id item, lay_vec2 size);
//LAY_EXPORT void lay_set_size_xy(TLayout::Context * ctx, lay_id item, lay_scalar width, lay_scalar height);
//
// Set the flags on an item which determines how it behaves as a parent. For
// example, setting LAY_COLUMN will make an item behave as if it were a column
// -- it will lay out its children vertically.
//
LAY_EXPORT void lay_set_contain(TLayout::Context * ctx, lay_id item, uint32 flags);
//
// Set the flags on an item which determines how it behaves as a child inside of
// a parent item. For example, setting LAY_VFILL will make an item try to fill
// up all available vertical space inside of its parent.
//
LAY_EXPORT void lay_set_behave(TLayout::Context * ctx, lay_id item, uint32 flags);
//
// Get the margins that were set by lay_set_margins. The _ltrb version writes
// the output values to the specified addresses instead of returning the values
// in a lay_vec4.
// l: left, t: top, r: right, b: bottom
//
LAY_EXPORT lay_vec4 lay_get_margins(TLayout::Context * ctx, lay_id item);
LAY_EXPORT void lay_get_margins_ltrb(TLayout::Context * ctx, lay_id item, lay_scalar * l, lay_scalar * t, lay_scalar * r, lay_scalar * b);
//
// Set the margins on an item. The components of the vector are:
// 0: left, 1: top, 2: right, 3: bottom.
//
LAY_EXPORT void lay_set_margins(TLayout::Context * ctx, lay_id item, lay_vec4 ltrb);
//
// Same as lay_set_margins, but the components are passed as separate arguments (left, top, right, bottom).
//
LAY_EXPORT void lay_set_margins_ltrb(TLayout::Context * ctx, lay_id item, lay_scalar l, lay_scalar t, lay_scalar r, lay_scalar b);
//
// Get the id of first child of an item, if any. Returns LAY_INVALID_ID if there is no child.
//
LAY_STATIC_INLINE lay_id lay_first_child(const TLayout::Context * ctx, lay_id id)
{
	const TLayout::Item * pitem = ctx->GetItemC(id);
	return pitem->FirstChild;
}
//
// Get the id of the next sibling of an item, if any. Returns LAY_INVALID_ID if there is no next sibling.
//
LAY_STATIC_INLINE lay_id lay_next_sibling(const TLayout::Context * ctx, lay_id id)
{
	const TLayout::Item * p_item = ctx->GetItemC(id);
	return p_item->NextSibling;
}
//
// Returns the calculated rectangle of an item. This is only valid after calling
// lay_run_context and before any other reallocation occurs. Otherwise, the
// result will be undefined. The vector components are:
// 0: x starting position, 1: y starting position, 2: width, 3: height
//
LAY_STATIC_INLINE lay_vec4 lay_get_rect(const TLayout::Context * ctx, lay_id id)
{
	assert(id != LAY_INVALID_ID && id < ctx->count);
	return ctx->P_Rects[id];
}
//
// The same as lay_get_rect, but writes the x,y positions and width,height
// values to the specified addresses instead of returning them in a lay_vec4.
//
LAY_STATIC_INLINE void lay_get_rect_xywh(const TLayout::Context * ctx, lay_id id, lay_scalar * x, lay_scalar * y, lay_scalar * width, lay_scalar * height)
{
	assert(id != LAY_INVALID_ID && id < ctx->count);
	lay_vec4 rect = ctx->P_Rects[id];
	*x = rect[0];
	*y = rect[1];
	*width = rect[2];
	*height = rect[3];
}

#undef LAY_EXPORT
#undef LAY_STATIC_INLINE

#endif // LAY_INCLUDE_HEADER
//
// Notes about the use of vector_size merely for syntax convenience:
//
// The current layout calculation procedures are not written in a way that
// would benefit from SIMD instruction usage.
//
// (Passing 128-bit float4 vectors using __vectorcall *might* get you some
// small benefit in very specific situations, but is unlikely to be worth the
// hassle. And I believe this would only be needed if you compiled the library
// in a way where the compiler was prevented from using inlining when copying
// rectangle/size data.)
//
// I might go back in the future and just use regular struct-wrapped arrays.
// I'm not sure if relying the vector thing in GCC/clang and then using C++
// operator overloading in MSVC is worth the annoyance of saving a couple of
// extra characters on each array access in the implementation code.
//
#ifdef LAY_IMPLEMENTATION

#include <stddef.h>
#include <stdbool.h>
#if 0 // {
// Users of this library can define LAY_REALLOC to use a custom (re)allocator
// instead of stdlib's realloc. It should have the same behavior as realloc --
// first parameter type is a void pointer, and its value is either a null
// pointer or an existing pointer. The second parameter is a size_t of the new
// desired size. The buffer contents should be preserved across reallocations.
//
// And, if you define LAY_REALLOC, you will also need to define LAY_FREE, which
// should have the same behavior as free.
#ifndef LAY_REALLOC
	//#include <stdlib.h>
	#define LAY_REALLOC(_block, _size) realloc(_block, _size)
	#define LAY_FREE(_block) free(_block)
#endif

// Like the LAY_REALLOC define, LAY_MEMSET can be used for a custom memset.
// Otherwise, the memset from string.h will be used.
#ifndef LAY_MEMSET
	//#include <string.h>
	#define LAY_MEMSET(_dst, _val, _size) memset(_dst, _val, _size)
#endif
#endif // } 0
#if defined(__GNUC__) || defined(__clang__)
	#define LAY_FORCE_INLINE __attribute__((always_inline)) inline
	#ifdef __cplusplus
		#define LAY_RESTRICT // @sobolev __restrict
	#else
		#define LAY_RESTRICT // @sobolev restrict
	#endif // __cplusplus
#elif defined(_MSC_VER)
	#define LAY_FORCE_INLINE __forceinline
	#define LAY_RESTRICT // @sobolev __restrict
#else
	#define LAY_FORCE_INLINE inline
	#ifdef __cplusplus
		#define LAY_RESTRICT
	#else
		#define LAY_RESTRICT // @sobolev restrict
	#endif // __cplusplus
#endif

#endif // LAY_IMPLEMENTATION
