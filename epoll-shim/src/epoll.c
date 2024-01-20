#include <sys/epoll.h>

#include <sys/event.h>
#include <sys/stat.h>

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <time.h>

#define EV_FORCEONESHOT 0x0100

int
epoll_create(int size)
{
	return kqueue();
}

// int
// epoll_create1(int flags)
// {
// 	if (flags != EPOLL_CLOEXEC) {
// 		errno = EINVAL;
// 		return -1;
// 	}
//
// 	return kqueue();
// }

#define KEY_BITS (20)
#define VAL_BITS (32 - KEY_BITS)
static int
kqueue_save_state(int kq, uint32_t key, uint16_t val)
{
	struct kevent kev[VAL_BITS * 2];
	int n = 0;
	int i;
	int oe, e;

	if ((key & ~(((uint32_t)1 << KEY_BITS) - 1)) ||
	    (val & ~(((uint16_t)1 << VAL_BITS) - 1))) {
		return (-EINVAL);
	}

	for (i = 0; i < VAL_BITS; ++i) {
		uint32_t info_bit = (uint32_t)1 << i;
		uint32_t kev_key = key | (info_bit << KEY_BITS);
		EV_SET(&kev[n], kev_key, EVFILT_USER, EV_ADD, 0, 0, 0);
		++n;
		if (!(val & info_bit)) {
			EV_SET(&kev[n], kev_key, EVFILT_USER, /**/
			    EV_DELETE, 0, 0, 0);
			++n;
		}
	}

	oe = errno;
	if ((n = kevent(kq, kev, n, NULL, 0, NULL)) < 0) {
		e = errno;
		errno = oe;
		return (-e);
	}

	return (0);
}

static int
kqueue_load_state(int kq, uint32_t key, uint16_t *val)
{
	struct kevent kev[VAL_BITS];
	int n = 0;
	int i;
	uint16_t nval = 0;
	int oe, e;

	if ((key & ~(((uint32_t)1 << KEY_BITS) - 1))) {
		return (-EINVAL);
	}

	for (i = 0; i < VAL_BITS; ++i) {
		uint32_t info_bit = (uint32_t)1 << i;
		uint32_t kev_key = key | (info_bit << KEY_BITS);
		EV_SET(&kev[i], kev_key, EVFILT_USER, EV_RECEIPT, 0, 0, 0);
	}

	oe = errno;
	if ((n = kevent(kq, kev, VAL_BITS, kev, VAL_BITS, NULL)) < 0) {
		e = errno;
		errno = oe;
		return (-e);
	}

	for (i = 0; i < n; ++i) {
		if (!(kev[i].flags & EV_ERROR)) {
			return (-EINVAL);
		}

		if (kev[i].data == 0) {
			nval |= (uint32_t)1 << i;
		} else if (kev[i].data != ENOENT) {
			return (-EINVAL);
		}
	}

	*val = nval;

	return (0);
}

#define KQUEUE_STATE_REGISTERED 0x1u
#define KQUEUE_STATE_EPOLLIN 0x2u
#define KQUEUE_STATE_EPOLLOUT 0x4u
#define KQUEUE_STATE_EPOLLRDHUP 0x8u
#define KQUEUE_STATE_EPOLLPRI 0x10u
#define KQUEUE_STATE_NYCSS 0x20u
#define KQUEUE_STATE_ISFIFO 0x40u
#define KQUEUE_STATE_ISSOCK 0x80u

