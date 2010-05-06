#ifndef Watchdog_h
#define Watchdog_h

class WDT
{
public:
	static void init(const unsigned int timeout_seconds); 
	static void feed(void);
};

#endif // ifndef Watchdog_h

