#ifdef PIXMAN_FB_ACCESSORS

#define READ(img, ptr)      (((bits_image_t *)(img))->read_func ((ptr), sizeof(*(ptr))))
#define WRITE(img, ptr,val) (((bits_image_t *)(img))->write_func ((ptr), (val), sizeof (*(ptr))))

#define MEMSET_WRAPPED(img, dst, val, size)				\
	do {								\
		uint8_t *_dst = reinterpret_cast<uint8_t *>(dst);				\
		for(size_t _i = 0; _i < (size_t) size; _i++) {				\
			WRITE((img), _dst +_i, (val));				\
		}								\
	} while(0)

#else
	#define READ(img, ptr)		(*(ptr))
	#define WRITE(img, ptr, val)	(*(ptr) = (val))
	#define MEMSET_WRAPPED(img, dst, val, size) memset(dst, val, size)
#endif

