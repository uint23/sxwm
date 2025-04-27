#pragma once
#include "defs.h"

static struct { const char *n; void (*fn)(void); } call_table[] = {
	{"close_window",	close_focused},
	{"decrease_gaps",	dec_gaps},
	{"focus_next",		focus_next},
	{"focus_previous", 	focus_prev},
	{"increase_gaps",	inc_gaps},
	{"master_next",		move_master_next},
	{"master_previous",	move_master_prev},
	{"quit",			quit},
	{"master_increase", resize_master_add},
	{"master_decrease", resize_master_sub},
	{"floating",		toggle_floating},
	{"global_floating", toggle_floating_global},
	{"fullscreen",		toggle_fullscreen},
	{NULL,NULL}
};
