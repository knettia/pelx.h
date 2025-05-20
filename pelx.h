// (c) A. C. Gäßler (a.k.a. Valken/knettia) 2025
// 
// Overview:
//     PELX is a minimal file format for storing image data that makes use of palettes.
//     To achieve this, PELX is intended to be converted to PNG,
//     automatically mapping its data to corresponding palettes
//     whenever converted for rendering or other purposes.
// 
// Requirements:
//     This library assumes the presence of:
//     - The standard C library
//     - The stb_image_write.h header by Sean Barrett
//
//     Both must be available and included during compilation.
// 
// To use the library:
//     As any other header-based C library, a macro must be defined to tell the header to include the implementation.
//     The macro is named "PELX_with_implementation", thus, to be able to use the library you must include like so:
// 
//     #define PELX_with_implementation 1
//     #include "pelx.h" // or <pelx.h> if used from an include directory
// 
// Here is a demo showcasing decoding and encoding PELX files:
// 
//     PELX_type(file) pelx_file = NULL;
// 
//     PELX_type(result) result = PELX_func(decode_pelx)("texture.pelx", &pelx_file); // decode as PELX
// 
//     PELX_type(palette_entry) entries[] =
//     {
//       	{ 0xFF, 0x00, 0x00, 0xFF },
//       	{ 0x00, 0xFF, 0x00, 0xFF }
//     };
// 
//     result = PELX_func(encode_png)("texture_mod.png", pelx_file, 2, entries, 4); // encode as PNG
// 
// TODO:
//     1. Wake use of the palette definitions block to assign unique names per palette

#if !defined (__PELX_H_LIBRARY__)
#define __PELX_H_LIBRARY__ 1

#if !defined (PELX_def)
#ifdef __cplusplus
#define PELX_def extern "C"
#else
#define PELX_def extern
#endif
#endif // PELX_def

#define PELX_type(n) pelx_##n##_t
#define PELX_func(n) pelx_##n##_f
#define PELX_enum(n) pelx_##n##_e

#include <stdint.h>
#include <string.h>

// Format of PELX:
// +------------------------+
// | Header                 |
// +------------------------+
// | Palette Definitions    | <- begins at offset palette_offset
// +------------------------+
// | Pixel Data             | <- begins at offset header_size
// +------------------------+

// Format of the Pixel Data:
// [00][00]          -> Palette index 0
// [01][11][22][33]  -> RGB pixel (0x11, 0x22, 0x33)
// [00][01]          -> Palette index 1
// [02][AA][BB][CC]  -> RGB pixel (0xAA, 0xBB, CC)

#define PELX_tag_void 0x00
#define PELX_tag_true 0x01
#define PELX_tag_pale 0x02

typedef struct
{
	char magic[5]; // 'PELX\0'
	uint32_t header_size;
	uint32_t palette_offset;
	uint16_t width;
	uint16_t height;
	uint8_t palette_channel_count;
	uint8_t true_channel_count;
	uint16_t palette_count;
	uint8_t reserved[5];
} PELX_type(header);

typedef struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a; // opt
} PELX_type(palette_entry);

typedef struct
{
	PELX_type(header) header;
	struct
	{
		uint16_t size;
		uint8_t *data;
	} body;
} PELX_type(file_data);

typedef PELX_type(file_data) *PELX_type(file);

typedef enum
{
	PELX_enum(success) = 0,

	// Results when there is an IO error (should never happen unless you modified pelx.h)
	PELX_enum(io_error),

	// Results when there is a malloc error (should never happen unless you modified pelx.h)
	PELX_enum(memory_allocation_failed),

	// Results when the PNG channels are invalid (i.e., not 3 or 4)
	PELX_enum(invalid_png_channels),

	// Results when the count of palettes provided is different than the header's palette count
	PELX_enum(mismatched_palettes),

	// Results when the data of the image is corrupted or invalid (not a PELX file)
	PELX_enum(invalid_data_format),

	// Results when the header magic number is not `PELX\0`
	PELX_enum(header_mismatched_magic_number),

	// Results when the width or height of the image are 0
	PELX_enum(header_invalid_size),

	// Results when the palette count of the header is 0
	PELX_enum(header_invalid_palette_count),

	// Results when the true colour channels are invalid (i.e., not 3 or 4)
	PELX_enum(header_invalid_true_channels),

	// Results when the palette channels are invalid (i.e., not 3 or 4)
	PELX_enum(header_invalid_palette_channels),
} PELX_type(result);


