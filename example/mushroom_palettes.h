
#include "pelx.h"

typedef struct
{
	const char *entry_name;
	PELX_type(palette_entry) palette_entries[2];
} mushroom_palette_entry_t;

static mushroom_palette_entry_t mushroom_palettes[] =
{
	{
		.entry_name = "overworld",
		.palette_entries = {
			{ .r = 0xEA, .g = 0x9E, .b = 0x22, .a = 0xFF },
			{ .r = 0xB5, .g = 0x31, .b = 0x20, .a = 0xFF },
		}
	},
	{
		.entry_name = "one_up",
		.palette_entries = {
			{ .r = 0xEA, .g = 0x9E, .b = 0x22, .a = 0xFF },
			{ .r = 0x00, .g = 0x98, .b = 0x00, .a = 0xFF },
		}
	},
	{
		.entry_name = "one_up_underworld",
		.palette_entries = {
			{ .r = 0x6D, .g = 0x41, .b = 0x04, .a = 0xFF },
			{ .r = 0x24, .g = 0x51, .b = 0x56, .a = 0xFF },
		}
	},
	{
		.entry_name = "poison",
		.palette_entries = {
			{ .r = 0x9D, .g = 0x5D, .b = 0x00, .a = 0xFF },
			{ .r = 0x06, .g = 0x00, .b = 0x07, .a = 0xFF },
		}
	}
};
