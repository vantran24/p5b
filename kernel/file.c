#include "types.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "buf.h"
#include "file.h"
#include "spinlock.h"
#include "stat.h"

struct devsw devsw[NDEV];
struct {
	struct spinlock lock;
	struct file file[NFILE];
} ftable;

void
fileinit(void)
{
	initlock(&ftable.lock, "ftable");
}

// Allocate a file structure.
struct file*
filealloc(void)
{
	struct file *f;

	acquire(&ftable.lock);
	for(f = ftable.file; f < ftable.file + NFILE; f++){
		if(f->ref == 0){
			f->ref = 1;
			release(&ftable.lock);
			return f;
		}
	}
	release(&ftable.lock);
	return 0;
}

// Increment ref count for file f.
struct file*
filedup(struct file *f)
{
	acquire(&ftable.lock);
	if(f->ref < 1)
		panic("filedup");
	f->ref++;
	release(&ftable.lock);
	return f;
}

// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f)
{
	struct file ff;

	acquire(&ftable.lock);
	if(f->ref < 1)
		panic("fileclose");
	if(--f->ref > 0){
		release(&ftable.lock);
		return;
	}
	ff = *f;
	f->ref = 0;
	f->type = FD_NONE;
	release(&ftable.lock);

	if(ff.type == FD_PIPE)
		pipeclose(ff.pipe, ff.writable);
	else if(ff.type == FD_INODE)
		iput(ff.ip);
}

// Get metadata about file f.
int
filestat(struct file *f, struct stat *st)
{
	uint adrbitmask = 0x00ffffff;
	uint csbitmask = 0xff000000;
	if(f->type == FD_INODE){
		ilock(f->ip);
		stati(f->ip, st);
		uchar checksum = (csbitmask&(f->ip->addrs[0]) >> 24);
		if (f->ip->type == T_CHECKED){
			int i;
			for (i = 1; i < NDIRECT; i++){
				//direct checksums
				checksum^=((csbitmask&(f->ip->addrs[i])) >> 24);
			}
			//need to make a buffer so we can do a bread
			struct buf* buff;
			buff = bread(f->ip->dev, (adrbitmask & f->ip->addrs[NDIRECT]));
			//^like a indirect case from bmap
			//create reference for data access to XOR all checksums
			uint* dref = (uint*)buff->data;
			for (i = 0; i < NINDIRECT; i++){
							//indirect blocks
				//indirect checksums
				checksum^=((csbitmask & dref[i]) >> 24);
			}
			st->checksum = checksum;
		}
		iunlock(f->ip);

		return 0;
	}
	return -1;
}

// Read from file f.  Addr is kernel address.
int
fileread(struct file *f, char *addr, int n)
{
	int r;

	if(f->readable == 0)
		return -1;
	if(f->type == FD_PIPE)
		return piperead(f->pipe, addr, n);
	if(f->type == FD_INODE){
		ilock(f->ip);
		if((r = readi(f->ip, addr, f->off, n)) > 0)
			f->off += r;
		iunlock(f->ip);
		return r;
	}
	panic("fileread");
}

// Write to file f.  Addr is kernel address.
int
filewrite(struct file *f, char *addr, int n)
{
	int r;

	if(f->writable == 0)
		return -1;
	if(f->type == FD_PIPE)
		return pipewrite(f->pipe, addr, n);
	if(f->type == FD_INODE){
		ilock(f->ip);
		if((r = writei(f->ip, addr, f->off, n)) > 0)
			f->off += r;
		iunlock(f->ip);
		return r;
	}
	panic("filewrite");
}

