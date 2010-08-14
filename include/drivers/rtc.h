/* Real time clock driver.
 *
 * Hugo Vincent, 2 August 2010.
 */

#ifndef RealTimeClock_h
#define RealTimeClock_h

#include <time.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void RTC_Init();

void RTC_GetTime(struct tm *tm);
int  RTC_SetTime(struct tm *tm); // returns 0 for success, -1 for failure
void RTC_GetTime_Epoch(time_t *now);
void RTC_SetTime_Epoch(time_t now);

void RTC_GetAlarm(struct tm *tm);
int  RTC_SetAlarm(struct tm *tm); // returns 0 for success, -1 for failure
void RTC_GetAlarm_Epoch(time_t *then);
void RTC_SetAlarm_Epoch(time_t then);

typedef enum
{
	Periodic_PerSec,
	Periodic_PerMin,
	Periodic_PerHour,
	Periodic_PerDay,
	Periodic_PerMonth,
	Periodic_PerYear
} RTC_PeriodicInt_Interval_t;

void RTC_PeriodicAlarm(RTC_PeriodicInt_Interval_t interval, bool enable);

typedef void (*RTC_Callback_t)(void);

RTC_Callback_t RTC_SetAlarmCallback(RTC_Callback_t cb);
RTC_Callback_t RTC_SetPeriodicCallback(RTC_Callback_t cb);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef RealTimeClock_h