int
epoll_ctl(int fd, int op, int fd2, struct epoll_event *ev)
{
	struct kevent kev[2];
	uint16_t flags;
	int e;

	if ((!ev && op != EPOLL_CTL_DEL) ||
	    (ev &&
		((ev->events &
		    ~(EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLRDHUP | EPOLLERR | EPOLLPRI | EPOLLET))
		    /* the user should really set one of EPOLLIN or EPOLLOUT
		     * so that EPOLLHUP and EPOLLERR work. Don't make this a
		     * hard error for now, though. */
		    /* || !(ev->events & (EPOLLIN | EPOLLOUT)) */))) {
		errno = EINVAL;
		return (-1);
	}

	if (fd2 < 0 || ((uint32_t)fd2 & ~(((uint32_t)1 << KEY_BITS) - 1))) {
		errno = EBADF;
		return (-1);
	}

	if ((e = kqueue_load_state(fd, fd2, &flags)) < 0) {
		errno = e;
		return (-1);
	}

	if (op == EPOLL_CTL_ADD) {
                /**
                 * HACK(mrarm): Allow "reregistration". When fds are close()'d, we do not have any means of detecting
                 * that with kqueue (we could takeover the close call in hybris, and I perhaps should consider doing
                 * that?). As fds are reused on Unix, it is possible that later another fd is opened with the same ID,
                 * but we still have the flags set for the previous one.
                 * This shouldn't have any significant side effects because the flags are reset in this function.
		if (flags & KQUEUE_STATE_REGISTERED) {
			errno = EEXIST;
			return (-1);
		}
                */

		int extra = (ev->events & EPOLLET ? EV_CLEAR : 0);
		EV_SET(&kev[0], fd2, EVFILT_READ,
		    EV_ADD | (ev->events & EPOLLIN ? 0 : EV_DISABLE) | extra, 0, 0,
		    ev->data.ptr);
		EV_SET(&kev[1], fd2, EVFILT_WRITE,
		    EV_ADD | (ev->events & EPOLLOUT ? 0 : EV_DISABLE) | extra, 0, 0,
		    ev->data.ptr);

		flags = KQUEUE_STATE_REGISTERED;

#define SET_FLAG(flag)                                                        \
	do {                                                                  \
		if (ev->events & (flag)) {                                    \
			flags |= KQUEUE_STATE_##flag;                         \
		}                                                             \
	} while (0)

		SET_FLAG(EPOLLIN);
		SET_FLAG(EPOLLOUT);
		SET_FLAG(EPOLLRDHUP);
		SET_FLAG(EPOLLPRI);

#undef SET_FLAG

	} else if (op == EPOLL_CTL_DEL) {
		if (!(flags & KQUEUE_STATE_REGISTERED)) {
			errno = ENOENT;
			return (-1);
		}

		EV_SET(&kev[0], fd2, EVFILT_READ, EV_DELETE, 0, 0, 0);
		EV_SET(&kev[1], fd2, EVFILT_WRITE, EV_DELETE, 0, 0, 0);

		flags = 0;
	} else if (op == EPOLL_CTL_MOD) {
		if (!(flags & KQUEUE_STATE_REGISTERED)) {
			errno = ENOENT;
			return (-1);
		}

		int extra = (ev->events & EPOLLET ? EV_CLEAR : 0);
		EV_SET(&kev[0], fd2, EVFILT_READ,
		    (ev->events & EPOLLIN ? EV_ENABLE : EV_DISABLE) | extra, 0, 0,
		    ev->data.ptr);
		EV_SET(&kev[1], fd2, EVFILT_WRITE,
		    (ev->events & EPOLLOUT ? EV_ENABLE : EV_DISABLE) | extra, 0, 0,
		    ev->data.ptr);

#define SET_FLAG(flag)                                                        \
	do {                                                                  \
		if (ev->events & (flag)) {                                    \
			flags |= KQUEUE_STATE_##flag;                         \
		} else {                                                      \
			flags &= ~KQUEUE_STATE_##flag;                        \
		}                                                             \
	} while (0)

		SET_FLAG(EPOLLIN);
		SET_FLAG(EPOLLOUT);
		SET_FLAG(EPOLLRDHUP);
		SET_FLAG(EPOLLPRI);

#undef SET_FLAG

	} else {
		errno = EINVAL;
		return (-1);
	}

	if ((e = kqueue_save_state(fd, fd2, flags)) < 0) {
		errno = e;
		return (-1);
	}

	for (int i = 0; i < 2; ++i) {
		kev[i].flags |= EV_RECEIPT;
	}

	int ret = kevent(fd, kev, 2, kev, 2, NULL);
	if (ret < 0) {
		return -1;
	}

	if (ret != 2) {
		errno = EINVAL;
		return -1;
	}

	for (int i = 0; i < 2; ++i) {
		if (!(kev[i].flags & EV_ERROR)) {
			errno = EINVAL;
			return -1;
		}

		/* ignore EVFILT_WRITE registration EINVAL errors (some fd
		 * types such as kqueues themselves don't support it) */
		if (i == 1 && kev[i].data == EINVAL) {
			continue;
		}

		if (kev[i].data != 0) {
			errno = kev[i].data;
			return -1;
		}
	}

	struct stat statbuf;
	if (fstat(fd2, &statbuf) < 0) {
		return -1;
	}

	if (S_ISFIFO(statbuf.st_mode)) {
		flags |= KQUEUE_STATE_ISFIFO;
	} else if (S_ISSOCK(statbuf.st_mode)) {
		flags |= KQUEUE_STATE_ISSOCK;
	}

	if ((e = kqueue_save_state(fd, fd2, flags)) < 0) {
		errno = e;
		return (-1);
	}
	return 0;
}

#undef VAL_BITS
#undef KEY_BITS

