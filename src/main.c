/*
 * sparsecat: tool for extracting the data from an Android sparse image
 * Copyright (C) 2023 Isaac True <isaac@is.having.coffee>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <android/sparse/sparse.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

static int open_file(const char *filename, struct sparse_file **sparse);
static void close_file(int fd, struct sparse_file *sparse);
static void print_info(struct sparse_file *sparse);
static int chunk_cb(void *priv, const void *data, size_t len);

static void print_info(struct sparse_file *sparse)
{
	assert(sparse);

	fprintf(stderr, "Sparse file size:    \t%.2f MiB\n",
		sparse_file_len(sparse, true, true) / 1024.0 / 1024.0);
	fprintf(stderr, "Extracted image size:\t%.2f MiB\n",
		sparse_file_len(sparse, false, true) / 1024.0 / 1024.0);
	fprintf(stderr, "Block size:          \t%d\n",
		sparse_file_block_size(sparse));
}

static int open_file(const char *filename, struct sparse_file **sparse)
{
	int fd = 0;
	struct sparse_file *_sparse;

	assert(filename);
	assert(sparse);

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		return -errno;
	}

	_sparse = sparse_file_import(fd, false, true);
	if (!_sparse) {
		fprintf(stderr, "Failed to open sparse file\n");
		return -EIO;
	}

	*sparse = _sparse;

	fprintf(stderr, "File opened: %s\n", filename);

	return fd;
}

static void close_file(int fd, struct sparse_file *sparse)
{
	if (sparse) {
		sparse_file_destroy(sparse);
	}
	close(fd);
}

static int chunk_cb(void *priv __attribute__((unused)), const void *data,
		    size_t len)
{
	assert(data);

	ssize_t ret = write(fileno(stdout), data, len);
	if (ret == -1) {
		fprintf(stderr, "Failed to write to stdout: %s\n",
			strerror(-errno));
		return errno;
	} else if ((size_t)ret != len) {
		fprintf(stderr, "Could only write %ld bytes\n", ret);
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	struct sparse_file *sparse = NULL;
	int fd;
	int ret;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <sparse file>\n", argv[0]);
		return -EINVAL;
	}

	if (isatty(fileno(stdout))) {
		fprintf(stderr, "Refusing to write to stdout! "
				"(Hint: pipe this to another program)\n");
		return -EBADF;
	}

	fd = open_file(argv[1], &sparse);
	if (fd < 0) {
		fprintf(stderr, "Error opening sparse file: %s\n",
			strerror(-fd));
		return fd;
	}

	print_info(sparse);

	ret = sparse_file_callback(sparse, false, false, chunk_cb, NULL);
	if (ret < 0) {
		fprintf(stderr, "Error reading sparse file: %s\n",
			strerror(-fd));
		return ret;
	}

	close_file(fd, sparse);

	return 0;
}
