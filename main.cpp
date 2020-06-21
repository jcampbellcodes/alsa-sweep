#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <math.h>
constexpr int gBufferLen = 48000 * 4;
constexpr unsigned int gNumChans = 1;
constexpr unsigned int gSampleRate = 48000.0;
constexpr const char *gDevice = "default";
constexpr float gTwoPi = 2.0 * M_PI;


int main()
{
    float buffer [gBufferLen];
    snd_pcm_t *handle;

    // 1) open your audio device 
    auto err = 0;
    printf("opening device\n");
    if ((err = snd_pcm_open(&handle, gDevice, SND_PCM_STREAM_PLAYBACK, 0)) < 0) 
    {
            printf("couldn't open device: %s\n", snd_strerror(err));
            return -1;
    }

    // 2) set hw (and sometimes sw) params
    // this is the easy way to set hw and sw params, but there are standalone functions
    // that take "params" structs if you need more fine grained configurations
    printf("setting params\n");
    if ((err = snd_pcm_set_params(handle,
                            SND_PCM_FORMAT_FLOAT, // this determines the data type of the audio calculation below
                            SND_PCM_ACCESS_RW_INTERLEAVED,
                            gNumChans,
                            gSampleRate,
                            1, // allow resampling
                            500000)) // required latency... just picked a big number 
                            < 0) 
    {   
        printf("couldn't set params: %s\n", snd_strerror(err));
        return -1;
    }

    // 3) get some audio data going. could read from input, a wave file, or just generate some audio like this does
    printf("generating audio\n");
    auto phase = 0.0;
    auto freq = 1760.0;
    constexpr float slideFactor = 1.0 / 1.000027;
    auto amplitude = 1.0;
	for (auto k=0; k<gBufferLen; k++)
	{
        phase += gTwoPi * freq / gSampleRate;
		buffer[k] = amplitude * asin(sin(phase));
        freq *= slideFactor;
        amplitude *= slideFactor;
	}

    // 4) then write the audio data to the driver
    printf("outputting audio\n");
	snd_pcm_writei(handle, buffer, gBufferLen);
		
    snd_pcm_close(handle);
    return 0;
}