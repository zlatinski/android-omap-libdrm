/*
 * Copyright (C) 2011 Texas Instruments
 * Author: Rob Clark <rob.clark@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "util/util.h"

#define NBUF 3
#define NVBUF 3
#define CNT  500

static void
usage(char *name)
{
	MSG("Usage: %s [OPTION]...", name);
	MSG("Simple page-flip test, similar to 'modetest' but with option to use tiled buffers.");
	MSG("");
	disp_usage();
}

int
main(int argc, char **argv)
{
	struct display *disp;
	struct buffer **buffers;
	struct buffer **vid_buffers;
	struct buffer **vid_buffers2;
	int ret, i;
	bool video_buff = 1;
	// uint32_t fourcc = FOURCC('A','R','2','4');
	uint32_t fourcc = FOURCC('N','V','1','2');
	// uint32_t fourcc = FOURCC('Y','U','Y','V');
	uint32_t width = 200, height = 200;
	uint32_t x = 0, y = 0;

	MSG("Opening Display..");
	disp = disp_open(argc, argv);
	if (!disp) {
		usage(argv[0]);
		return 1;
	}

	if (check_args(argc, argv)) {
		/* remaining args.. print usage msg */
		usage(argv[0]);
		return 0;
	}

	if(1)
	{
		buffers = disp_get_buffers(disp, NBUF);
		if (!buffers) {
			return 1;
		}
	}

	if(1)
	{
		vid_buffers = disp_get_vid_buffers(disp, NVBUF,
				fourcc, width, height);
		if (!vid_buffers) {
			return 1;
		}
	}

	if(1)
	{
		vid_buffers2 = disp_get_vid_buffers(disp, NVBUF,
				fourcc, width, height);
		if (!vid_buffers2) {
			return 1;
		}
	}

	for (i = 0; i < CNT; i++) {

		if(1)
		{
			struct buffer *buf = buffers[i % NBUF];
			fill(buf, i * 2);
			ret = disp_post_buffer(disp, buf);
			if (ret) {
				return ret;
			}
		}

		if(1)
		{
			struct buffer *vbuf = vid_buffers[i % NVBUF];
			fill(vbuf, i * 2);
			ret = disp_post_vid_buffer(disp, vbuf,
					x,y, width, height,
					x,y, width, height);
			if (ret) {
				return ret;
			}
		}

		if(1)
		{
			struct buffer *vbuf2 = vid_buffers2[i % NVBUF];
			fill(vbuf2, i * 2);
			ret = disp_post_vid_buffer(disp, vbuf2,
					x + 300,y + 300, width, height,
					x,y, width, height);
			if (ret) {
				return ret;
			}
		}
	}

	MSG("Ok!");

	return 0;
}