// Frees a PELX files
PELX_def void PELX_func(free_file)(PELX_type(file) *file);

// Ensures a PELX header is valid
PELX_def PELX_type(result) PELX_func(sanitize_header)(PELX_type(header) *header);

// Converts a PELX file to a PNG file
PELX_def PELX_type(result) PELX_func(to_png)(PELX_type(file) *pelx_file,
                                             uint16_t palette_count, PELX_type(palette_entry) *palette_entries,
                                             uint8_t png_channels, uint8_t **png_buffer);

// Decodes a PELX file to a PELX file_data_t output
PELX_def PELX_type(result) PELX_func(decode_pelx)(const char *file, PELX_type(file) *output);

// Decodes a PELX file to a PNG uint8_t output
PELX_def PELX_type(result) PELX_func(decode_png)(const char *file,
                                                 uint16_t palette_count, PELX_type(palette_entry) *palette_entries,
                                                 uint8_t png_channels, uint8_t **png_buffer);

// Encodes a PELX file to PELX format
PELX_def PELX_type(result) PELX_func(encode_pelx)(const char *file, PELX_type(file_data) *input_data);

// Encodes a PELX file to PNG format
PELX_def PELX_type(result) PELX_func(encode_png)(const char *file, PELX_type(file_data) *input_data,
                                                 uint16_t palette_count, PELX_type(palette_entry) *palette_entries,
                                                 uint8_t png_channels);

// Implementation
#if defined (PELX_with_implementation)

// Writes a uint8 to a file (big-endianess)
static void PELX_func(write_uint8)(FILE *fp, uint8_t value)
{
	fwrite(&value, 1, 1, fp);
}

// Writes a uint16 to a file (big-endianess)
static void PELX_func(write_uint16)(FILE *fp, uint16_t value)
{
	uint8_t buf[2];
	buf[0] = (value >> 8) & 0xFF;
	buf[1] = value & 0xFF;
	fwrite(buf, 1, 2, fp);
}

// Writes a uint32 to a file (big-endianess)
static void PELX_func(write_uint32)(FILE *fp, uint32_t value)
{
	uint8_t buf[4];
	buf[0] = (value >> 24) & 0xFF;
	buf[1] = (value >> 16) & 0xFF;
	buf[2] = (value >> 8) & 0xFF;
	buf[3] = value & 0xFF;
	fwrite(buf, 1, 4, fp);
}

// Reads a uint32 from a file (big-endianess)
static int PELX_func(read_uint8)(FILE *fp, uint8_t *value)
{
	return fread(value, 1, 1, fp) == 1 ? 0 : -1;
}

// Reads a uint32 from a file (big-endianess)
static int PELX_func(read_uint16)(FILE *fp, uint16_t *value)
{
	uint8_t buf[2];
	if (fread(buf, 1, 2, fp) != 2)
	{
		return -1;
	}

	*value = ((uint16_t)buf[0] << 8) | buf[1];
	return 0;
}

// Reads a uint32 from a file (big-endianess)
static int PELX_func(read_uint32)(FILE *fp, uint32_t *value)
{
	uint8_t buf[4];
	if (fread(buf, 1, 4, fp) != 4)
	{
		return -1;
	}

	*value = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | buf[3];
	return 0;
}

// If the following definitions create problems, you can remove them and handle STBIW yourself
#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include "stb_image_write.h"
#undef STB_IMAGE_WRITE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>

PELX_def void PELX_func(free_file)(PELX_type(file) *file)
{
	if (file == NULL || *file == NULL)
	{
		return;
	}

	free((*file)->body.data);
	(*file)->body.data = NULL;

	free(*file);
	*file = NULL;
}

