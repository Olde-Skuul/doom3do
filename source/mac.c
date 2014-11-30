#include "Doom.h"
#include <IntMath.h>

LongWord LastTics;

void WritePrefsFile(void) {}
void ClearPrefsFile(void) {}
void ReadPrefsFile(void) {}
void UpdateAndPageFlip(void) {}
Word LeftVolume;
Word RightVolume;
void DrawPlaque(Word RezNum){}
void DrawFloorColumn(Word y,Word x,Word Count,LongWord ds_xfrac,
	LongWord ds_yfrac,Fixed ds_xstep,Fixed sd_ystep) {}
void PlaySong(Word y) {}
void DrawSkyLine(void) {}
void DrawWallColumn(Word y,Word colnum,Byte *Source,Word Run) {}
void PauseMusic(void) {}
void ResumeMusic(void) {}
void SetSfxVolume(Word New) {}
void SetMusicVolume(Word New) {}
void EnableHardwareClipping(void) {}
void DisableHardwareClipping(void) {}
void DrawSpriteNoClip(vissprite_t *vis) {}
void DrawSpriteClip(Word x1,Word x2,vissprite_t *vis) {}
void DrawColors(void) {}

Word SfxVolume;
Word MusicVolume;
Fixed IMFixMul(Fixed a,Fixed b) { return 0;}
Fixed IMFixDiv(Fixed a,Fixed b) { return 0;}
void PlaySound(Word SoundNum) {}
void StopSound(Word SoundNum) {}
void Halt(void) {}
void DrawSpriteCenter(Word RezNum) {}

void main(void)
{
	D_DoomMain();
}