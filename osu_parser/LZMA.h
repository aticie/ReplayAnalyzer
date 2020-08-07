#pragma once
#ifndef LZMA_H
#define LZMA_H

static class lzma
{
public:
	struct data_stream
	{
		const unsigned char* in_data;
		size_t in_len;

		unsigned char* out_data;
		size_t out_len;
	};

	static int decompress_stream_data(data_stream* stream);
	static int compress_stream_data(data_stream* _stream);

private:
	static size_t output_callback(void* ctx, const void* buf, size_t _size);
	static int input_callback(void* ctx, void* buf, size_t* size);
};

#endif
