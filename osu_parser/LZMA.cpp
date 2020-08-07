#include "LZMA.h"
#include <easylzma/decompress.h>
#include <easylzma/compress.h>

#include <assert.h>
#include <string.h>

int LZMA::DecompressStreamData(DataStream* _stream)
{
	elzma_decompress_handle handle = elzma_decompress_alloc();
	assert(handle != NULL);

	int ret = elzma_decompress_run(handle, inputCallback, (void*)_stream, outputCallback, (void*)_stream, ELZMA_lzma);

	if (ret != ELZMA_E_OK)
	{
		if (_stream->outData != NULL)
			free(_stream->outData);
		elzma_decompress_free(&handle);
	}

	return ret;
}

int LZMA::CompressStreamData(DataStream* _stream)
{
	elzma_compress_handle handle = elzma_compress_alloc();
	assert(handle != NULL);

	int ret = elzma_compress_run(handle, inputCallback, (void*) _stream,
		outputCallback, (void*) _stream, NULL, NULL);

	if (ret != ELZMA_E_OK)
	{
		if (_stream->outData != NULL)
			free(_stream->outData);
		elzma_compress_free(&handle);
	}

	return ret;
}


// ------------ [PRIVATE] -------------


/* an input callback that will be passed to elzma_decompress_run(),
* it reads from a memory buffer */
int LZMA::inputCallback(void* _ctx, void* _buf, size_t* _size)
{
	size_t rd = 0;
	struct DataStream* stream = (struct DataStream*)_ctx;
	assert(stream != NULL);

	rd = (stream->inLen < *_size) ? stream->inLen : *_size;
	if (rd > 0)
	{
		memcpy(_buf, (void*)stream->inData, rd);
		stream->inData += rd;
		stream->inLen -= rd;
	}

	*_size = rd;

	return 0;
}

/* an ouput callback that will be passed to elzma_decompress_run(),
* it reallocs and writes to a memory buffer */
size_t LZMA::outputCallback(void* _ctx, const void* _buf, size_t _size)
{
	struct DataStream* stream = (struct DataStream*)_ctx;
	assert(stream != nullptr);

	if (_size > 0)
	{
		stream->outData = (unsigned char*)realloc(stream->outData, stream->outLen + _size);
		memcpy((void*)(stream->outData + stream->outLen), _buf, _size);
		stream->outLen += _size;
	}

	return _size;
}