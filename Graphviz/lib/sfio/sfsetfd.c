/* $Id: sfsetfd.c,v 1.1.1.1 2004/12/23 04:04:14 ellson Exp $ $Revision: 1.1.1.1 $ */
/* vim:set shiftwidth=4 ts=8: */

/**********************************************************
*      This software is part of the graphviz package      *
*                http://www.graphviz.org/                 *
*                                                         *
*            Copyright (c) 1994-2004 AT&T Corp.           *
*                and is licensed under the                *
*            Common Public License, Version 1.0           *
*                      by AT&T Corp.                      *
*                                                         *
*        Information and Software Systems Research        *
*              AT&T Research, Florham Park NJ             *
**********************************************************/

#include	"sfhdr.h"

/*	Change the file descriptor
**
**	Written by Kiem-Phong Vo.
*/

#if __STD_C
static int _sfdup(reg int fd, reg int newfd)
#else
static int _sfdup(fd, newfd)
reg int fd;
reg int newfd;
#endif
{
    reg int dupfd;

#ifdef F_DUPFD			/* the simple case */
    while ((dupfd = fcntl(fd, F_DUPFD, newfd)) < 0 && errno == EINTR)
	errno = 0;
    return dupfd;

#else				/* do it the hard way */
    if ((dupfd = dup(fd)) < 0 || dupfd >= newfd)
	return dupfd;

    /* dup() succeeded but didn't get the right number, recurse */
    newfd = _sfdup(fd, newfd);

    /* close the one that didn't match */
    CLOSE(dupfd);

    return newfd;
#endif
}

#if __STD_C
int sfsetfd(reg Sfio_t * f, reg int newfd)
#else
int sfsetfd(f, newfd)
reg Sfio_t *f;
reg int newfd;
#endif
{
    reg int oldfd;

    SFMTXSTART(f, -1);

    if (f->flags & SF_STRING)
	SFMTXRETURN(f, -1);

    if ((f->mode & SF_INIT) && f->file < 0) {	/* restoring file descriptor after a previous freeze */
	if (newfd < 0)
	    SFMTXRETURN(f, -1);
    } else {			/* change file descriptor */
	if ((f->mode & SF_RDWR) != f->mode && _sfmode(f, 0, 0) < 0)
	    SFMTXRETURN(f, -1);
	SFLOCK(f, 0);

	oldfd = f->file;
	if (oldfd >= 0) {
	    if (newfd >= 0) {
		if ((newfd = _sfdup(oldfd, newfd)) < 0) {
		    SFOPEN(f, 0);
		    SFMTXRETURN(f, -1);
		}
		CLOSE(oldfd);
	    } else {		/* sync stream if necessary */
		if (((f->mode & SF_WRITE) && f->next > f->data) ||
		    (f->mode & SF_READ) || f->disc == _Sfudisc) {
		    if (SFSYNC(f) < 0) {
			SFOPEN(f, 0);
			SFMTXRETURN(f, -1);
		    }
		}

		if (((f->mode & SF_WRITE) && f->next > f->data) ||
		    ((f->mode & SF_READ) && f->extent < 0 &&
		     f->next < f->endb)) {
		    SFOPEN(f, 0);
		    SFMTXRETURN(f, -1);
		}
#ifdef MAP_TYPE
		if ((f->bits & SF_MMAP) && f->data) {
		    SFMUNMAP(f, f->data, f->endb - f->data);
		    f->data = NIL(uchar *);
		}
#endif

		/* make stream appears uninitialized */
		f->endb = f->endr = f->endw = f->data;
		f->extent = f->here = 0;
		f->mode = (f->mode & SF_RDWR) | SF_INIT;
		f->bits &= ~SF_NULL;	/* off /dev/null handling */
	    }
	}

	SFOPEN(f, 0);
    }

    /* notify changes */
    if (_Sfnotify)
	(*_Sfnotify) (f, SF_SETFD, newfd);

    f->file = newfd;

    SFMTXRETURN(f, newfd);
}
