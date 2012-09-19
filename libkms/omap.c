/**************************************************************************
 *
 * Copyright © 2009 VMware, Inc., Palo Alto, CA., USA
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


#ifndef HAVE_STDINT_H
#define HAVE_STDINT_H
#endif
#define _FILE_OFFSET_BITS 64

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"

#include <sys/mman.h>
#include <sys/ioctl.h>
#include "xf86drm.h"

#include <xf86drmMode.h>

#include "omap_drm.h"
#include "omap_drmif.h"

#ifndef PAGE_SHIFT
#  define PAGE_SHIFT 12
#endif

/* align x to next highest multiple of 2^n */
#define ALIGN2(x,n)   (((x) + ((1 << (n)) - 1)) & ~((1 << (n)) - 1))

#ifndef container_of
#define container_of(ptr, type, member) \
    (type *)((char *)(ptr) - (char *) &((type *)0)->member)
#endif

#define MSG(fmt, ...) \
		do { fprintf(stderr, fmt "\n", ##__VA_ARGS__); } while (0)
#define ERROR(fmt, ...) \
		do { fprintf(stderr, "ERROR:%s:%d: " fmt "\n", __func__, __LINE__, ##__VA_ARGS__); } while (0)

#define to_display_kms(x) container_of(x, struct display_kms, kms)
struct display_kms {
	struct kms_driver kms;
	struct omap_device *dev;
	uint32_t width, height;
	uint32_t bo_flags;
};

struct omap_kms_bo
{
	struct kms_bo base;
	struct omap_bo * omap_bo;
	unsigned map_count;
};

static int
omapkms_get_prop(struct kms_driver *kms, unsigned key, unsigned *out)
{
	switch (key) {
	case KMS_BO_TYPE:
		*out = KMS_BO_TYPE_SCANOUT_X8R8G8B8 | KMS_BO_TYPE_CURSOR_64X64_A8R8G8B8;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int
omapkms_bo_create(struct kms_driver *kms,
		 const unsigned width, const unsigned height,
		 const enum kms_bo_type type, const unsigned *attr,
		 struct kms_bo **out)
{
	unsigned size = 0, pitch = 0;
	struct omap_kms_bo *bo = NULL;
	struct drm_omap_gem_new arg;
	int i, ret;
	uint32_t bpp;
	struct display_kms *disp = to_display_kms(kms);
	uint32_t bo_flags = disp->bo_flags;

	for (i = 0; attr[i]; i += 2) {
		switch (attr[i]) {
			case KMS_BO_TYPE:
			case KMS_WIDTH:
			case KMS_HEIGHT:
			case KMS_PITCH:
			case KMS_HANDLE:
				break;
			default:
				return -EINVAL;
		}
	}

	bo = calloc(1, sizeof(*bo));
	if (!bo)
		return -ENOMEM;

	bpp = 32;

	if ((bo_flags & OMAP_BO_TILED) == OMAP_BO_TILED) {
		bo_flags &= ~OMAP_BO_TILED;
		if (bpp == 8) {
			bo_flags |= OMAP_BO_TILED_8;
		} else if (bpp == 16) {
			bo_flags |= OMAP_BO_TILED_16;
		} else if (bpp == 32) {
			bo_flags |= OMAP_BO_TILED_32;
		}
	}

	bo_flags |= OMAP_BO_WC;

	if (bo_flags & OMAP_BO_TILED) {
		bo->omap_bo = omap_bo_new_tiled(disp->dev, width, height, bo_flags);
	} else {
		bo->omap_bo = omap_bo_new(disp->dev, width * height * bpp / 8, bo_flags);
	}

	if (bo->omap_bo) {
		bo->base.handle = omap_bo_handle(bo->omap_bo);
		bo->base.pitch = width * bpp / 8;
		if (bo_flags & OMAP_BO_TILED)
			bo->base.pitch = ALIGN2(bo->base.pitch, PAGE_SHIFT);
	}

	bo->base.kms = kms;
	bo->base.size = bo->base.pitch * height;

	*out = &bo->base;

	return 0;

err_free:
	free(bo);
	return ret;
}

static int
omapkms_bo_get_prop(struct kms_bo *bo, unsigned key, unsigned *out)
{
	switch (key) {
	default:
		return -EINVAL;
	}
}

static int
omapkms_bo_map(struct kms_bo *_bo, void **out)
{
	struct omap_kms_bo *bo = (struct omap_kms_bo *)_bo;

	if(bo->base.ptr == NULL) {
		omap_bo_cpu_prep(bo->omap_bo, OMAP_GEM_WRITE /* | OMAP_GEM_READ */);
		bo->base.ptr = omap_bo_map(bo->omap_bo);
	}

	if (bo->base.ptr) {
		bo->map_count++;
	}

	*out = bo->base.ptr;

	return (bo->base.ptr) ? 0 : -1;
}

static int
omapkms_bo_unmap(struct kms_bo *_bo)
{
	struct omap_kms_bo *bo = (struct omap_kms_bo *)_bo;

	if(bo->map_count <= 0)
		return -1;

	bo->map_count--;

	if(bo->map_count == 0)
	{
		omap_bo_cpu_fini(bo->omap_bo, OMAP_GEM_WRITE /* | OMAP_GEM_READ */);
		bo->base.ptr = NULL;
	}

	return 0;
}

static int
omapkms_bo_destroy(struct kms_bo *_bo)
{
	struct omap_kms_bo *bo = (struct omap_kms_bo *)_bo;

	omap_bo_del(bo->omap_bo);

	free(bo);

	return 0;
}

static int
omapkms_destroy(struct kms_driver *kms)
{
	struct display_kms *disp = to_display_kms(kms);

	if(disp->dev)
	{
		omap_device_del(disp->dev);
		disp->dev = NULL;
	}

	free(disp);

	return 0;
}

int
omap_create(int fd, struct kms_driver **out)
{
	struct display_kms *disp = NULL;
	struct kms_driver *kms;

	disp = calloc(1, sizeof(*disp));
	if (!disp)
		return -ENOMEM;

	kms = &disp->kms;

	kms->fd = fd;

	kms->bo_create = omapkms_bo_create;
	kms->bo_map = omapkms_bo_map;
	kms->bo_unmap = omapkms_bo_unmap;
	kms->bo_get_prop = omapkms_bo_get_prop;
	kms->bo_destroy = omapkms_bo_destroy;
	kms->get_prop = omapkms_get_prop;
	kms->destroy = omapkms_destroy;
	*out = kms;

	disp->dev = omap_device_new(fd);
	if (!disp->dev) {
		ERROR("couldn't create device");
		goto fail;
	}

	disp->bo_flags = OMAP_BO_SCANOUT;

	return 0;

fail:
	free(disp);
	return -ENOMEM;
}