#if 0
int
epoll_pwait(
    int fd, struct epoll_event *ev, int cnt, int to, const sigset_t *sigs)
{
	int r = __syscall(SYS_epoll_pwait, fd, ev, cnt, to, sigs, _NSIG / 8);
#ifdef SYS_epoll_wait
	if (r == -ENOSYS && !sigs)
		r = __syscall(SYS_epoll_wait, fd, ev, cnt, to);
#endif
	return __syscall_ret(r);
}
#endif

int
epoll_wait(int fd, struct epoll_event *ev, int cnt, int to)
{
	if (cnt < 1) {
		errno = EINVAL;
		return -1;
	} else if (cnt > 32) {
		cnt = 32;
	}

	struct timespec timeout = {0, 0};
	if (to > 0) {
		timeout.tv_sec = to / 1000;
		timeout.tv_nsec = (to % 1000) * 1000 * 1000;
	}

	struct timespec *ptimeout = NULL;
	if (to >= 0) {
		ptimeout = &timeout;
	}

	struct kevent evlist[32];
	int ret = kevent(fd, NULL, 0, evlist, cnt, ptimeout);
	if (ret < 0) {
		return -1;
	}

	for (int i = 0; i < ret; ++i) {
		int events = 0;
		if (evlist[i].flags & EV_OOBAND) {
                    uint16_t flags = 0;
                    kqueue_load_state(fd, evlist[i].ident, &flags);

		    if (flags & KQUEUE_STATE_EPOLLPRI)
                        events |= EPOLLPRI;
		}
		if (evlist[i].filter == EVFILT_READ) {
			events |= EPOLLIN;
		} else if (evlist[i].filter == EVFILT_WRITE) {
			events |= EPOLLOUT;
		}

		if (evlist[i].flags & EV_ERROR) {
			events |= EPOLLERR;
		}

		if (evlist[i].flags & EV_EOF) {
			if (evlist[i].fflags) {
				events |= EPOLLERR;
			}

			uint16_t flags = 0;
			kqueue_load_state(fd, evlist[i].ident, &flags);

			int epoll_event;

			if (flags & KQUEUE_STATE_ISFIFO) {
				if (evlist[i].filter == EVFILT_READ) {
					epoll_event = EPOLLHUP;
					if (evlist[i].data == 0) {
						events &= ~EPOLLIN;
					}
				} else if (evlist[i].filter == EVFILT_WRITE) {
					epoll_event = EPOLLERR;
				} else {
					/* should not happen */
					return -1;
				}
			} else if (flags & KQUEUE_STATE_ISSOCK) {
				if (evlist[i].filter == EVFILT_READ) {
					/* do some special EPOLLRDHUP handling
					 * for sockets */

					/* if we are reading, we just know for
					 * sure that we can't receive any more,
					 * so use EPOLLIN/EPOLLRDHUP per
					 * default */
					epoll_event = EPOLLIN;

					if (flags & KQUEUE_STATE_EPOLLRDHUP) {
						epoll_event |= EPOLLRDHUP;
					}
				} else if (evlist[i].filter == EVFILT_WRITE) {
					epoll_event = EPOLLOUT;
				} else {
					/* should not happen */
					return -1;
				}

				struct pollfd pfd = {.fd = evlist[i].ident,
				    .events = POLLIN | POLLOUT | POLLHUP};

				if (poll(&pfd, 1, 0) == 1) {
					if (pfd.revents & POLLHUP) {
						/*
						 * We need to set these flags
						 * so that readers still have a
						 * chance to read the last data
						 * from the socket. This is
						 * very important to preserve
						 * Linux poll/epoll semantics
						 * when coming from an
						 * EVFILT_WRITE event.
						 */
						if (flags &
						    KQUEUE_STATE_EPOLLIN) {
							epoll_event |= EPOLLIN;
						}
						if (flags &
						    KQUEUE_STATE_EPOLLRDHUP) {
							epoll_event |=
							    EPOLLRDHUP;
						}

						epoll_event |= EPOLLHUP;
					}

					/* might as well steal flags from the
					 * poll call while we're here */

					if ((pfd.revents & POLLIN) &&
					    (flags & KQUEUE_STATE_EPOLLIN)) {
						epoll_event |= EPOLLIN;
					}

					if ((pfd.revents & POLLOUT) &&
					    (flags & KQUEUE_STATE_EPOLLOUT)) {
						epoll_event |= EPOLLOUT;
					}
				}
			} else {
				epoll_event = EPOLLHUP;
			}

			events |= epoll_event;
		}
		ev[i].events = events;
		ev[i].data.ptr = evlist[i].udata;
	}

	return ret;
}
