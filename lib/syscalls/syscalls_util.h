#define AngelSWI_Reason_Open		0x01
#define AngelSWI_Reason_Close		0x02
#define AngelSWI_Reason_WriteC		0x03
#define AngelSWI_Reason_Write0		0x04
#define AngelSWI_Reason_Write		0x05
#define AngelSWI_Reason_Read		0x06
#define AngelSWI_Reason_ReadC		0x07
#define AngelSWI_Reason_IsTTY		0x09
#define AngelSWI_Reason_Seek		0x0A
#define AngelSWI_Reason_FLen		0x0C
#define AngelSWI_Reason_TmpNam		0x0D
#define AngelSWI_Reason_Remove		0x0E
#define AngelSWI_Reason_Rename		0x0F
#define AngelSWI_Reason_Clock		0x10
#define AngelSWI_Reason_Time		0x11
#define AngelSWI_Reason_System		0x12
#define AngelSWI_Reason_Errno		0x13
#define AngelSWI_Reason_GetCmdLine 	0x15
#define AngelSWI_Reason_HeapInfo 	0x16
#define AngelSWI_Reason_EnterSVC 	0x17
#define AngelSWI_Reason_ReportException 0x18
#define ADP_Stopped_ApplicationExit ((2 << 16) + 38)
#define ADP_Stopped_RunTimeError 	((2 << 16) + 35)
#define do_AngelSWI(a, b) 			(0)

int	checkerror	_PARAMS ((int));
int	error		_PARAMS ((int));

/* Struct used to keep track of the file position, just so we
   can implement fseek(fh,x,SEEK_CUR).  */
struct fdent
{
	int handle;
	int pos;
};

#define MAX_OPEN_FILES 10

/* User file descriptors (fd) are integer indexes into 
   the openfiles[] array. Error checking is done by using
   findslot(). 

   This openfiles array is manipulated directly by only 
   these 5 functions:

   findslot() - Translate entry.
   newslot() - Find empty entry.
   initialise_stdio() - Initialize entries.
   _open() - Initialize entry.
   _close() - Handle stdout == stderr case.

   Every other function must use findslot().  */

struct fdent openfiles [MAX_OPEN_FILES];

struct fdent* 	findslot	_PARAMS ((int));
int				newslot		_PARAMS ((void));

/* following is copied from libc/stdio/local.h to check std streams */
void   _EXFUN(__sinit,(struct _reent *));

