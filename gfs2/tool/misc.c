/******************************************************************************
*******************************************************************************
**
**  Copyright (C) Sistina Software, Inc.  1997-2003  All rights reserved.
**  Copyright (C) 2004 Red Hat, Inc.  All rights reserved.
**
**  This copyrighted material is made available to anyone wishing to use,
**  modify, copy, or redistribute it subject to the terms and conditions
**  of the GNU General Public License v.2.
**
*******************************************************************************
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <errno.h>
#include <dirent.h>

#define __user
#include <linux/gfs2_ioctl.h>
#include <linux/gfs2_ondisk.h>

#include "gfs2_tool.h"

/**
 * do_file_flush - 
 * @argc:
 * @argv:
 *
 */

void
do_file_flush(int argc, char **argv)
{
	char *gi_argv[] = { "do_file_flush" };
	struct gfs2_ioctl gi;
	int fd;
	int error;

	if (optind == argc)
		die("Usage: gfs2_tool flush <filenames>\n");

	gi.gi_argc = 1;
	gi.gi_argv = gi_argv;

	for (; optind < argc; optind++) {
		fd = open(argv[optind], O_RDONLY);
		if (fd < 0)
			die("can't open %s: %s\n", argv[optind], strerror(errno));

		check_for_gfs2(fd, argv[optind]);

		error = ioctl(fd, GFS2_IOCTL_SUPER, &gi);
		if (error)
			die("error doing do_file_flush (%d): %s\n",
			    error, strerror(errno));

		close(fd);
	}
}

/**
 * do_freeze - freeze a GFS2 filesystem
 * @argc:
 * @argv:
 *
 */

void
do_freeze(int argc, char **argv)
{
	char *command = argv[optind - 1];
	char *name;

	if (optind == argc)
		die("Usage: gfs2_tool %s <mountpoint>\n", command);

	name = mp2fsname(argv[optind]);

	if (strcmp(command, "freeze") == 0)
		set_sysfs(name, "freeze", "1");
	else if (strcmp(command, "unfreeze") == 0)
		set_sysfs(name, "freeze", "0");

	sync();
}

/**
 * print_lockdump -
 * @argc:
 * @argv:
 *
 */

void
print_lockdump(int argc, char **argv)
{
	die("lockdump not implemented\n");
}

/**
 * margs -
 * @argc:
 * @argv:
 *
 */

void
margs(int argc, char **argv)
{
	die("margs not implemented\n");
}

/**
 * print_flags - print the flags in a dinode's di_flags field
 * @di: the dinode structure
 *
 */

static void
print_flags(struct gfs2_dinode *di)
{
	if (di->di_flags) {
		printf("Flags:\n");
		if (di->di_flags & GFS2_DIF_SYSTEM)
			printf("  system\n");
		if (di->di_flags & GFS2_DIF_JDATA)
			printf("  jdata\n");
		if (di->di_flags & GFS2_DIF_EXHASH)
			printf("  exhash\n");
		if (di->di_flags & GFS2_DIF_EA_INDIRECT)
			printf("  ea_indirect\n");
		if (di->di_flags & GFS2_DIF_DIRECTIO)
			printf("  directio\n");
		if (di->di_flags & GFS2_DIF_IMMUTABLE)
			printf("  immutable\n");
		if (di->di_flags & GFS2_DIF_APPENDONLY)
			printf("  appendonly\n");
		if (di->di_flags & GFS2_DIF_NOATIME)
			printf("  noatime\n");
		if (di->di_flags & GFS2_DIF_SYNC)
			printf("  sync\n");
		if (di->di_flags & GFS2_DIF_INHERIT_DIRECTIO)
			printf("  inherit_directio\n");
		if (di->di_flags & GFS2_DIF_INHERIT_JDATA)
			printf("  inherit_jdata\n");
		if (di->di_flags & GFS2_DIF_TRUNC_IN_PROG)
			printf("  trunc_in_prog\n");
	}
}

/**
 * set_flag - set or clear flags in some dinodes
 * @argc:
 * @argv:
 *
 */

void
set_flag(int argc, char **argv)
{
	struct gfs2_dinode di;
	char *set;
	char *flag;
	struct gfs2_ioctl gi;
	int fd;
	int error;

	if (optind == argc) {
		di.di_flags = 0xFFFFFFFF;
		print_flags(&di);
		return;
	}

	set = (strcmp(argv[optind - 1], "setflag") == 0) ? "set" : "clear";
	flag = argv[optind++];

	for (; optind < argc; optind++) {
		fd = open(argv[optind], O_RDONLY);
		if (fd < 0)
			die("can't open %s: %s\n", argv[optind], strerror(errno));

		check_for_gfs2(fd, argv[optind]);

		{
			char *gi_argv[] = { "set_file_flag",
					    set,
					    flag };
			gi.gi_argc = 3;
			gi.gi_argv = gi_argv;

			error = ioctl(fd, GFS2_IOCTL_SUPER, &gi);
			if (error)
				die("can't change flag on %s: %s\n", argv[optind], strerror(errno));
		}

		close(fd);
	}
}

