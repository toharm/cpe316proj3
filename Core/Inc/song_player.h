/*
 * song_player.h
 *
 *  Created on: Mar 16, 2026
 *      Author: alexi
 */

#ifndef INC_SONG_PLAYER_H_
#define INC_SONG_PLAYER_H_

#include <stdint.h>
#include <stddef.h>

typedef struct {
    float freq_hz;
    float length_quarter_notes;
} SongNote;

/* Called whenever the note changes */
typedef void (*song_set_target_cb)(uint16_t target_hz);

typedef struct SongPlayer SongPlayer;

SongPlayer* SongPlayer_Init(song_set_target_cb set_target, uint32_t tempo_bpm);

void SongPlayer_SetSong(
    SongPlayer *p,
    const SongNote *notes,
    size_t len,
    uint8_t loop
);

void SongPlayer_Start(SongPlayer *p, size_t start_index);
void SongPlayer_Stop(SongPlayer *p);

void SongPlayer_SetTempo(SongPlayer *p, uint32_t tempo_bpm);

void SongPlayer_Poll(SongPlayer *p);


#endif /* INC_SONG_PLAYER_H_ */
