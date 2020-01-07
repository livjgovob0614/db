/*------------------------------------------------------------------------
 *  Copyright 2007-2010 (c) Jeff Brown <spadix@users.sourceforge.net>
 *
 *  This file is part of the ZBar Bar Code Reader.
 *
 *  The ZBar Bar Code Reader is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU Lesser Public License as
 *  published by the Free Software Foundation; either version 2.1 of
 *  the License, or (at your option) any later version.
 *
 *  The ZBar Bar Code Reader is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with the ZBar Bar Code Reader; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301  USA
 *
 *  http://sourceforge.net/projects/zbar
 *------------------------------------------------------------------------*/

#include <zbar.h>
#pragma hdrstop
//#include "error.h"

#ifdef HAVE_LIBJPEG
extern struct jpeg_decompress_struct * _zbar_jpeg_decomp_create(void);
extern void _zbar_jpeg_decomp_destroy(struct jpeg_decompress_struct * cinfo);
#endif
//
// video.next_image and video.recycle_image have to be thread safe wrt/other apis
//
int video_lock(zbar_video_t * vdo)
{
	const int rc = _zbar_mutex_lock(&vdo->qlock);
	if(rc) {
		err_capture(vdo, SEV_FATAL, ZBAR_ERR_LOCKING, __func__, "unable to acquire lock");
		vdo->err.errnum = rc;
		return -1;
	}
	else
		return 0;
}

int video_unlock(zbar_video_t * vdo)
{
	const int rc = _zbar_mutex_unlock(&vdo->qlock);
	if(rc) {
		err_capture(vdo, SEV_FATAL, ZBAR_ERR_LOCKING, __func__, "unable to release lock");
		vdo->err.errnum = rc;
		return -1;
	}
	else
		return 0;
}

int video_nq_image(zbar_video_t * vdo, zbar_image_t * img)
{
	// maintains queued buffers in order 
	img->next = NULL;
	if(vdo->nq_image)
		vdo->nq_image->next = img;
	vdo->nq_image = img;
	if(!vdo->dq_image)
		vdo->dq_image = img;
	return video_unlock(vdo);
}

zbar_image_t * video_dq_image(zbar_video_t * vdo)
{
	zbar_image_t * img = vdo->dq_image;
	if(img) {
		vdo->dq_image = img->next;
		img->next = NULL;
	}
	if(video_unlock(vdo))
		return NULL; // FIXME reclaim image 
	return (img);
}

static void _zbar_video_recycle_image(zbar_image_t * img)
{
	zbar_video_t * vdo = img->src;
	assert(vdo);
	assert(img->srcidx >= 0);
	video_lock(vdo);
	if(vdo->images[img->srcidx] != img)
		vdo->images[img->srcidx] = img;
	if(vdo->active)
		vdo->nq(vdo, img);
	else
		video_unlock(vdo);
}

static void _zbar_video_recycle_shadow(zbar_image_t * img)
{
	zbar_video_t * vdo = img->src;
	assert(vdo);
	assert(img->srcidx == -1);
	video_lock(vdo);
	img->next = vdo->shadow_image;
	vdo->shadow_image = img;
	video_unlock(vdo);
}

zbar_video_t * zbar_video_create()
{
	zbar_video_t * vdo = (zbar_video_t *)SAlloc::C(1, sizeof(zbar_video_t));
	if(vdo) {
		err_init(&vdo->err, ZBAR_MOD_VIDEO);
		vdo->fd = -1;
		(void)_zbar_mutex_init(&vdo->qlock);
		/* pre-allocate images */
		vdo->num_images = ZBAR_VIDEO_IMAGES_MAX;
		vdo->images = (zbar_image_t **)SAlloc::C(ZBAR_VIDEO_IMAGES_MAX, sizeof(zbar_image_t *));
		if(!vdo->images) {
			zbar_video_destroy(vdo);
			return NULL;
		}
		else {
			for(int i = 0; i < ZBAR_VIDEO_IMAGES_MAX; i++) {
				zbar_image_t * img = vdo->images[i] = zbar_image_create();
				if(!img) {
					zbar_video_destroy(vdo);
					return NULL;
				}
				img->refcnt = 0;
				img->cleanup = _zbar_video_recycle_image;
				img->srcidx = i;
				img->src = vdo;
			}
		}
	}
	return vdo;
}

