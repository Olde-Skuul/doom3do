#include "Burger.h"
#include <Portfolio.h>
#include <SoundFile.h>
#include <TimerUtils.h>

extern uint32 MainTask;					/* My own task item */

/**********************************

	This big chunk of code handles all 3DO music and sound
	for generic drivers

**********************************/

#define NUMBLOCKS 6		/* Size of memory needed for player */
#define BLOCKSIZE 2048
#define BUFSIZE (NUMBLOCKS*BLOCKSIZE)

static Item MusicIns;			/* Instrument for music so I can pause it */
static char *MusicName;
static Item MixerIns;			/* Main Mixer */
static Item SamplerIns[VOICECOUNT];		/* 3DO Instruments for sound voices */
static Item SamplerRateKnob[VOICECOUNT];	/* 3DO Sample rate knob */
static Item SampleAtt[VOICECOUNT];		/* 3DO Attachment items */
static Boolean LockHit;

Word SampleSound[VOICECOUNT];			/* Sound number being playing in this channel */
Word SamplePriority[VOICECOUNT] = {0,1,2,3};	/* Priority chain */
Item LeftKnobs[VOICECOUNT];		/* Left and right output volumes */
Item RightKnobs[VOICECOUNT];
Word LeftVolume;
Word RightVolume;

static volatile Word SongPtr;	/* Current song script */
static Item iSoundHandler;		/* Item for music thread */
static uint32 SongIsDeadSignal;	/* Signals for the music driver */
static uint32 NewSongIsLoaded;	/* A new song is present */
static uint32 AbortSongSignal;	/* Stop a song */
static Item DiskLocker;		/* So I can lock the multitasking */
static volatile Word OhShit;	/* Flag for aborting the tool */
static char SongFileName[40];		/* Filename of the song */
static char SongNum[10];			/* Number for song name */
Word MusicVolume = 15;			/* Maximum volume */
Word SfxVolume = 15;
Word LoopSong;
Word SongNumber;

/**********************************

	This thread will play the music

**********************************/

static void MusicPlayer(void)
{
	SoundFilePlayer *SongItem;	/* Pointer to data */
	int32 Result;
	int32 SignalIn;
	int32 SignalsNeeded;

	NewSongIsLoaded = AllocSignal(0);
	AbortSongSignal = AllocSignal(0);
	OpenAudioFolio();			/* Start up the sound tools */
	SongItem = CreateSoundFilePlayer(8,BUFSIZE,0);	/* Create my song thread */
	SongItem->sfp_Flags |= (SFP_NO_SAMPLER);
	SongItem->sfp_SamplerIns = LoadInstrument(MusicName,0,100);
	MusicIns = SongItem->sfp_SamplerIns;

/* Wait until a song is ready! */

Again:
	while (!SongPtr) {		/* No song to play? */
		WaitSignal(NewSongIsLoaded|AbortSongSignal);	/* Go to sleep */
	}
	strcpy(SongFileName,"Music/Song");	/* Open the sound file */
	LongWordToAscii(SongNumber,(Byte *)SongNum);
	strcat(SongFileName,SongNum);
	LockItem(DiskLocker,SEM_WAIT);	/* Wait for the lock to happen */
	Result = LoadSoundFile(SongItem,SongFileName);	/* Open up the sound stream */
	UnlockItem(DiskLocker);
	if (Result < 0) {	/* Open up the sound stream */
		goto Abort;		/* Fatal!! */
	}
Repeat:
	StartSoundFile(SongItem,MusicVolume*0x888);	/* Play the music */
	SignalIn = 0;		/* No signals pending */
	SignalsNeeded = 0;
	do {
		if (!SignalIn || (SignalIn & SignalsNeeded)) {
			LockItem(DiskLocker,SEM_WAIT);	/* Wait for the lock to happen */
			if (LockHit) {		/* Was a disk load started? */
				LongWord TimeMark;
				TimeMark = ReadTick();	/* Log the time */
				do {
					LockHit = FALSE;		/* Ack the disk load */
					UnlockItem(DiskLocker);	/* Release the disk */
					SleepMSec(0,80);		/* Sleep a little */
					LockItem(DiskLocker,SEM_WAIT);	/* Try again... */
					if ((ReadTick()-TimeMark)>=40) {	/* Am I getting hungry? */
						LockHit = FALSE;	/* Forget it!! */
						break;
					}
				} while (LockHit);	/* Still hitting the CD ROM? */
			}
			Result = ServiceSoundFile(SongItem,SignalIn, &SignalsNeeded);
			UnlockItem(DiskLocker);
		}
		if (Result<0 || OhShit) {		/* Sound error?!? */
			goto BadNews;		/* Forget it! */
		}
		if (SignalsNeeded) {	/* Do I need to wait? */
			SignalIn = WaitSignal(SignalsNeeded | AbortSongSignal);
		}
		if (OhShit) {		/* Time to abort? */
BadNews:
			StopSoundFile(SongItem);		/* Stop the current song */
			UnloadSoundFile(SongItem);	/* Release the memory!! IMPORTANT! */
			Result = GetCurrentSignals() & SignalsNeeded;
			if (Result) {
				WaitSignal(Result);
			}
			goto Abort;
		}
	} while (SignalsNeeded);		/* Do I still have data pending? */

	StopSoundFile(SongItem);		/* Stop the current song */
	if (LoopSong) {		/* End of list? */
		RewindSoundFile(SongItem);	/* Rewind the song */
		goto Repeat;			/* Repeat the song */
	}
	SongPtr = 0;				/* Index to the next song */
	UnloadSoundFile(SongItem);	/* Release the memory!! IMPORTANT! */
	Result = GetCurrentSignals() & SignalsNeeded;
	if (Result) {
		WaitSignal(Result);
	}
	if (SongPtr) {			/* Are there any more songs? */
		goto Again;
	}
Abort:;
	SongPtr = 0;
	SendSignal(MainTask,SongIsDeadSignal);	/* Tell the main task that I am dead */
	goto Again;
}						/* Exit the task */