/**
 * print_stat - print out the struct gfs2_dinode for a file
 * @argc:
 * @argv:
 *
 */

void
print_stat(int argc, char **argv)
{
	int fd;
	char *gi_argv[] = { "get_file_stat" };
	struct gfs2_ioctl gi;
	struct gfs2_dinode di;
	int error;

	if (optind == argc)
		die("Usage: gfs2_tool stat <filename>\n");

	fd = open(argv[optind], O_RDONLY);
	if (fd < 0)
		die("can't open %s: %s\n", argv[optind], strerror(errno));

	check_for_gfs2(fd, argv[optind]);

	gi.gi_argc = 1;
	gi.gi_argv = gi_argv;
	gi.gi_data = (char *)&di;
	gi.gi_size = sizeof(struct gfs2_dinode);

	error = ioctl(fd, GFS2_IOCTL_SUPER, &gi);
	if (error != gi.gi_size)
		die("error doing get_file_stat (%d): %s\n",
		    error, strerror(errno));

	close(fd);

	gfs2_dinode_print(&di);
	printf("\n");
	print_flags(&di);
}

/**
 * print_sb - the superblock
 * @argc:
 * @argv:
 *
 */

void
print_sb(int argc, char **argv)
{
	int fd;
	char *gi_argv[] = { "get_super" };
	struct gfs2_ioctl gi;
	struct gfs2_sb sb;
	int error;

	if (optind == argc)
		die("Usage: gfs2_tool getsb <mountpoint>\n");

	fd = open(argv[optind], O_RDONLY);
	if (fd < 0)
		die("can't open %s: %s\n", argv[optind], strerror(errno));
	
	check_for_gfs2(fd, argv[optind]);

	gi.gi_argc = 1;
	gi.gi_argv = gi_argv;
	gi.gi_data = (char *)&sb;
	gi.gi_size = sizeof(struct gfs2_sb);

	error = ioctl(fd, GFS2_IOCTL_SUPER, &gi);
	if (error != gi.gi_size)
		die("error doing get_super (%d): %s\n",
		    error, strerror(errno));

	close(fd);

	gfs2_sb_print(&sb);
}

/**
 * print_args -
 * @argc:
 * @argv:
 *
 */


void
print_args(int argc, char **argv)
{
	int fd;
	char *fs;
	DIR *d;
	struct dirent *de;
	char path[PATH_MAX];

	if (optind == argc)
		die("Usage: gfs2_tool getargs <mountpoint>\n");

	fd = open(argv[optind], O_RDONLY);
	if (fd < 0)
		die("can't open %s: %s\n", argv[optind], strerror(errno));
	
	check_for_gfs2(fd, argv[optind]);

	close(fd);
	fs = mp2fsname(argv[optind]);

	memset(path, 0, PATH_MAX);
	snprintf(path, PATH_MAX - 1, "%s/%s/args/", SYS_BASE, fs);

	d = opendir(path);
	if (!d)
		die("can't open %s: %s\n", path, strerror(errno));

	while((de = readdir(d))) {
		if (de->d_name[0] == '.')
			continue;

		snprintf(path, PATH_MAX - 1, "args/%s", de->d_name);
		printf("%s %s\n", de->d_name, get_sysfs(fs, path));
	}

	closedir(d);
	
}

/**
 * print_jindex - print out the journal index
 * @argc:
 * @argv:
 *
 */

void
print_jindex(int argc, char **argv)
{
	int fd;
	struct gfs2_ioctl gi;
	int error;


	if (optind == argc)
		die("Usage: gfs2_tool jindex <mountpoint>\n");

	fd = open(argv[optind], O_RDONLY);
	if (fd < 0)
		die("can't open %s: %s\n", argv[optind], strerror(errno));

	check_for_gfs2(fd, argv[optind]);


	{
		char *argv[] = { "get_hfile_stat",
				 "jindex" };
		struct gfs2_dinode di;

		gi.gi_argc = 2;
		gi.gi_argv = argv;
		gi.gi_data = (char *)&di;
		gi.gi_size = sizeof(struct gfs2_dinode);

		error = ioctl(fd, GFS2_IOCTL_SUPER, &gi);
		if (error != gi.gi_size)
			die("error doing get_hfile_stat (%d): %s\n",
			    error, strerror(errno));

		printf("Jindex\n");
		gfs2_dinode_print(&di);
	}


	close(fd);
}

/**
 * print_rindex - print out the journal index
 * @argc:
 * @argv:
 *
 */

