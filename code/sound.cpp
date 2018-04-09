
struct LoadedSound
{
	u32 sample_count;
	u32 channel_count;
	i16 *samples[2];
};



//
//	WAV loader
//

#pragma pack(push, 1)
struct WaveHeader
{
	u32 RIFFID;
	u32 size;
	u32 WAVEID;
};

#define RIFF_CODE(a, b, c, d) (((u32)(a) << 0) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))
enum
{
	WAVE_CHUNKID_fmt  = RIFF_CODE('f','m','t',' '),
	WAVE_CHUNKID_RIFF = RIFF_CODE('R','I','F','F'),
	WAVE_CHUNKID_WAVE = RIFF_CODE('W','A','V','E'),
	WAVE_CHUNKID_data = RIFF_CODE('d','a','t','a')
};

struct WaveChunk
{
	u32 id;
	i32 size;	
};

struct WaveFmt
{
	u16 w_format_tag;
	u16 n_chanels;
	u32 n_samples_per_sec;
	u32 n_avg_bytes_per_sec;
	u16 n_block_align;
	u16 w_bits_per_sample;
	u32 dw_channel_mask;
	u8  sub_format[16];	
};
#pragma pack(pop)

struct RiffIterator
{
	u8 *at;
	u8 *stop;
};

inline RiffIterator
ParseChunkAt(void *at, void *stop)
{
	RiffIterator it;
	it.at = (u8*)at;
	it.stop = (u8*)stop;
	return it;
}

inline RiffIterator
NextChunk(RiffIterator it)
{
	WaveChunk *chunk = (WaveChunk*)it.at;
	u32 size = (chunk->size + 1) & ~1;
	it.at += sizeof(WaveChunk) + size;
	return it;
}

inline b32
IsValid(RiffIterator it)
{
	return (it.at < it.stop);
}

inline void *
GetChunkData(RiffIterator it)
{
	return (void*) (it.at + sizeof(WaveChunk));
}

inline u32
GetChunkDataSize(RiffIterator it)
{
	WaveChunk *chunk = (WaveChunk*)it.at;
	return chunk->size;
}

inline u32
GetType(RiffIterator it)
{
	WaveChunk *chunk = (WaveChunk*)it.at;
	return (chunk->id);
}

internal LoadedSound
LoadWAV (const char *filename)
{
	LoadedSound res = {};

	u8 *pop = PushMemoryPool(temp_memory);
	void *contents = GetRawFileContents(filename, temp_memory).contents;
	if(contents)
	{
		WaveHeader *header = (WaveHeader*)contents;
		assert(header->RIFFID == WAVE_CHUNKID_RIFF);
		assert(header->WAVEID == WAVE_CHUNKID_WAVE);

		u32 channel_count = 0;
		i16 *sample_data = 0;
		u32 sample_data_size = 0;

		for(RiffIterator it = ParseChunkAt((header + 1), (u8*)(header + 1) + header->size-4);
			IsValid(it);
			it = NextChunk(it))
		{
			switch(GetType(it))
			{
				case WAVE_CHUNKID_fmt:
				{
					WaveFmt *fmt = (WaveFmt*)GetChunkData(it);
					
					assert(fmt->w_format_tag == 1); // PCM
					assert(fmt->n_samples_per_sec == 48000);
					assert(fmt->w_bits_per_sample == 16);
					assert(fmt->n_block_align == sizeof(i16)*fmt->n_chanels);

					channel_count = fmt->n_chanels;
				} break;

				case WAVE_CHUNKID_data:
				{
					sample_data = (i16*)GetChunkData(it);
					sample_data_size = GetChunkDataSize(it);
				} break;
			}
		}

		assert(channel_count && sample_data);

		res.channel_count = channel_count;
		res.sample_count = sample_data_size / (channel_count * sizeof(i16));
		if(channel_count == 1)
		{
			res.samples[0] = AllocateFromMemoryPool(memory, i16, res.sample_count);
			res.samples[1] = 0;

			for (u32 sample_index = 0;
				 sample_index < res.sample_count; 
				 ++sample_index)
			{
				res.samples[0][sample_index] = sample_data[sample_index];
			}
		}
		else if(channel_count == 2)
		{
			res.samples[0] = AllocateFromMemoryPool(memory, i16, res.sample_count);
			res.samples[1] = AllocateFromMemoryPool(memory, i16, res.sample_count);
		
			i16 *src_sample = sample_data;
			for (u32 sample_index = 0;
				 sample_index < res.sample_count; 
				 ++sample_index)
			{
				res.samples[0][sample_index] = *(sample_data++);
				res.samples[1][sample_index] = *(sample_data++);
			}
		}
		else
			ShowSystemPopupTextWindow("waveform","# invalido de channels no WAV!");
	}
	PopMemoryPool(temp_memory, pop);
	
	return res;
}