/**********************************

	Allocate resources for the music player for 3DO

**********************************/

void InitMusicPlayer(char *MusicDSP)
{
	MusicName = MusicDSP;
	DiskLocker = CreateSemaphore("DiskLocker",KernelBase->kb_CurrentTask->t.n_Priority);
	SongIsDeadSignal = AllocSignal(0);
	NewSongIsLoaded = AllocSignal(0);
	AbortSongSignal = AllocSignal(0);
	iSoundHandler = CreateThread("MusicPlayer",KernelBase->kb_CurrentTask->t.n_Priority+1,MusicPlayer,4000);
	SetMusicVolume(15);
}

/**********************************

	Play a song

**********************************/

void PlaySong(Word NewSong)
{
	LoopSong = NewSong;
	if (NewSong==SongNumber) {	/* Same as before? */
		return;			/* Bye */
	}
	if (SongPtr && NewSong==SongPtr) {
		return;
	}

	if (SongNumber) {			/* Was a song playing? */
		OhShit = 1;
		SendSignal(iSoundHandler,AbortSongSignal);

		/* Wait until song is aborted */

		while (SongPtr) {	/* No result */
			WaitSignal(SongIsDeadSignal);
		}
		SongNumber = 0;		/* No song anymore */
		OhShit = 0;
		ScavengeMem();		/* Release memory */
	}

	if (NewSong) {		/* Shall I play a song? */
		SongNumber = NewSong;	/* Set the song */
		if (!(SystemState&MusicActive)) {
			return;
		}
		SongPtr = NewSong;	/* Get the real song file # */
		if (iSoundHandler) {
			SendSignal(iSoundHandler,NewSongIsLoaded);
		}
	}
}

/**********************************

	Prepare the sound player for the threedo
	Audio requirements
	Total memory (512 words)
	Total clocks (565 ticks)
	Head.dsp 29/32
	Tail.dsp 16/16
	Adpcm 91/51 (364/204)
	dcsqxstereo.dsp 66/64
	mixer4x2.dsp 32/32
	directout.dsp 8/8


**********************************/

static char LeftG[] = "LeftGain0";
static char RightG[] = "RightGain0";
static char Inp[] = "Input0";

void InitSoundPlayer(char *SoundDSP,Word Rate)
{
	Word i;

	MixerIns = LoadInstrument("system/audio/dsp/mixer4x2.dsp",0,0);
	i = 0;
	do {
		LeftG[8] = i+'0';		/* Set the left gain text */
		RightG[9] = i+'0';		/* Set the right gain text */
		LeftKnobs[i] = GrabKnob(MixerIns,LeftG);	/* Grab the knobs */
		RightKnobs[i] = GrabKnob(MixerIns,RightG);
	} while (++i<VOICECOUNT);			/* All 4 channels */
	StartInstrument(MixerIns,0);		/* Begin execution of the mixer */

	i = 0;
	do {
		SamplerIns[i] = LoadInstrument(SoundDSP,0,100);
		SamplerRateKnob[i] = GrabKnob(SamplerIns[i],"Frequency");
		Inp[5] = i+'0';
		ConnectInstruments(SamplerIns[i],"Output",MixerIns,Inp);	/* Allow mono out on both speakers */
	} while (++i<VOICECOUNT);
	SetSfxVolume(15);
	LeftVolume = 255;
	RightVolume = 255;
}

/**********************************

	Play a sound effect

**********************************/

