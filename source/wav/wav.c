#include "wav.h"
#define wav_printf printf

/** posix wrapper */
static inline stWaveFile_t *wav_fopen(const char szFilePath[], const char szMode[])
{
	if ((szFilePath == NULL) || (szMode == NULL))
	{
		return NULL;
	}
	return (stWaveFile_t *)fopen(szFilePath, szMode);
}
static inline void wav_fclose(stWaveFile_t *fp)
{
	fclose((FILE*)fp);
}
static inline int32_t wav_fseek(stWaveFile_t *fp, uint64_t offset, uint32_t origin)
{
	return (int32_t)fseek((FILE*)fp, offset, origin);
}
static inline size_t wav_fread(void *buf, size_t size, size_t n, stWaveFile_t *fp)
{
	return fread(buf, size, n, (FILE*)fp);
}
static inline size_t wav_fwrite(const void *buf, size_t size, size_t n, stWaveFile_t *fp)
{
	return fwrite(buf, size, n, (FILE*)fp);
}
static inline int32_t wav_feof(stWaveFile_t *fp){
	return (int32_t)feof((FILE*)fp);
}

_Bool WavFileGetFmtChunk(const char szFilePath[], stFmtChunk_t *pstFmtChunk)
{
	_Bool bret = false;
	stHdrChunk_t stHdrChunk;
	stWaveFile_t *fp;

	fp = wav_fopen(szFilePath, "rb");
	if (fp == NULL)
	{
		return false;
	}

	for (;;)
	{
		if (sizeof(stHdrChunk_t) != wav_fread(&stHdrChunk, 1, sizeof(stHdrChunk_t), fp))
		{
			wav_printf("[%s (%d)] wav_fread NG\n", __FUNCTION__, __LINE__);
			break;
		}

		if (memcmp(stHdrChunk.u8chunkID, "RIFF", 4) == 0)
		{
			stRiffChunk_t stRiffChunk;
			if (sizeof(stRiffChunk_t) != wav_fread(&stRiffChunk, 1, sizeof(stRiffChunk_t), fp))
			{
				wav_printf("[%s (%d)] wav_fread NG\n", __FUNCTION__, __LINE__);
				break;
			}
			if (memcmp(stRiffChunk.chunkFormType, "WAVE", 4) != 0)
			{
				wav_printf("[%s (%d)] Unkown Format(%c%c%c%c)\n", __FUNCTION__, __LINE__,
						   stRiffChunk.chunkFormType[0],
						   stRiffChunk.chunkFormType[1],
						   stRiffChunk.chunkFormType[2],
						   stRiffChunk.chunkFormType[3]);
				break;
			}
		}
		else if (memcmp(stHdrChunk.u8chunkID, "fmt ", 4) == 0)
		{
			if (sizeof(stFmtChunk_t) == wav_fread(pstFmtChunk, 1, sizeof(stFmtChunk_t), fp))
			{
				wav_printf("[%s (%d)] FMT CHUNK OK\n", __FUNCTION__, __LINE__);
				bret = true;
			}
			break;
		}
		else
		{
			wav_printf("[%s (%d)] Unkoen Chunk\n", __FUNCTION__, __LINE__);
			/** Skip otherwise */
			if (wav_fseek(fp, stHdrChunk.u32chunkSize, SEEK_CUR) != 0)
			{
				wav_printf("[%s (%d)] wav_fseek NG\n", __FUNCTION__, __LINE__);
				break;
			}
		}
	}

	wav_fclose(fp);
	return bret;
}

stWaveFile_t *WavFileSearchTopOfDataChunk(const char szFilePath[], uint32_t *pu32ChunkSize)
{
	stHdrChunk_t stHdrChunk;
	stWaveFile_t *fp;

	fp = wav_fopen(szFilePath, "rb");
	if (fp == NULL)
	{
		return NULL;
	}

	while (!wav_feof(fp))
	{
		if (sizeof(stHdrChunk_t) != wav_fread(&stHdrChunk, 1, sizeof(stHdrChunk_t), fp))
		{
			wav_printf("[%s (%d)] wav_fread NG\n", __FUNCTION__, __LINE__);
			wav_fclose(fp);
			fp = NULL;
			break;
		}

		if (memcmp(stHdrChunk.u8chunkID, "RIFF", 4) == 0)
		{
			stRiffChunk_t stRiffChunk;
			if (sizeof(stRiffChunk_t) != wav_fread(&stRiffChunk, 1, sizeof(stRiffChunk), fp))
			{
				wav_printf("[%s (%d)] wav_fread NG\n", __FUNCTION__, __LINE__);
				wav_fclose(fp);
				fp = NULL;
				break;
			}
			if (memcmp(stRiffChunk.chunkFormType, "WAVE", 4) != 0)
			{
				wav_printf("[%s (%d)] Unkown Format(%c%c%c%c)\n", __FUNCTION__, __LINE__,
						   stRiffChunk.chunkFormType[0],
						   stRiffChunk.chunkFormType[1],
						   stRiffChunk.chunkFormType[2],
						   stRiffChunk.chunkFormType[3]);
				wav_fclose(fp);
				fp = NULL;
				break;
			}
		}
		else if (memcmp(stHdrChunk.u8chunkID, "data", 4) == 0)
		{
			*pu32ChunkSize = stHdrChunk.u32chunkSize;
			break;
		}
		else
		{
			/** Skip otherwise */
			if (wav_fseek(fp, stHdrChunk.u32chunkSize, SEEK_CUR) != 0)
			{
				wav_printf("[%s (%d)] wav_fseek NG\n", __FUNCTION__, __LINE__);
				wav_fclose(fp);
				fp = NULL;
				break;
			}
		}
	}

	return fp;
}