void
print_rindex(int argc, char **argv)
{
	int fd;
	struct gfs2_ioctl gi;
	uint64_t offset;
	unsigned int x;
	int error;


	if (optind == argc)
		die("Usage: gfs2_tool rindex <mountpoint>\n");

	fd = open(argv[optind], O_RDONLY);
	if (fd < 0)
		die("can't open %s: %s\n", argv[optind], strerror(errno));

	check_for_gfs2(fd, argv[optind]);


	{
		char *argv[] = { "get_hfile_stat",
				 "rindex" };
		struct gfs2_dinode di;

		gi.gi_argc = 2;
		gi.gi_argv = argv;
		gi.gi_data = (char *)&di;
		gi.gi_size = sizeof(struct gfs2_dinode);

		error = ioctl(fd, GFS2_IOCTL_SUPER, &gi);
		if (error != gi.gi_size)
			die("error doing get_hfile_stat (%d): %s\n",
			    error, strerror(errno));

		gfs2_dinode_print(&di);
	}


	for (offset = 0, x = 0; ; offset += sizeof(struct gfs2_rindex), x++) {
		char *argv[] = { "do_hfile_read",
				 "rindex" };
		char buf[sizeof(struct gfs2_rindex)];
		struct gfs2_rindex ri;
		
		gi.gi_argc = 2;
		gi.gi_argv = argv;
		gi.gi_data = buf;
		gi.gi_size = sizeof(struct gfs2_rindex);
		gi.gi_offset = offset;

		error = ioctl(fd, GFS2_IOCTL_SUPER, &gi);
		if (!error)
			break;
		if (error != sizeof(struct gfs2_rindex))
			die("error doing do_hfile_read (%d): %s\n",
			    error, strerror(errno));

		gfs2_rindex_in(&ri, buf);

		printf("\nRG %u:\n\n", x);
		gfs2_rindex_print(&ri);
	}


	close(fd);
}

/**
 * print_quota - print out the quota file
 * @argc:
 * @argv:
 *
 */

void
print_quota(int argc, char **argv)
{
	int fd;
	struct gfs2_ioctl gi;
	uint64_t offset;
	unsigned int x;
	int error;


	if (optind == argc)
		die("Usage: gfs2_tool quota <mountpoint>\n");

	fd = open(argv[optind], O_RDONLY);
	if (fd < 0)
		die("can't open %s: %s\n", argv[optind], strerror(errno));

	check_for_gfs2(fd, argv[optind]);


	{
		char *argv[] = { "get_hfile_stat",
				 "quota" };
		struct gfs2_dinode di;

		gi.gi_argc = 2;
		gi.gi_argv = argv;
		gi.gi_data = (char *)&di;
		gi.gi_size = sizeof(struct gfs2_dinode);

		error = ioctl(fd, GFS2_IOCTL_SUPER, &gi);
		if (error != gi.gi_size)
			die("error doing get_hfile_stat (%d): %s\n",
			    error, strerror(errno));

		gfs2_dinode_print(&di);
	}


	for (offset = 0, x = 0; ; offset += sizeof(struct gfs2_quota), x++) {
		char *argv[] = { "do_hfile_read",
				 "quota" };
		char buf[sizeof(struct gfs2_quota)];
		struct gfs2_quota q;
		
		gi.gi_argc = 2;
		gi.gi_argv = argv;
		gi.gi_data = buf;
		gi.gi_size = sizeof(struct gfs2_quota);
		gi.gi_offset = offset;

		error = ioctl(fd, GFS2_IOCTL_SUPER, &gi);
		if (!error)
			break;
		if (error != sizeof(struct gfs2_quota))
			die("error doing do_hfile_read (%d): %s\n",
			    error, strerror(errno));

		gfs2_quota_in(&q, buf);

		if (q.qu_limit || q.qu_warn || q.qu_value) {
			printf("\nQuota %s %u:\n\n", (x & 1) ? "group" : "user", x >> 1);
			gfs2_quota_print(&q);
		}
	}


	close(fd);
}

/**
 * print_list - print the list of mounted filesystems
 *
 */

void
print_list(void)
{
	char *list = get_list();
	printf("%s", list);
}

/**
 * do_shrink - shrink the inode cache for a filesystem
 * @argc:
 * @argv:
 *
 */

void
do_shrink(int argc, char **argv)
{
	int fd;
	char *fs;

	if (optind == argc)
		die("Usage: gfs2_tool shrink <mountpoint>\n");

	fd = open(argv[optind], O_RDONLY);
	if (fd < 0)
		die("can't open %s: %s\n",
		    argv[optind], strerror(errno));

	check_for_gfs2(fd, argv[optind]);
	close(fd);
	fs = mp2fsname(argv[optind]);
	
	set_sysfs(fs, "shrink", "1");
}

/**
 * do_withdraw - withdraw a GFS2 filesystem
 * @argc:
 * @argv:
 *
 */

void
do_withdraw(int argc, char **argv)
{
	char *name;

	if (optind == argc)
		die("Usage: gfs2_tool withdraw <mountpoint>\n");

	name = mp2fsname(argv[optind]);

	set_sysfs(name, "withdraw", "1");
}

