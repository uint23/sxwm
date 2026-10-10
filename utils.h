#ifndef UTILS_H
#define UTILS_H

#define llog(...) wlog_at(__FILE__, __LINE__, __VA_ARGS__)

typedef enum {
	E_OK,
	E_OOM,
	E_DISPLAY,
	E_WM_RUNNING,
	E_COUNT
} ErrorCode;

void die(ErrorCode ec, const char* fmt, ...);
void wlog(const char* fmt, ...);
void wlog_at(const char* file, int line, const char* fmt, ...);

#endif /* UTILS_H */

