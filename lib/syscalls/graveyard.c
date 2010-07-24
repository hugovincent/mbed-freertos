int _rename_r(struct _reent *ptr, const char * oldpath, const char * newpath)
{
	int block[4];
	block[0] = (int)oldpath;
	block[1] = strlen(oldpath);
	block[2] = (int)newpath;
	block[3] = strlen(newpath);
	return checkerror(do_AngelSWI(AngelSWI_Reason_Rename, block)) ? -1 : 0;
}

int _gettimeofday_r(struct _reent *ptr, struct timeval * tp, void * tzvp)
{
	struct timezone *tzp = (struct timezone *)tzvp;
	if (tp)
	{
		/* Ask the host for the seconds since the Unix epoch.  */
		tp->tv_sec = do_AngelSWI(AngelSWI_Reason_Time,NULL);
		tp->tv_usec = 0;
	}

	/* Return fixed data for the timezone.  */
	if (tzp)
	{
		tzp->tz_minuteswest = 0;
		tzp->tz_dsttime = 0;
	}

	return 0;
}

int _system(const char *s)
{
	int block[2];
	int e;

	/* Hmmm.  The ARM debug interface specification doesn't say whether
	   SYS_SYSTEM does the right thing with a null argument, or assign any
	   meaning to its return value.  Try to do something reasonable....  */
	if (!s)
		return 1;  /* maybe there is a shell available? we can hope. :-P */
	block[0] = (int)s;
	block[1] = strlen(s);
	e = checkerror(do_AngelSWI(AngelSWI_Reason_System, block));
	if ((e >= 0) && (e < 256))
	{
		/* We have to convert e, an exit status to the encoded status of
		   the command.  To avoid hard coding the exit status, we simply
		   loop until we find the right position.  */
		int exit_code;

		for (exit_code = e; e && WEXITSTATUS (e) != exit_code; e <<= 1)
			continue;
	}
	return e;
}

