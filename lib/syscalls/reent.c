#include <reent.h>

// Function to add multithread support to newlib
struct _reent *__getreent( void )
{
#if 0
	NU_HISR *HisrPtr;
	NU_TASK *TaskPtr;

	if ((HisrPtr = TCC_Current_HISR_Pointer()) == NULL)
	{
		// Running in normal task mode
		if ((TaskPtr = TCC_Current_Task_Pointer()) == NULL)
		{
			// No valid tasks are running currently return global space
			return _impure_ptr;
		}
		return TaskPtr->_impure_ptr;
	}
	return HisrPtr->_impure_ptr;
#endif
	return _impure_ptr;
}

int *__errno( void )
{
	return &__getreent()->_errno;
}

#if 0
/*** From reent.c ***/

/* Interim cleanup code */

void
_DEFUN (cleanup_glue, (ptr, glue),
		struct _reent *ptr _AND
		struct _glue *glue)
{
	/* Have to reclaim these in reverse order: */
	if (glue->_next)
		cleanup_glue (ptr, glue->_next);

	_free_r (ptr, glue);
}

	void
_DEFUN (_reclaim_reent, (ptr),
		struct _reent *ptr)
{
	if (ptr != _impure_ptr)
	{
		/* used by mprec routines. */
#ifdef _REENT_SMALL
		if (ptr->_mp)	/* don't bother allocating it! */
#endif
			if (_REENT_MP_FREELIST(ptr))
			{
				int i;
				for (i = 0; i < 15 /* _Kmax */; i++)
				{
					struct _Bigint *thisone, *nextone;

					nextone = _REENT_MP_FREELIST(ptr)[i];
					while (nextone)
					{
						thisone = nextone;
						nextone = nextone->_next;
						_free_r (ptr, thisone);
					}
				}

				_free_r (ptr, _REENT_MP_FREELIST(ptr));
			}
		if (_REENT_MP_RESULT(ptr))
			_free_r (ptr, _REENT_MP_RESULT(ptr));

#ifdef _REENT_SMALL
		if (ptr->_emergency)
			_free_r (ptr, ptr->_emergency);
		if (ptr->_mp)
			_free_r (ptr, ptr->_mp);
		if (ptr->_r48)
			_free_r (ptr, ptr->_r48);
		if (ptr->_localtime_buf)
			_free_r (ptr, ptr->_localtime_buf);
		if (ptr->_asctime_buf)
			_free_r (ptr, ptr->_asctime_buf);
		if (ptr->_atexit->_on_exit_args_ptr)
			_free_r (ptr, ptr->_atexit->_on_exit_args_ptr);
#else
		/* atexit stuff */
		if ((ptr->_atexit) && (ptr->_atexit != &ptr->_atexit0))
		{
			struct _atexit *p, *q;
			for (p = ptr->_atexit; p != &ptr->_atexit0;)
			{
				q = p;
				p = p->_next;
				_free_r (ptr, q);
			}
		}
#endif

		if (ptr->_cvtbuf)
	_free_r (ptr, ptr->_cvtbuf);

      if (ptr->__sdidinit)
	{
	  /* cleanup won't reclaim memory 'coz usually it's run
	     before the program exits, and who wants to wait for that? */
	  ptr->__cleanup (ptr);

	  if (ptr->__sglue._next)
	    cleanup_glue (ptr, ptr->__sglue._next);
	}

      /* Malloc memory not reclaimed; no good way to return memory anyway. */

    }
}

/*
 *  Do atexit() processing and cleanup
 *
 *  NOTE:  This is to be executed at task exit.  It does not tear anything
 *         down which is used on a global basis.
 */

void
_DEFUN (_wrapup_reent, (ptr), struct _reent *ptr)
{
  register struct _atexit *p;
  register int n;

  if (ptr == 0)
      ptr = _REENT;

#ifdef _REENT_SMALL
  for (p = &ptr->_atexit, n = p->_ind; --n >= 0;)
    (*p->_fns[n]) ();
#else
  for (p = ptr->_atexit; p; p = p->_next)
    for (n = p->_ind; --n >= 0;)
      (*p->_fns[n]) ();
#endif
  if (ptr->__cleanup)
    (*ptr->__cleanup) (ptr);
}


/*** From impure.c ***/
#ifndef __ATTRIBUTE_IMPURE_PTR__
#define __ATTRIBUTE_IMPURE_PTR__
#endif
#ifndef __ATTRIBUTE_IMPURE_DATA__
#define __ATTRIBUTE_IMPURE_DATA__
#endif

static struct _reent __ATTRIBUTE_IMPURE_DATA__ impure_data = _REENT_INIT (impure_data);
struct _reent *__ATTRIBUTE_IMPURE_PTR__ _impure_ptr = &impure_data;
struct _reent *_CONST __ATTRIBUTE_IMPURE_PTR__ _global_impure_ptr = &impure_data;



#endif