void zbar_video_destroy(zbar_video_t * vdo)
{
	if(vdo->intf != VIDEO_INVALID)
		zbar_video_open(vdo, 0);
	if(vdo->images) {
		for(int i = 0; i < ZBAR_VIDEO_IMAGES_MAX; i++) {
			_zbar_image_free(vdo->images[i]);
		}
		SAlloc::F(vdo->images);
	}
	while(vdo->shadow_image) {
		zbar_image_t * img = vdo->shadow_image;
		vdo->shadow_image = img->next;
		SAlloc::F((void *)img->P_Data);
		img->P_Data = NULL;
		SAlloc::F(img);
	}
	SAlloc::F(vdo->buf);
	SAlloc::F(vdo->formats);
	err_cleanup(&vdo->err);
	_zbar_mutex_destroy(&vdo->qlock);

#ifdef HAVE_LIBJPEG
	if(vdo->jpeg_img) {
		zbar_image_destroy(vdo->jpeg_img);
		vdo->jpeg_img = NULL;
	}
	if(vdo->jpeg) {
		_zbar_jpeg_decomp_destroy(vdo->jpeg);
		vdo->jpeg = NULL;
	}
#endif
	SAlloc::F(vdo);
}

int zbar_video_open(zbar_video_t * vdo, const char * dev)
{
	char * ldev = NULL;
	int rc;
	zbar_video_enable(vdo, 0);
	video_lock(vdo);
	if(vdo->intf != VIDEO_INVALID) {
		if(vdo->cleanup) {
			vdo->cleanup(vdo);
			vdo->cleanup = NULL;
		}
		zprintf(1, "closed camera (fd=%d)\n", vdo->fd);
		vdo->intf = VIDEO_INVALID;
	}
	video_unlock(vdo);
	if(!dev)
		return 0;
	if((uchar)dev[0] < 0x10) {
		/* default linux device, overloaded for other platforms */
		int id = dev[0];
		dev = ldev = _strdup("/dev/video0");
		ldev[10] = '0' + id;
	}
	rc = _zbar_video_open(vdo, dev);
	SAlloc::F(ldev);
	return (rc);
}

int zbar_video_get_fd(const zbar_video_t * vdo)
{
	if(vdo->intf == VIDEO_INVALID)
		return err_capture(vdo, SEV_ERROR, ZBAR_ERR_INVALID, __func__, "video device not opened");
	if(vdo->intf != VIDEO_V4L2)
		return err_capture(vdo, SEV_WARNING, ZBAR_ERR_UNSUPPORTED, __func__, "video driver does not support polling");
	return vdo->fd;
}

int zbar_video_request_size(zbar_video_t * vdo, uint width, uint height)
{
	if(vdo->initialized)
		/* FIXME re-init different format? */
		return err_capture(vdo, SEV_ERROR, ZBAR_ERR_INVALID, __func__, "already initialized, unable to resize");
	vdo->width = width;
	vdo->height = height;
	zprintf(1, "request size: %d x %d\n", width, height);
	return 0;
}

int zbar_video_request_interface(zbar_video_t * vdo, int ver)
{
	if(vdo->intf != VIDEO_INVALID)
		return err_capture(vdo, SEV_ERROR, ZBAR_ERR_INVALID, __func__, "device already opened, unable to change interface");
	vdo->intf = (video_interface_t)ver;
	zprintf(1, "request interface version %d\n", vdo->intf);
	return 0;
}

int zbar_video_request_iomode(zbar_video_t * vdo, int iomode)
{
	if(vdo->intf != VIDEO_INVALID)
		return err_capture(vdo, SEV_ERROR, ZBAR_ERR_INVALID, __func__, "device already opened, unable to change iomode");
	if(iomode < 0 || iomode > VIDEO_USERPTR)
		return err_capture(vdo, SEV_ERROR, ZBAR_ERR_INVALID, __func__, "invalid iomode requested");
	vdo->iomode = (video_iomode_t)iomode;
	return 0;
}

int zbar_video_get_width(const zbar_video_t * vdo)
{
	return (vdo->width);
}

int zbar_video_get_height(const zbar_video_t * vdo)
{
	return (vdo->height);
}

uint32 zbar_video_get_format(const zbar_video_t * vdo)
{
	return (vdo->format);
}