PELX_def PELX_type(result) PELX_func(sanitize_header)(PELX_type(header) *header)
{
	if (header == NULL)
	{
		return PELX_enum(io_error);
	}

	if (memcmp(header->magic, "PELX\0", 5) != 0)
	{
		return PELX_enum(header_mismatched_magic_number);
	}

	const uint16_t width = header->width;
	const uint16_t height = header->height;
	const uint8_t true_channels = header->true_channel_count;
	const uint8_t palette_channels = header->palette_channel_count;
	const uint16_t palette_count = header->palette_count;

	if (width == 0 || height == 0)
	{
		return PELX_enum(header_invalid_size);
	}

	if (true_channels != 3 && true_channels != 4)
	{
		return PELX_enum(header_invalid_true_channels);
	}

	if (palette_channels != 3 && palette_channels != 4)
	{
		return PELX_enum(header_invalid_palette_channels);
	}

	if (palette_count == 0)
	{
		return PELX_enum(header_invalid_palette_count);
	}

	return PELX_enum(success);
}

PELX_def PELX_type(result) PELX_func(to_png)(PELX_type(file) *pelx_data,
                                             uint16_t palette_count, PELX_type(palette_entry) *palette_entries,
                                             uint8_t png_channels, uint8_t **png_buffer)
{
	if (pelx_data == NULL || *pelx_data == NULL || palette_entries == NULL || png_buffer == NULL)
	{
		return PELX_enum(io_error);
	}
	
	if (png_channels != 3 && png_channels != 4)
	{
		return PELX_enum(invalid_png_channels);
	}
	
	PELX_type(result) result = PELX_func(sanitize_header)(&(*pelx_data)->header);
	if (result != PELX_enum(success))
	{
		return result;
	}
	
	const uint16_t width = (*pelx_data)->header.width;
	const uint16_t height = (*pelx_data)->header.height;
	const uint8_t true_channels = (*pelx_data)->header.true_channel_count;
	const uint8_t palette_channels = (*pelx_data)->header.palette_channel_count;

	size_t output_buffer_size = (size_t)width * height * png_channels;
	
	*png_buffer = (uint8_t *)malloc(output_buffer_size);
	if (*png_buffer == NULL)
	{
		return PELX_enum(memory_allocation_failed);
	}
	
	uint8_t *src = (*pelx_data)->body.data;
	size_t src_pos = 0;
	size_t src_size = (*pelx_data)->body.size;
	
	uint8_t *out_ptr = *png_buffer;
	size_t out_pos = 0;
	
	while (out_pos < output_buffer_size && src_pos < src_size)
	{
		uint8_t tag = src[src_pos++];
		if (tag == PELX_tag_void)
		{
			out_ptr[out_pos++] = 0x00; // R
			out_ptr[out_pos++] = 0x00; // G
			out_ptr[out_pos++] = 0x00; // B

			if (png_channels == 4)
			{
				out_ptr[out_pos++] = 0x00; // A
			}
		}
		else if (tag == PELX_tag_true)
		{
			if (src_pos + true_channels > src_size)
			{
				free(*png_buffer);
				*png_buffer = NULL;
				return PELX_enum(io_error);
			}
			
			out_ptr[out_pos++] = src[src_pos++]; // R
			out_ptr[out_pos++] = src[src_pos++]; // G
			out_ptr[out_pos++] = src[src_pos++]; // B
			
			if (png_channels == 4)
			{
				if (true_channels == 4)
				{
					out_ptr[out_pos++] = src[src_pos++]; // A
				}
				else
				{
					out_ptr[out_pos++] = 0xFF;
				}
			}
			else
			{
				if (true_channels == 4)
				{
					src_pos++; // consume uneeded
				}
			}
		}
		else if (tag == PELX_tag_pale)
		{
			if (src_pos + 1 > src_size)
			{
				free(*png_buffer);
				*png_buffer = NULL;
				return PELX_enum(io_error);
			}

			uint8_t palette_index = src[src_pos++];
			if (palette_index >= palette_count)
			{
				free(*png_buffer);
				*png_buffer = NULL;
				return PELX_enum(io_error);
			}
			
			PELX_type(palette_entry) *entry = &palette_entries[palette_index];
			
			out_ptr[out_pos++] = entry->r;
			out_ptr[out_pos++] = entry->g;
			out_ptr[out_pos++] = entry->b;
			
			if (png_channels == 4)
			{
				if (palette_channels == 4)
				{
					out_ptr[out_pos++] = entry->a;
				}
				else
				{
					out_ptr[out_pos++] = 0xFF;
				}
			}
		}
		else
		{
			free(*png_buffer);
			*png_buffer = NULL;
			return PELX_enum(invalid_data_format);
		}
	}
	
	if (out_pos != output_buffer_size)
	{
	#if defined (PELX_error_output)
		fprintf(stderr, "Mismatch: expected %zu bytes, but wrote %zu\n", output_buffer_size, out_pos);	
	#endif // PELX_error_output
		free(*png_buffer);
		*png_buffer = NULL;
		return PELX_enum(invalid_data_format);
	}

	return PELX_enum(success);
}

