#include "LZMA.h"
#include <easylzma/decompress.h>
#include <easylzma/compress.h>

#include <cassert>
#include <cstring>

int lzma::decompress_stream_data(data_stream* stream)
{
	auto* handle = elzma_decompress_alloc();
	assert(handle != NULL);

	const auto ret = elzma_decompress_run(handle, input_callback, static_cast<void*>(stream), output_callback,
	                                      static_cast<void*>(stream), ELZMA_lzma);

	if (ret != ELZMA_E_OK)
	{
		if (stream->out_data != nullptr)
			free(stream->out_data);
		elzma_decompress_free(&handle);
	}

	return ret;
}

int lzma::compress_stream_data(data_stream* _stream)
{
	auto* handle = elzma_compress_alloc();
	assert(handle != NULL);

	const auto ret = elzma_compress_run(handle, input_callback, static_cast<void*>(_stream),
	                                    output_callback, static_cast<void*>(_stream), nullptr, nullptr);

	if (ret != ELZMA_E_OK)
	{
		if (_stream->out_data != nullptr)
			free(_stream->out_data);
		elzma_compress_free(&handle);
	}

	return ret;
}


// ------------ [PRIVATE] -------------


/* an input callback that will be passed to elzma_decompress_run(),
* it reads from a memory buffer */
int lzma::input_callback(void* ctx, void* buf, size_t* size)
{
	auto* stream = static_cast<struct data_stream*>(ctx);
	assert(stream != NULL);

	const auto rd = (stream->in_len < *size) ? stream->in_len : *size;
	if (rd > 0)
	{
		memcpy(buf, (void*)stream->in_data, rd);
		stream->in_data += rd;
		stream->in_len -= rd;
	}

	*size = rd;

	return 0;
}

/* an ouput callback that will be passed to elzma_decompress_run(),
* it reallocs and writes to a memory buffer */
size_t lzma::output_callback(void* ctx, const void* buf, const size_t _size)
{
	auto* stream = static_cast<struct data_stream*>(ctx);
	assert(stream != nullptr);

	if (_size > 0)
	{
		stream->out_data = static_cast<unsigned char*>(realloc(stream->out_data, stream->out_len + _size));
		memcpy(static_cast<void*>(stream->out_data + stream->out_len), buf, _size);
		stream->out_len += _size;
	}

	return _size;
}
