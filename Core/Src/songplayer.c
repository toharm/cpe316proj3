/*
 * songplayer.c
 *
 *  Created on: Mar 16, 2026
 *      Author: alexi
 */


#include "song_player.h"
#include <stdlib.h>

extern uint32_t HAL_GetTick(void);

struct SongPlayer {
    song_set_target_cb set_target;

    const SongNote *notes;
    size_t notes_len;

    uint8_t loop;
    uint8_t playing;

    size_t current_idx;

    uint32_t note_end_ms;
    uint32_t tempo_bpm;
};

static uint32_t quarter_to_ms(uint32_t bpm, float q)
{
    float quarter_ms = 60000.0f / (float)bpm;
    return (uint32_t)(quarter_ms * q);
}

SongPlayer* SongPlayer_Init(song_set_target_cb set_target, uint32_t tempo_bpm)
{
    SongPlayer *p = malloc(sizeof(SongPlayer));

    p->set_target = set_target;
    p->tempo_bpm = tempo_bpm;

    p->notes = NULL;
    p->notes_len = 0;

    p->playing = 0;
    p->loop = 0;
    p->current_idx = 0;

    return p;
}

void SongPlayer_SetSong(SongPlayer *p, const SongNote *notes, size_t len, uint8_t loop)
{
    p->notes = notes;
    p->notes_len = len;
    p->loop = loop;
}

void SongPlayer_Start(SongPlayer *p, size_t start_index)
{
    if (!p || p->notes_len == 0) return;

    p->playing = 1;
    p->current_idx = start_index;

    float f = p->notes[p->current_idx].freq_hz;

    if (f > 0.0f)
        p->set_target((uint16_t)f);

    p->note_end_ms =
        HAL_GetTick() +
        quarter_to_ms(p->tempo_bpm,
        p->notes[p->current_idx].length_quarter_notes);
}

void SongPlayer_Stop(SongPlayer *p)
{
    if (!p) return;

    p->playing = 0;
}

void SongPlayer_SetTempo(SongPlayer *p, uint32_t tempo_bpm)
{
    p->tempo_bpm = tempo_bpm;
}

void SongPlayer_Poll(SongPlayer *p)
{
    if (!p || !p->playing) return;

    uint32_t now = HAL_GetTick();

    if (now >= p->note_end_ms)
    {
        p->current_idx++;

        if (p->current_idx >= p->notes_len)
        {
            if (p->loop)
                p->current_idx = 0;
            else
            {
                p->playing = 0;
                return;
            }
        }

        SongNote n = p->notes[p->current_idx];

        if (n.freq_hz > 0.0f)
            p->set_target((uint16_t)n.freq_hz);

        p->note_end_ms =
            now +
            quarter_to_ms(p->tempo_bpm,
            n.length_quarter_notes);
    }
}
