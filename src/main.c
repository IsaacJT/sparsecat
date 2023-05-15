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
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdlib.h>

static int open_file(const char *filename, struct sparse_file **sparse);
static void close_file(int fd, struct sparse_file *sparse);
static void print_info(struct sparse_file *sparse);
static int chunk_cb(void *priv, const void *data, size_t len);
static void print_usage(void);

/**
 * Print some basic metadata about the sparse file
 *
 * @param sparse pointer to the sparse_file struct whose metadata should be
 *               printed out
 */
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

/**
 * Open the given file path and create a sparse_file struct
 *
 * @param file to open
 * @param sparse pointer to the sparse_file struct to be initialised
 *
 * @return 0 if successful, error code if not
 */
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

	return fd;
}

/**
 * Safely close the given sparse file referred
 *
 * @param fd open file descriptor that should be closed
 * @param sparse pointer to the sparse file struct representing the sparse file
 *               that fd points to
 */
static void close_file(int fd, struct sparse_file *sparse)
{
	if (sparse) {
		sparse_file_destroy(sparse);
	}
	close(fd);
}

/**
 * Function which will be called on each block of data inside the sparse file.
 * Adhers to the function prototype accepted by sparse_file_callback()
 *
 * @param priv unused, required by function prototype
 * @param data pointer to the data chunk inside the sparse image
 * @param len length of the data chunk
 *
 * @return 0 if successful, error code if not
 */
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

/**
 * Print the argument usage and copyright info
 */
static void print_usage(void)
{
	fprintf(stderr,
		"sparsecat: tool for extracting the data from an Android "
		"sparse image\n");
	fprintf(stderr, "Created by Isaac True <isaac@is.having.coffee>\n");
	fprintf(stderr, "Usage: sparsecat [-h] [-v] <filename>\n");
	fprintf(stderr, "  -v: verbose output\n");
	fprintf(stderr, "  -h: display this text and exit\n");
	fprintf(stderr, "  filename: path to the Android sparse image\n");
}

int main(int argc, char *argv[])
{
	struct sparse_file *sparse = NULL;
	int fd;
	int ret;
	int c;
	bool verbose = false;

	while ((c = getopt(argc, argv, "vh")) != -1) {
		switch (c) {
		case 'h':
			print_usage();
			return -1;
		case 'v':
			verbose = true;
			break;
		case '?':
			if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n",
					optopt);
			else
				fprintf(stderr,
					"Unknown option character `\\x%x'.\n",
					optopt);
			return 1;
		default:
			abort();
		}
	}

	if (isatty(fileno(stdout))) {
		print_usage();
		fprintf(stderr, "\nRefusing to write to stdout! "
				"(Hint: pipe this to another program)\n");
		return -EBADF;
	}

	fd = open_file(argv[1], &sparse);
	if (fd < 0) {
		fprintf(stderr, "Error opening sparse file: %s\n",
			strerror(-fd));
		return fd;
	}

	if (verbose) {
		fprintf(stderr, "File opened: %s\n", argv[1]);
		print_info(sparse);
	}

	ret = sparse_file_callback(sparse, false, false, chunk_cb, NULL);
	if (ret < 0) {
		fprintf(stderr, "Error reading sparse file: %s\n",
			strerror(-fd));
		return ret;
	}

	close_file(fd, sparse);

	return 0;
}