static inline int video_init_images(zbar_video_t * vdo)
{
	int i;
	assert(vdo->datalen);
	if(vdo->iomode != VIDEO_MMAP) {
		assert(!vdo->buf);
		vdo->buflen = vdo->num_images * vdo->datalen;
		vdo->buf = SAlloc::C(1, vdo->buflen);
		if(!vdo->buf)
			return err_capture(vdo, SEV_FATAL, ZBAR_ERR_NOMEM, __func__, "unable to allocate image buffers");
		zprintf(1, "pre-allocated %d %s buffers size=0x%lx\n", vdo->num_images, (vdo->iomode == VIDEO_READWRITE) ? "READ" : "USERPTR", vdo->buflen);
	}
	for(i = 0; i < vdo->num_images; i++) {
		zbar_image_t * img = vdo->images[i];
		img->Format = vdo->format;
		zbar_image_set_size(img, vdo->width, vdo->height);
		if(vdo->iomode != VIDEO_MMAP) {
			ulong offset = i * vdo->datalen;
			img->datalen = vdo->datalen;
			img->P_Data = (uint8 *)vdo->buf + offset;
			zprintf(2, "    [%02d] @%08lx\n", i, offset);
		}
	}
	return 0;
}

int zbar_video_init(zbar_video_t * vdo,
    ulong fmt)
{
#ifdef HAVE_LIBJPEG
	const zbar_format_def_t * vidfmt;
#endif
	if(vdo->initialized)
		/* FIXME re-init different format? */
		return err_capture(vdo, SEV_ERROR, ZBAR_ERR_INVALID, __func__, "already initialized, re-init unimplemented");
	if(vdo->init(vdo, fmt))
		return -1;
	vdo->format = fmt;
	if(video_init_images(vdo))
		return -1;
#ifdef HAVE_LIBJPEG
	vidfmt = _zbar_format_lookup(fmt);
	if(vidfmt && vidfmt->group == ZBAR_FMT_JPEG) {
		zbar_image_t * img;
		/* prepare for decoding */
		if(!vdo->jpeg)
			vdo->jpeg = _zbar_jpeg_decomp_create();
		if(vdo->jpeg_img)
			zbar_image_destroy(vdo->jpeg_img);

		/* create intermediate image for decoder to use*/
		img = vdo->jpeg_img = zbar_image_create();
		img->Format = fourcc('Y', '8', '0', '0');
		zbar_image_set_size(img, vdo->width, vdo->height);
		img->datalen = vdo->width * vdo->height;
	}
#endif
	vdo->initialized = 1;
	return 0;
}

int zbar_video_enable(zbar_video_t * vdo, int enable)
{
	if(vdo->active == enable)
		return 0;
	if(enable) {
		if(vdo->intf == VIDEO_INVALID)
			return err_capture(vdo, SEV_ERROR, ZBAR_ERR_INVALID, __func__, "video device not opened");
		if(!vdo->initialized && zbar_negotiate_format(vdo, NULL))
			return -1;
	}
	if(video_lock(vdo))
		return -1;
	vdo->active = enable;
	if(enable) {
		/* enqueue all buffers */
		int i;
		for(i = 0; i < vdo->num_images; i++)
			if(vdo->nq(vdo, vdo->images[i]) || ((i + 1 < vdo->num_images) && video_lock(vdo)))
				return -1;
		return (vdo->start(vdo));
	}
	else {
		int i;
		for(i = 0; i < vdo->num_images; i++)
			vdo->images[i]->next = NULL;
		vdo->nq_image = vdo->dq_image = NULL;
		if(video_unlock(vdo))
			return -1;
		return (vdo->stop(vdo));
	}
}

zbar_image_t * zbar_video_next_image(zbar_video_t * vdo)
{
	uint frame;
	zbar_image_t * img;
	if(video_lock(vdo))
		return NULL;
	if(!vdo->active) {
		video_unlock(vdo);
		return NULL;
	}
	frame = vdo->frame++;
	img = vdo->dq(vdo);
	if(img) {
		img->seq = frame;
		if(vdo->num_images < 2) {
			/* return a *copy* of the video image and immediately recycle
			 * the driver's buffer to avoid deadlocking the resources
			 */
			zbar_image_t * tmp = img;
			video_lock(vdo);
			img = vdo->shadow_image;
			vdo->shadow_image = (img) ? img->next : NULL;
			video_unlock(vdo);
			if(!img) {
				img = zbar_image_create();
				assert(img);
				img->refcnt = 0;
				img->src = vdo;
				// recycle the shadow images 
				img->Format = vdo->format;
				zbar_image_set_size(img, vdo->width, vdo->height);
				img->datalen = vdo->datalen;
				img->P_Data = SAlloc::M(vdo->datalen);
			}
			img->cleanup = _zbar_video_recycle_shadow;
			img->seq = frame;
			memcpy((void *)img->P_Data, tmp->P_Data, img->datalen);
			_zbar_video_recycle_image(tmp);
		}
		else
			img->cleanup = _zbar_video_recycle_image;
		_zbar_image_refcnt(img, 1);
	}
	return (img);
}