uint64_t WavFileGetPCMData(stWaveFile_t *fp, uint32_t *pu32RemainChunkSize, uint8_t pu8Buffer[], uint32_t u32BufferSize){
	uint64_t u64ReadBytes = 0;
	stHdrChunk_t stHdrChunk;

	if((fp == NULL) ||
		(pu32RemainChunkSize == NULL) ||
		(pu8Buffer == NULL)){
			return 0;
	}
	while (!wav_feof(fp))
	{
		if(*pu32RemainChunkSize > 0){
			uint32_t u32Tmp;
			uint32_t br;

			if(*pu32RemainChunkSize >= u32BufferSize){
				u32Tmp = u32BufferSize;
			}else{
				u32Tmp = *pu32RemainChunkSize;
			}
			
			br = wav_fread(pu8Buffer, 1, u32Tmp, fp);
			if(u32Tmp != br){
				u64ReadBytes += br;
				*pu32RemainChunkSize -= br;
				break;
			}
			u64ReadBytes += u32Tmp;
			*pu32RemainChunkSize -= u32Tmp;
			break;
		}else{
			if (sizeof(stHdrChunk_t) != wav_fread(&stHdrChunk, 1, sizeof(stHdrChunk_t), fp))
			{
				//wav_printf("[%s (%d)] wav_fread NG\n", __FUNCTION__, __LINE__);
				break;
			}
			if (memcmp(stHdrChunk.u8chunkID, "data", 4) == 0)
			{
				*pu32RemainChunkSize = stHdrChunk.u32chunkSize;
			}
			else
			{
				/** Skip otherwise */
				if (wav_fseek(fp, stHdrChunk.u32chunkSize, SEEK_CUR) != 0)
				{
					wav_printf("[%s (%d)] wav_fseek NG\n", __FUNCTION__, __LINE__);
					break;
				}
			}
		}
	}
	return u64ReadBytes;
}

void WavFileClose(stWaveFile_t *fp){
	wav_fclose(fp);
}

stWaveFile_t *WavFileWriteFmtChunk(const char szFilePath[], const stFmtChunk_t *pstFmtChunk){
	stWaveFile_t *fp;

	fp = wav_fopen(szFilePath, "wb");
	if (fp == NULL)
	{
		return NULL;
	}
	stHdrWriteChunk_t stHdr;
	memcpy(stHdr.u8chunkID, "RIFF", 4);
	memcpy(stHdr.chunkFormType, "WAVE", 4);
	if(sizeof(stHdr) != wav_fwrite(&stHdr, 1, sizeof(stHdrWriteChunk_t), fp)){
		wav_fclose(fp);
		return NULL;
	}
	stHdrChunk_t stHdrFmtChuk;
	memcpy(stHdrFmtChuk.u8chunkID, "fmt ", 4);
	stHdrFmtChuk.u32chunkSize = sizeof(stHdrWriteChunk_t) + 4;
	if(sizeof(stHdrChunk_t) != wav_fwrite(&stHdrFmtChuk, 1, sizeof(stHdrChunk_t), fp)){
		wav_fclose(fp);
		return NULL;
	}
	if(sizeof(stFmtChunk_t) != wav_fwrite(pstFmtChunk, 1, sizeof(stFmtChunk_t), fp)){
		wav_fclose(fp);
		return NULL;
	}
	stHdrChunk_t stHdrDataChunk;
	memcpy(stHdrDataChunk.u8chunkID, "data", 4);
	stHdrDataChunk.u32chunkSize = 0;
	if(sizeof(stHdrDataChunk) != wav_fwrite(&stHdrDataChunk, 1, sizeof(stHdrDataChunk), fp)){
		wav_fclose(fp);
		return NULL;
	}
	return fp;

}

uint64_t WavFileWritePCMData(stWaveFile_t *fp, const uint8_t pu8Buffer[], uint32_t u32BufferSize, uint32_t *pu32DataChunkSize){
	uint64_t ret;

	ret = wav_fwrite(pu8Buffer, 1, u32BufferSize, fp);
	*pu32DataChunkSize += ret;
	return ret; 
}

void WavFileWriteClose(stWaveFile_t *fp, uint32_t u32DataChunkSize)
{
	if (wav_fseek(fp, 40, SEEK_SET) != 0)
	{
		wav_printf("[%s (%d)] wav_fseek NG\n", __FUNCTION__, __LINE__);
		wav_fclose(fp);
		return;
	}
	if(sizeof(uint32_t) != wav_fwrite((void*)&u32DataChunkSize, 1, sizeof(uint32_t), fp)){
		wav_printf("[%s (%d)] wav_fwrite NG\n", __FUNCTION__, __LINE__);
		wav_fclose(fp);
		return;
	}
	u32DataChunkSize += 36;

	if (wav_fseek(fp, 4, SEEK_SET) != 0)
	{
		wav_printf("[%s (%d)] wav_fseek NG\n", __FUNCTION__, __LINE__);
		wav_fclose(fp);
		return;
	}
	if(sizeof(uint32_t) != wav_fwrite((void*)&u32DataChunkSize, 1, sizeof(uint32_t), fp)){
		wav_printf("[%s (%d)] wav_fwrite NG\n", __FUNCTION__, __LINE__);
		wav_fclose(fp);
		return;
	}
}