void PlaySound(Word SoundNum)
{
	Word Number;
	Word i;
	Word OldPrior;

	if (SoundNum&0x8000) {		/* Stop previous sound */
		SoundNum&=(~0x8000);	/* Clear the flag */
		StopSound(SoundNum);	/* Stop the sound */
	}
	if (!SoundNum ||			/* No sound at all? */
		!AllSamples[SoundNum-1] ||	/* Valid sound loaded? */
		!(SystemState&SfxActive)) {	/* Sound enabled? */
		goto WrapUp;
	}

	Number = 0;
	do {
		if (!SampleSound[Number]) {		/* Find an empty sound channel */
			goto Begin;				/* Found it! */
		}
	} while (++Number<VOICECOUNT);

	Number = 0;			/* Get the lowest priority sound */
	do {
		if (!SamplePriority[Number]) {	/* Get the priority */
			break;				/* Priority zero? */
		}
	} while (++Number<(VOICECOUNT-1));	/* Drop out on the last channel */

	StopInstrument(SamplerIns[Number],0);		/* Stop the sound */
	if (SampleAtt[Number]) {					/* Delete the attachment */
		DetachSample(SampleAtt[Number]);
	}

Begin:
    SampleSound[Number] = SoundNum;		/* Attach the sound number to this channel */
	SampleAtt[Number] = AttachSample(SamplerIns[Number],AllSamples[SoundNum-1],0);
	TweakKnob(SamplerRateKnob[Number],AllRates[SoundNum-1]);		/* Set to 11 Khz fixed */
	TweakKnob(LeftKnobs[Number],LeftVolume<<6);		/* Set the volume setting */
	TweakKnob(RightKnobs[Number],RightVolume<<6);
	StartInstrument(SamplerIns[Number],0);		/* Begin playing... */
	i = 0;
	OldPrior = SamplePriority[Number];		/* Get the old priority */
	do {
		if (SamplePriority[i]>=OldPrior) {
			--SamplePriority[i];			/* Reduce the priority */
		}
	} while (++i<VOICECOUNT);
	SamplePriority[Number] = (VOICECOUNT-1);		/* Set the higher priority */
WrapUp:
	LeftVolume = 255;		/* Reset the volume */
	RightVolume = 255;
}

/**********************************

	Stop a sound effect (If playing)

**********************************/

void StopSound(Word SoundNum)
{
	Word i;
	if (SoundNum) {
		i = 0;
		do {
			if (SampleSound[i] == SoundNum) {		/* Match? */
				StopInstrument(SamplerIns[i],0);	/* Stop the sound */
				if (SampleAtt[i]) {
					DetachSample(SampleAtt[i]);		/* Remove the attachment */
					SampleAtt[i] = 0;
				}
				SampleSound[i] = 0;		/* No sound active */
			}
		} while (++i<VOICECOUNT);		/* All channels checked? */
	}
}

/**********************************

	Pause the sound effects

**********************************/

static Boolean Paused;

void PauseSound(void)
{
	Word i;
	if (Paused) {
		return;
	}
	Paused = TRUE;
	i = 0;
	do {
		PauseInstrument(SamplerIns[i]);
	} while (++i<VOICECOUNT);
}

/**********************************

	Resume paused sound

**********************************/

void ResumeSound(void)
{
	Word i;
	if (Paused) {
		Paused = FALSE;
		i = 0;
		do {
			ResumeInstrument(SamplerIns[i]);
		} while (++i<VOICECOUNT);
	}
}

void PauseMusic(void)
{
	PauseInstrument(MusicIns);
}

void ResumeMusic(void)
{
	ResumeInstrument(MusicIns);
}

void SetSfxVolume(Word NewVolume)
{
	Word i;
	Item MyKnob;
	if (NewVolume>=16) {
		NewVolume=15;
	}
	SfxVolume=NewVolume;
	NewVolume = NewVolume*0x888;	/* Convert 0-15 -> 0-0x7FF8 */
	i = 0;
	do {
		MyKnob = GrabKnob(SamplerIns[i],"Amplitude");
		TweakKnob(MyKnob,NewVolume);		/* Set to 11 Khz fixed */
		ReleaseKnob(MyKnob);
	} while (++i<VOICECOUNT);
}

void SetMusicVolume(Word NewVolume)
{
	Item MyKnob;
	if (NewVolume>=16) {
		NewVolume=15;
	}
	MusicVolume=NewVolume;
	NewVolume = NewVolume*0x888;	/* Convert 0-15 -> 0-0x7FF8 */
	MyKnob = GrabKnob(MusicIns,"Amplitude");
	TweakKnob(MyKnob,NewVolume);		/* Set to 11 Khz fixed */
	ReleaseKnob(MyKnob);
}

void LockMusic(void)
{
	if (DiskLocker) {
		LockItem(DiskLocker,SEM_WAIT);	/* Lock me down */
	}
}

void UnlockMusic(void)
{
	if (DiskLocker) {
		LockHit = TRUE;
		UnlockItem(DiskLocker);		/* Release the disk */
	}
}
