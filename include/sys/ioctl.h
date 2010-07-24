/* sys/ioctl.h
 *
 * Hugo Vincent, 24 July 2010.
 */

#ifndef sys_ioctl_h 
#define sys_ioctl_h

#ifdef __cplusplus
extern "C" {
#endif

// FIXME add the ioctl request definitions

int ioctl(int fd, unsigned long request, ...);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef sys_ioctl_h

