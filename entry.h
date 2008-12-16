#ifndef _ENTRY_H
#define _ENTRY_H

/* almost the whole stat structure... */
struct r_entry {
	char plusmin;	        /* '-' remove, '+' add. Added because of rdup-tr */
        char *f_name;		/* filename, or hardlink target */
        size_t f_name_size;	/* size of filename */
	guint f_lnk;		/* 0, 1 if hardlink */
        uid_t f_uid;		/* uid */
        gid_t f_gid;		/* gid */
        mode_t f_mode;		/* mode bits */
        time_t f_ctime;		/* change time of the inode */
        off_t f_size;		/* file size */
	dev_t f_dev;		/* ID of device containing file */
	dev_t f_rdev;		/* device ID (if special file), we use this for major, minor */
	ino_t f_ino;		/* inode number */
};

#endif  /* _ENTRY_H */
