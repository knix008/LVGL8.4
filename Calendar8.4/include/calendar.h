#ifndef CALENDAR_H
#define CALENDAR_H

#include <time.h>

// Calendar state structure
typedef struct {
    int year;
    int month;
    int day;
} calendar_date_t;

// Calendar logic functions - only those used externally
void calendar_init(calendar_date_t* date);
void calendar_set_date(calendar_date_t* date, int year, int month, int day);
void calendar_prev_month(calendar_date_t* date);
void calendar_next_month(calendar_date_t* date);
void calendar_prev_year(calendar_date_t* date);
void calendar_next_year(calendar_date_t* date);
void calendar_prev_day(calendar_date_t* date);
void calendar_next_day(calendar_date_t* date);
int calendar_get_day_of_week(const calendar_date_t* date);
const char* calendar_get_month_name(int month);
const char* calendar_get_day_name(int day_of_week);
void calendar_format_date_string(const calendar_date_t* date, char* buffer, size_t buffer_size);
const char* calendar_get_month_abbr(int month);

#endif // CALENDAR_H 