PELX_def PELX_type(result) PELX_func(decode_pelx)(const char *file, PELX_type(file) *pelx)
{
	FILE *fp = fopen(file, "rb");
	if (fp == NULL || pelx == NULL)
	{
		return PELX_enum(io_error);
	}

	PELX_type(file_data) *pelx_file = (PELX_type(file_data) *)malloc(sizeof(PELX_type(file_data)));
	if (pelx_file == NULL)
	{
		fclose(fp);
		return PELX_enum(memory_allocation_failed);
	}

	memset(pelx_file, 0, sizeof(PELX_type(file_data)));

	// Read magic PELX\0 (5 bytes)
	if (fread(pelx_file->header.magic, 1, 5, fp) != 5)
	{
		goto return_failure;
	}

	// Read header size: 4 bytes
	if (PELX_func(read_uint32)(fp, &pelx_file->header.header_size) < 0)
	{
		goto return_failure;
	}

	// Read palette offset: 4 bytes
	if (PELX_func(read_uint32)(fp, &pelx_file->header.palette_offset) < 0)
	{
		goto return_failure;
	}
	
	// Read width: 2 bytes
	if (PELX_func(read_uint16)(fp, &pelx_file->header.width) < 0)
	{
		goto return_failure;
	}

	// Read height: 2 bytes
	if (PELX_func(read_uint16)(fp, &pelx_file->header.height) < 0)
	{
		goto return_failure;
	}
	
	// Read palette channels: 1 byte
	if (PELX_func(read_uint8)(fp, &pelx_file->header.palette_channel_count) < 0)
	{
		goto return_failure;
	}

	// Read true channels: 1 byte
	if (PELX_func(read_uint8)(fp, &pelx_file->header.true_channel_count) < 0)
	{
		goto return_failure;
	}
	
	// Read palette count: 1 byte
	if (PELX_func(read_uint16)(fp, &pelx_file->header.palette_count) < 0)
	{
		goto return_failure;
	}

	// Read reserved: 5 bytes
	if (fread(pelx_file->header.reserved, 1, 5, fp) != 5)
	{
		goto return_failure;
	}

	if (fseek(fp, 0, SEEK_END) != 0)
	{
		goto return_failure;
	}

	long file_size = ftell(fp);
	if (file_size < 0 || pelx_file->header.header_size > (uint32_t)file_size)
	{
		goto return_failure;
	}

	size_t raw_data_size = (size_t)file_size - pelx_file->header.header_size;

	pelx_file->body.data = (uint8_t *)malloc(raw_data_size);
	if (pelx_file->body.data == NULL)
	{
		goto return_failure;
	}

	pelx_file->body.size = (uint16_t)raw_data_size;

	if (fseek(fp, pelx_file->header.header_size, SEEK_SET) != 0)
	{
		goto return_failure;
	}

	// Read body data
	if (fread(pelx_file->body.data, 1, raw_data_size, fp) != raw_data_size)
	{
		goto return_failure;
	}

	fclose(fp);
	*pelx = pelx_file;
	return PELX_enum(success);

return_failure:
	free(pelx_file->body.data);
	free(pelx_file);
	fclose(fp);
	return PELX_enum(invalid_data_format);
}

