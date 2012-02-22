//
//  SoundManager.h
//  SLQTSOR
//
//  Created by Michael Daley on 22/05/2009.
//  Copyright 2009 Michael Daley. All rights reserved.
//

#import <OpenAL/al.h>
#import <OpenAL/alc.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>

#define kFadeInterval (1.0f/60)  // Causes the music volume to be updated 60 times per second when music is fading

// SoundManager provides a basic wrapper for OpenAL and AVAudioPlayer.  It is a singleton
// class that allows sound clips to be loaded and cached with a key and then played back
// using that key.  It also allows for music tracks to be played, stopped and paused
//
@interface SoundManager : NSObject <AVAudioPlayerDelegate, AVAudioSessionDelegate> {

	//////////////////// Sound Setup
	ALCcontext *context;				// Context in which all sounds will be played
	ALCdevice *device;					// Reference to the device to use when playing sounds
    ALenum alError;						// Any OpenAL errors that are rasied
	CGPoint listenerPosition;			// Location of the OpenAL Listener
	NSString *soundCategory;			// The sound category to be used by the sound manager
	NSMutableArray *soundSources;		// Mutable array of all sound sources
	AVAudioSession *audioSession;		// Reference to an audio session
	NSError *audioSessionError;			// Audiosession errors are placed in this ivar

	//////////////////// Sound dicionaries
	NSMutableDictionary *soundLibrary;		// Dictionary of all sounds loaded and their keys
	NSMutableDictionary *musicLibrary;		// Dictionary of all music/ambient sounds loaded and their keys
	NSMutableDictionary *musicPlaylists;	// Dictionary of playlists
	NSMutableArray *currentPlaylistTracks;	// Array of tracks for the current play list
	
	//////////////////// Music
	AVAudioPlayer *musicPlayer;			// AVAudioPlayer instance for the music
	
	//////////////////// Volume
	float currentMusicVolume;			// Volume of music/ambient sounds played through AVAudioPlayer
	float fxVolume;						// Volume of OpenAL sound effects
	float musicVolume;					// The master music volume.  This value is not affected by fading music
	
	//////////////////// Fading sound
	NSTimer *timer;						// Timer used fade the music volume up or down
	float fadeAmount;					// Amount the volume should be faded each timer call
	float fadeDuration;					// The amount of time the fade has been running
	float targetFadeDuration;			// The duration the current fade should run for
	
	//////////////////// Flags
	BOOL isExternalAudioPlaying;		// YES if music was playing before the sound engine was initialized i.e.
	BOOL isFading;						// YES if the sound manager is currently fading music
	BOOL isMusicPlaying;				// YES if music is currently playing
	BOOL stopMusicAfterFade;			// YES if music is to be stopped once fading has finished
	BOOL usePlaylist;					// YES if tracks in the playlist should be played one after the other
	BOOL loopPlaylist;					// YES if the playlist should loop when it reaches the end
	BOOL loopLastPlaylistTrack;			// YES if you want the last track of the playlist to be looped forever
	
	//////////////////// Playlist tracking
	int playlistIndex;					// Current index being played in the playlist
	NSString *currentPlaylistName;		// Holds the name of the currently playing play list

}

@property (nonatomic, assign) float currentMusicVolume;
@property (nonatomic, assign) float fxVolume;
@property (nonatomic, assign) BOOL isExternalAudioPlaying;
@property (nonatomic, assign) BOOL isMusicPlaying;
@property (nonatomic, assign) BOOL usePlaylist;
@property (nonatomic, assign) BOOL loopLastPlaylistTrack;
@property (nonatomic, assign) float musicVolume;

// Returns as instance of the SoundManager class.  If an instance has already been created
// then this instance is returned, otherwise a new instance is created and returned.
+ (SoundManager *)sharedSoundManager;

// Designated initializer.
- (id)init;

// Plays the sound which is found with |aSoundKey| using the provided |aGain| and |aPitch|.
// |aLocation| is used to set the location of the sound source in relation to the listener
// and |aLoop| specifies if the sound should be continuously looped or not.
- (NSUInteger)playSoundWithKey:(NSString*)aSoundKey gain:(float)aGain pitch:(float)aPitch 
                       location:(CGPoint)aLocation shouldLoop:(BOOL)aLoop sourceID:(NSInteger)aSourceID;

// Stops all sounds playing with the supplied sound key
- (void)stopSoundWithKey:(NSString*)aSoundKey;

// Loads a sound with the supplied key, filename, file extension and frequency.  Frequency
// could be worked out from the file but this implementation takes it as an argument. If no
// sound is found with a matching key then nothing happens.
- (void)loadSoundWithKey:(NSString*)aSoundKey musicFile:(NSString*)aMusicFile;

// Removes the sound with the supplied key from the sound library.  This deletes the buffer that was created
// to hold the sound
- (void)removeSoundWithKey:(NSString*)aSoundKey;

// Plays the music with the supplied key.  If no music is found then nothing happens.
// |aRepeatCount| specifies the number of times the music should loop.
- (void)playMusicWithKey:(NSString*)aMusicKey timesToRepeat:(NSUInteger)aRepeatCount;

// Plays the next track in the play list if there is one
- (void)playNextTrack;

// Loads the path of a music files into a dictionary with the a key of |aMusicKey|
- (void)loadBackgroundMusicWithKey:(NSString*)aMusicKey musicFile:(NSString*)aMusicFile;

// Removes the path to the music file with the matching key from the music library array.
- (void)removeBackgroundMusicWithKey:(NSString*)aMusicKey;

// Adds a track to the named play list
- (void)addToPlaylistNamed:(NSString*)aPlaylistName track:(NSString*)aTrackName;

// Plays the contents of the named play list
- (void)startPlaylistNamed:(NSString*)aPlaylistName;

// Removes the named track from the named playlist
- (void)removeFromPlaylistNamed:(NSString*)aPlaylistName track:(NSString*)aTrackName;

// Removes the named playlist
- (void)removePlaylistNamed:(NSString*)aPlaylistName;

// Clears the named playlist
- (void)clearPlaylistNamed:(NSString*)aPlaylistName;

// Stops any currently playing music.
- (void)stopMusic;

// Pauses any currently playing music.
- (void)pauseMusic;

// Resumes music that has been paused
- (void)resumeMusic;

// Fades the music volume down to the specified value over the specified period of time.  This method
// does not change the musicVolume for the sound manager which allows you to always get the music volume
// allowing you to fade music down and then back up to the defined value set inside the settings screen for
// example
- (void)fadeMusicVolumeFrom:(float)aFromVolume toVolume:(float)aToVolume duration:(float)aSeconds stop:(BOOL)aStop;

// Cross fades between the currently playing track and the track specified over the duration 
// specified
- (void)crossFadeTo:(NSString*)aTrack duration:(float)aDuration;

// Shutsdown the SoundManager class and deallocates resources which have been assigned.
- (void)shutdownSoundManager;

#pragma mark -
#pragma mark SoundManager settings

// Set the volume for music which is played.
- (void)setMusicVolume:(float)aVolume;

// Sets the location of the OpenAL listener.
- (void)setListenerPosition:(CGPoint)aPosition;

// Sets the orientation of the listener.  This is used to make sure that sound
// is played correctly based on the direction the player is moving in
- (void)setOrientation:(CGPoint)aPosition;

// Sets the volume for all sounds which are played.  This acts as a global FX volume for
// all sounds.
- (void)setFxVolume:(float)aVolume;

@end
