#include "interface.h"

#define MINIMP3_IMPLEMENTATION
#include "minimp3_ex.h"
#include "vaudio/ivaudio.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

struct AudioStream_s
{
	IAudioStreamEvent *stream_event;
	int offset;
	unsigned int decode_size;
};

size_t mp3dec_read_callback(void *buf, size_t size, void *user_data)
{
	AudioStream_s *stream = static_cast<AudioStream_s*>( (void*)user_data);

	if( stream->decode_size > 0)
	{
		printf("mp3dec_read_callback size: %d\n", (int)size);		
		return size;
	}
	
	int ret_size = stream->stream_event->StreamRequestData( buf, size, stream->offset );
	printf("mp3dec_read_callback size: %d, ret_size: %d\n", (int)size, ret_size);	
	stream->offset += ret_size;

	return ret_size;
}


int mp3dec_seek_callback(uint64_t position, void *user_data)
{
	struct AudioStream_s *stream = static_cast<AudioStream_s*>( (void*)user_data);
	stream->offset = position;
	printf("mp3dec_seek_callback position: %d\n", (int)position);	
	
	return 0;
}

class CMiniMP3 : public IAudioStream
{
public:
	CMiniMP3();
	bool Init( IAudioStreamEvent *pHandler );
	~CMiniMP3();

	// IAudioStream functions
	virtual int	Decode( void *pBuffer, unsigned int bufferSize );
	virtual int GetOutputBits();
	virtual int GetOutputRate();
	virtual int GetOutputChannels();
	virtual unsigned int GetPosition();
	virtual void SetPosition( unsigned int position );
private:

    short pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
	mp3dec_ex_t mp3d;
	mp3dec_io_t mp3io;
	struct AudioStream_s audio_stream;
};

CMiniMP3::CMiniMP3()
{
}

bool CMiniMP3::Init( IAudioStreamEvent *pHandler )
{
	printf("CMiniMP3::Init\n");
	
	audio_stream.stream_event = pHandler;
	audio_stream.offset = 0;
	audio_stream.decode_size = 0;
	
	mp3io.read = &mp3dec_read_callback;
	mp3io.read_data = &audio_stream;
	mp3io.seek = &mp3dec_seek_callback;
	mp3io.seek_data = &audio_stream;
	if( mp3dec_ex_open_cb(&mp3d, &mp3io, MP3D_SEEK_TO_SAMPLE) )
	{
		printf("mp3dec_ex_open_cb failed\n");		
		return false;
	}
		
	if ( mp3dec_ex_seek(&mp3d, 0) )
	{
		printf("mp3dec_ex_seek failed\n");		
		return false;
	}
	
	return true;
}


CMiniMP3::~CMiniMP3()
{
//	m_decoder.Shutdown();
}
	

// IAudioStream functions
int	CMiniMP3::Decode( void *pBuffer, unsigned int bufferSize )
{
	printf("CMiniMP3::Decode size: %d\n", bufferSize);
	audio_stream.decode_size = 1;
	unsigned int readed = mp3dec_ex_read(&mp3d, (mp3d_sample_t*)pBuffer, bufferSize/2);
	printf("mp3dec_ex_read %d\n", readed);
	printf("dec.samples: %d\n", (int)mp3d.samples);

    if (readed != mp3d.samples) /* normal eof or error condition */
    {
        if (mp3d.last_error)
        {
			printf("last_err: %d\n", mp3d.last_error);
            /* error */
        }
    }
	
	return readed;
}


int CMiniMP3::GetOutputBits()
{
	printf("CMiniMP3::GetOutputBits\n");
	return 16;
//	return m_decoder.GetAttribute( m_decoder.OUTPUT_BITS );
}


int CMiniMP3::GetOutputRate()
{
	printf("CMiniMP3::GetOutputRate: %d\n", 44100);
	return 44100;
//	return m_decoder.GetAttribute( m_decoder.OUTPUT_RATE );
}


int CMiniMP3::GetOutputChannels()
{
	printf("CMiniMP3::GetOutputChannels %d\n", 2);	
	return 2;
//	return m_decoder.GetAttribute( m_decoder.OUTPUT_CHANNELS );
}


unsigned int CMiniMP3::GetPosition()
{
	printf("CMiniMP3::GetPosition %d\n", 0);		
	return 0;
//	return m_decoder.GetAttribute( m_decoder.POSITION );
}

// NOTE: Only supports seeking forward right now
void CMiniMP3::SetPosition( unsigned int position )
{
	printf("CMiniMP3::SetPosition %d\n", position);	
//	m_decoder.Seek( position );
}


class CVAudio : public IVAudio
{
public:
	CVAudio()
	{
		// Assume the user will be creating multiple miles objects, so
		// keep miles running while this exists
		//IncrementRefMiles();
	}

	~CVAudio()
	{
		//DecrementRefMiles();
	}

	IAudioStream *CreateMP3StreamDecoder( IAudioStreamEvent *pEventHandler )
	{
		CMiniMP3 *pMP3 = new CMiniMP3;
		if ( !pMP3->Init( pEventHandler ) )
		{
			delete pMP3;
			return NULL;
		}
		return pMP3;
	}

	void DestroyMP3StreamDecoder( IAudioStream *pDecoder )
	{
		 delete pDecoder;
	}
};

EXPOSE_INTERFACE( CVAudio, IVAudio, VAUDIO_INTERFACE_VERSION );
