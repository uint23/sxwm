#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

static void vlog(const char* file, int line, const char* fmt, va_list ap);

static const char* error_messages[E_COUNT] = {
	[E_OK] = "Success",
	[E_OOM] = "Out of memory",
	[E_DISPLAY] = "Could not open X display",
	[E_WM_RUNNING] = "Another window manager is already running",
	[E_EXEC] = "Could not execute command"
};

void die(ErrorCode ec, const char* fmt, ...)
{
	va_list ap;
	int valid = ec > E_OK && ec < E_COUNT;
	int code = valid ? ec : EXIT_FAILURE;

	fprintf(stderr, "sxwm: %s", valid ? error_messages[ec] : "Unknown error");

	if (fmt) {
		fputs(": ", stderr);
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}

	fprintf(stderr, " [%d]\n", code);
	exit(code);
}

static void vlog(const char* file, int line, const char* fmt, va_list ap)
{
	fputs("sxwm: ", stderr);

	if (file)
		fprintf(stderr, "%s:%d: ", file, line);

	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	fflush(stderr);
}

void wlog(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vlog(NULL, 0, fmt, ap);
	va_end(ap);
}

void wlog_at(const char* file, int line, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vlog(file, line, fmt, ap);
	va_end(ap);
}