PELX_def PELX_type(result) PELX_func(decode_png)(const char *file,
                                                 uint16_t palette_count, PELX_type(palette_entry) *palette_entries,
                                                 uint8_t png_channels, uint8_t **png_buffer)
{
	if (png_channels != 3 && png_channels != 4)
	{
		return PELX_enum(invalid_png_channels);
	}

	PELX_type(file) pelx_file = NULL;

	PELX_type(result) result = PELX_func(decode_pelx)(file, &pelx_file);
	if (result != PELX_enum(success))
	{
		return result;
	}
	
	result = PELX_func(to_png)(&pelx_file, palette_count, palette_entries, png_channels, png_buffer);

	PELX_func(free_file)(&pelx_file);
	return result;
}

PELX_def PELX_type(result) PELX_func(encode_pelx)(const char *file, PELX_type(file_data) *input_data)
{
	FILE *fp = fopen(file, "wb");
	if (fp == NULL || input_data == NULL)
	{
		return PELX_enum(io_error);
	}

	// Write magic (5 bytes)
	if (fwrite(input_data->header.magic, 1, 5, fp) != 5)
	{
		fclose(fp);
		return PELX_enum(io_error);
	}

	PELX_func(write_uint32)(fp, input_data->header.header_size);
	PELX_func(write_uint32)(fp, input_data->header.palette_offset);

	PELX_func(write_uint16)(fp, input_data->header.width);
	PELX_func(write_uint16)(fp, input_data->header.height);

	PELX_func(write_uint8)(fp, input_data->header.palette_channel_count);
	PELX_func(write_uint8)(fp, input_data->header.true_channel_count);

	PELX_func(write_uint16)(fp, input_data->header.palette_count);

	if (fwrite(input_data->header.reserved, 1, 5, fp) != 5)
	{
		fclose(fp);
		return PELX_enum(io_error);
	}

	if (fwrite(input_data->body.data, 1, input_data->body.size, fp) != input_data->body.size)
	{
		fclose(fp);
		return PELX_enum(io_error);
	}

	fclose(fp);
	return PELX_enum(success);
}

PELX_def PELX_type(result) PELX_func(encode_png)(const char *file, PELX_type(file_data) *input_data,
                                                 uint16_t palette_count, PELX_type(palette_entry) *palette_entries,
                                                 uint8_t png_channels)
{
	if (file == NULL || input_data == NULL || palette_entries == NULL)
	{
		return PELX_enum(io_error);
	}

	if (png_channels != 3 && png_channels != 4)
	{
		return PELX_enum(invalid_png_channels);
	}

	PELX_type(result) result = PELX_func(sanitize_header)(&input_data->header);
	if (result != PELX_enum(success))
	{
		return result;
	}

	const uint16_t width = input_data->header.width;
	const uint16_t height = input_data->header.height;

	size_t png_buffer_size = (size_t)width * height * png_channels;
	uint8_t *png_buffer = (uint8_t *)malloc(png_buffer_size);
	if (png_buffer == NULL)
	{
		return PELX_enum(memory_allocation_failed);
	}

	result = PELX_func(to_png)(&input_data, palette_count, palette_entries, png_channels, &png_buffer);
	if (result != PELX_enum(success))
	{
		free(png_buffer);
		return result;
	}

	int write_success = stbi_write_png(file, width, height, png_channels, png_buffer, width * png_channels);

	free(png_buffer);

	if (write_success == 0)
	{
		return PELX_enum(io_error);
	}

	return PELX_enum(success);
}
#endif // PELX_with_implementation

#endif // __PELX_H_LIBRARY__
