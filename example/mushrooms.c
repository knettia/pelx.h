// (c) A. C. Gäßler 2025
//
// This is an example demo for pelx.h, it creates different PNG palettes from one PELX file
// 
// The source of the PELX data is derived from Super Mario Bros. (c) Nintendo 1985,
// and is included only for demonstration purposes.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h> // for mkdir

#define PELX_with_implementation
#define PELX_error_output
#include "pelx.h"

#include "mushroom_texture.h"
#include "mushroom_palettes.h"

static PELX_type(file) create_makeshift_data(void)
{
	PELX_type(file) pelx_file = (PELX_type(file))malloc(sizeof(*pelx_file));
	memset(pelx_file, 0, sizeof(*pelx_file));

	memcpy(pelx_file->header.magic, "PELX\0", 5);
	pelx_file->header.width = 16;
	pelx_file->header.height = 16;
	pelx_file->header.palette_channel_count = 4;
	pelx_file->header.true_channel_count = 4;
	pelx_file->header.palette_count = 2;

	pelx_file->header.header_size = 26;
	pelx_file->header.palette_offset = 26;

	size_t raw_data_size = sizeof(mushroom_texture_data);
	pelx_file->body.data = malloc(raw_data_size);
	pelx_file->body.size = (uint16_t)raw_data_size;
	memcpy(pelx_file->body.data, mushroom_texture_data, raw_data_size);

	return pelx_file;
}

int main(void)
{
	int _ = mkdir("mushrooms", 0777);
	// if on Windows, you will have to replace this with _mkdir("mushrooms") from direct.h

	PELX_type(result) result;

	PELX_type(file) mock_file = create_makeshift_data();
	result = PELX_func(encode_pelx)("mushrooms/data.pelx", mock_file);

	PELX_type(file) pelx_file;
	result = PELX_func(decode_pelx)("mushrooms/data.pelx", &pelx_file);

	for (int i = 0; i < (int)(sizeof(mushroom_palettes) / sizeof(mushroom_palette_entry_t)); i++)
	{
		mushroom_palette_entry_t entry = mushroom_palettes[i];

		char file_path[256];
		snprintf(file_path, sizeof(file_path), "mushrooms/%s_mushroom.png", entry.entry_name);

		result = PELX_func(encode_png)(file_path, pelx_file, 2, entry.palette_entries, 4);
		if (result != PELX_enum(success))
		{
			printf("Encoding failed with error code %d\n", result);
			PELX_func(free_file)(&pelx_file);
			return 1;
		}
	}

	PELX_func(free_file)(&pelx_file);

	return 0;
}
