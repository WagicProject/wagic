//
//  SoundManager.m
//  SLQTSOR
//
//  Created by Michael Daley on 22/05/2009.
//  Copyright 2009 Michael Daley. All rights reserved.
//

#import "SoundManager.h"
#import "SynthesizeSingleton.h"
#import "MyOpenALSupport.h"

#pragma mark -
#pragma mark Private interface

@interface SoundManager (Private)

// This method is used to initialize OpenAL.  It gets the default device, creates a new context 
// to be used and then preloads the define # sources.  This preloading means we wil be able to play up to
// (max 32) different sounds at the same time
- (BOOL)initOpenAL;

// Used to get the next available OpenAL source.  The returned source is then bound to a sound
// buffer so that the sound can be played.  This method checks each of the available OpenAL 
// soucres which have been generated and returns the first source which is not currently being
// used.  If no sources are found to be free then the first looping source is returned.  If there
// are no looping sources then the first source created is returned
- (NSUInteger)nextAvailableSource;

// Used to set the current state of OpenAL.  When the game is interrupted the OpenAL state is
// stopped and then restarted when the game becomes active again.
- (void)setActivated:(BOOL)aState;

// If audio is currently playing this method returns YES
- (BOOL)isAudioPlaying;

// Checks to see if an OpenAL error has been logged.  If so it renders the error to the screen
- (void)checkForErrors;

@end


#pragma mark -
#pragma mark Public implementation

@implementation SoundManager

// Make this class a singleton class
SYNTHESIZE_SINGLETON_FOR_CLASS(SoundManager);

@synthesize currentMusicVolume;
@synthesize fxVolume;
@synthesize isExternalAudioPlaying;
@synthesize isMusicPlaying;
@synthesize usePlaylist;
@synthesize loopLastPlaylistTrack;
@synthesize musicVolume;

#pragma mark -
#pragma mark Dealloc and Init and Shutdown

- (void)dealloc {
	// Loop through the OpenAL sources and delete them
	for(NSNumber *sourceIDVal in soundSources) {
		NSUInteger sourceID = [sourceIDVal unsignedIntValue];
		alDeleteSources(1, &sourceID);
		[self checkForErrors];
	}
	
	// Loop through the OpenAL buffers and delete 
	NSEnumerator *enumerator = [soundLibrary keyEnumerator];
	id key;
	while ((key = [enumerator nextObject])) {
		NSNumber *bufferIDVal = [soundLibrary objectForKey:key];
		NSUInteger bufferID = [bufferIDVal unsignedIntValue];
		alDeleteBuffers(1, &bufferID);
		[self checkForErrors];
	}
    
	// Release the arrays and dictionaries we have been using
	[soundLibrary release];
	[soundSources release];
	[musicLibrary release];
	[musicPlaylists release];
	if (currentPlaylistTracks) {
		[currentPlaylistTracks release];
	}
	
	// If background music has been played then release the AVAudioPlayer
	if(musicPlayer)
		[musicPlayer release];
	
	// Disable and then destroy the context
	alcMakeContextCurrent(NULL);
	[self checkForErrors];
	alcDestroyContext(context);
	[self checkForErrors];
	
	// Close the device
	alcCloseDevice(device);
	[self checkForErrors];
	
	[super dealloc];
}

- (id)init {
    self = [super init];
	if(self != nil) {
		
        // Initialize the array and dictionaries we are going to use
		soundSources = [[NSMutableArray alloc] init];
		soundLibrary = [[NSMutableDictionary alloc] init];
		musicLibrary = [[NSMutableDictionary alloc] init];
		musicPlaylists = [[NSMutableDictionary alloc] init];
		
		// Grab a reference to the AVAudioSession singleton
		audioSession = [AVAudioSession sharedInstance];

		// Reset the error ivar
		audioSessionError = nil;
		
		// Check to see if music is already playing. If that is the case then you can leave the sound category as AmbientSound.  
		// If music is not playing we can set the sound category to SoloAmbientSound so that decoding is done using the hardware.
		isExternalAudioPlaying = [self isAudioPlaying];
		
		if (!isExternalAudioPlaying) {
			NSLog(@"INFO - SoundManager: No external audio playing so using the SoloAmbient audio session category");
			soundCategory = AVAudioSessionCategorySoloAmbient;
		} else {
			NSLog(@"INFO - SoundManager: External sound detected so using the Ambient audio session category");
			soundCategory = AVAudioSessionCategoryAmbient;
		}

		// Having decided on the category we then set it
		[audioSession setCategory:soundCategory error:&audioSessionError];
		
		if (audioSessionError) {
			NSLog(@"WARNING - SoundManager: Unable to set the sound category to ambient");
		}
		
        // Set up the OpenAL.  If an error occurs then nil will be returned.
		BOOL success = [self initOpenAL];
		if(!success) {
            NSLog(@"ERROR - SoundManager: Error initializing OpenAL");
            return nil;
        }
		
		// Set up the listener position
		float listener_pos[] = {0, 0, 0};
		float listener_ori[] = {0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
		float listener_vel[] = {0, 0, 0};
		
		alListenerfv(AL_POSITION, listener_pos);
		[self checkForErrors];
		alListenerfv(AL_ORIENTATION, listener_ori);
		[self checkForErrors];
		alListenerfv(AL_VELOCITY, listener_vel);
		[self checkForErrors];
        
        // Set the default volume for music and fx along the fading flag
		currentMusicVolume = 0.5f;
		musicVolume = 0.5f;
		fxVolume = 0.5f;
		playlistIndex = 0;
		
		// Set up initial flag values
		isFading = NO;
		isMusicPlaying = NO;
		stopMusicAfterFade = YES;
		usePlaylist = NO;
		loopLastPlaylistTrack = NO;
	}
    return self;
}

- (void)shutdownSoundManager {
	@synchronized(self) {
		if(sharedSoundManager != nil) {
			[self dealloc];
		}
	}
}

#pragma mark -
#pragma mark Sound management

- (void)loadSoundWithKey:(NSString*)aSoundKey musicFile:(NSString*)aMusicFile {

    // Check to make sure that a sound with the same key does not already exist
    NSNumber *numVal = [soundLibrary objectForKey:aSoundKey];
    
    // If the key is not found log it and finish
    if(numVal != nil) {
        NSLog(@"WARNING - SoundManager: Sound key '%@' already exists.", aSoundKey);
        return;
    }
    
    NSUInteger bufferID;
	
	// Generate a buffer within OpenAL for this sound
	alGenBuffers(1, &bufferID);
	[self checkForErrors];
    
    // Set up the variables which are going to be used to hold the format
    // size and frequency of the sound file we are loading
	ALenum  format;
	ALsizei size;
	ALsizei freq;
	ALvoid *data;
	alError = AL_NO_ERROR;
    
	NSBundle *bundle = [NSBundle mainBundle];
	
	// Get the audio data from the file which has been passed in
	NSString *fileName = [[aMusicFile lastPathComponent] stringByDeletingPathExtension];
	NSString *fileType = [aMusicFile pathExtension];
    NSString *filePath = [bundle pathForResource:fileName ofType:fileType];
    if ( filePath == nil )
        filePath = aMusicFile;
    
	CFURLRef fileURL = (CFURLRef)[[NSURL fileURLWithPath: filePath] retain];
	
	if (fileURL)
	{	
		data = MyGetOpenALAudioData(fileURL, &size, &format, &freq);
		CFRelease(fileURL);
		
		if((alError = alGetError()) != AL_NO_ERROR) {
			NSLog(@"ERROR - SoundManager: Error loading sound: %@ with error %x\n", fileName, alError);

		}
		
		// Use the static buffer data API
		alBufferData(bufferID, format, data, size, freq);
		[self checkForErrors];
		
		if((alError = alGetError()) != AL_NO_ERROR) {
			NSLog(@"ERROR - SoundManager: Error attaching audio to buffer: %x\n", alError);
		}
		
		// Free the memory we used when getting the audio data
		if (data)
			free(data);
	}
	else
	{
		NSLog(@"ERROR - SoundManager: Could not find file '%@.%@'", fileName, fileType);
		if (data)
			free(data);
		data = NULL;
	}
	
	// Place the buffer ID into the sound library against |aSoundKey|
	[soundLibrary setObject:[NSNumber numberWithUnsignedInt:bufferID] forKey:aSoundKey];
    NSLog(@"INFO - SoundManager: Loaded sound with key '%@' into buffer '%d'", aSoundKey, bufferID);
}

- (void)removeSoundWithKey:(NSString*)aSoundKey {
 
	// Reset errors in OpenAL
	alError = alGetError();
	alError = AL_NO_ERROR;

    // Find the buffer which has been linked to the sound key provided
    NSNumber *numVal = [soundLibrary objectForKey:aSoundKey];
    
    // If the key is not found log it and finish
    if(numVal == nil) {
        NSLog(@"WARNING - SoundManager: No sound with key '%@' was found so cannot be removed", aSoundKey);
        return;
    }
    
    // Get the buffer number from
    NSUInteger bufferID = [numVal unsignedIntValue];
	NSInteger bufferForSource;
	NSInteger sourceState;
	for(NSNumber *sourceID in soundSources) {

		NSUInteger currentSourceID = [sourceID unsignedIntValue];
		
		// Grab the current state of the source and also the buffer attached to it
		alGetSourcei(currentSourceID, AL_SOURCE_STATE, &sourceState);
		[self checkForErrors];
		alGetSourcei(currentSourceID, AL_BUFFER, &bufferForSource);
		[self checkForErrors];

		// If this source is not playing then unbind it.  If it is playing and the buffer it
		// is playing is the one we are removing, then also unbind that source from this buffer
		if(sourceState != AL_PLAYING || (sourceState == AL_PLAYING && bufferForSource == bufferID)) {
			alSourceStop(currentSourceID);
			[self checkForErrors];
			alSourcei(currentSourceID, AL_BUFFER, 0);
			[self checkForErrors];
		}
	} 
    
	// Delete the buffer
	alDeleteBuffers(1, &bufferID);
	
	// Check for any errors
	if((alError = alGetError()) != AL_NO_ERROR) {
		NSLog(@"ERROR - SoundManager: Could not delete buffer %d with error %x", bufferID, alError);
		exit(1);
	}
	
	// Remove the soundkey from the soundLibrary
    [soundLibrary removeObjectForKey:aSoundKey];

    NSLog(@"INFO - SoundManager: Removed sound with key '%@'", aSoundKey);
}


- (void)loadBackgroundMusicWithKey:(NSString*)aMusicKey musicFile:(NSString*)aMusicFile {
	
	// Get the filename and type from the music file name passed in
	NSString *fileName = [[aMusicFile lastPathComponent] stringByDeletingPathExtension];
	NSString *fileType = [aMusicFile pathExtension];
	
    // Check to make sure that a sound with the same key does not already exist
    NSString *path = [musicLibrary objectForKey:aMusicKey];
    
    // If the key is found log it and finish
    if(path != nil) {
        NSLog(@"WARNING - SoundManager: Music with the key '%@' already exists.", aMusicKey);
        return;
    }
    
	path = [[NSBundle mainBundle] pathForResource:fileName ofType:fileType];
	if (!path) {
        if ( ![[NSFileManager defaultManager] fileExistsAtPath: aMusicFile] )
        {
            NSLog(@"WARNING - SoundManager: Cannot find file '%@.%@'", fileName, fileType);
            return;
        }
        else
            path = aMusicFile;
	}
	
	[musicLibrary setObject:path forKey:aMusicKey];
    NSLog(@"INFO - SoundManager: Loaded background music with key '%@'", aMusicKey);
}

- (void)removeBackgroundMusicWithKey:(NSString*)aMusicKey {
    NSString *path = [musicLibrary objectForKey:aMusicKey];
    if(path == NULL) {
        NSLog(@"WARNING - SoundManager: No music found with key '%@' was found so cannot be removed", aMusicKey);
        return;
    }
    [musicLibrary removeObjectForKey:aMusicKey];
    NSLog(@"INFO - SoundManager: Removed music with key '%@'", aMusicKey);
}

- (void)addToPlaylistNamed:(NSString*)aPlaylistName track:(NSString*)aTrackName {

	NSString *path = [musicLibrary objectForKey:aTrackName];
	if (!path) {
		NSLog(@"WARNING - SoundManager: Track '%@' does not exist in the music library and cannot be added to the play list.");
		return;
	}

	// See if the playlist already exists
	NSMutableArray *playlistTracks = [musicPlaylists objectForKey:aPlaylistName];
	
	if (!playlistTracks) {
		playlistTracks = [NSMutableArray arrayWithCapacity: 1];
	}
	
	[playlistTracks addObject:aTrackName];
	
	// Add the track key to the play list
	[musicPlaylists setObject:playlistTracks forKey:aPlaylistName];

}

- (void)startPlaylistNamed:(NSString*)aPlaylistName {

	NSMutableArray *playlistTracks = [musicPlaylists objectForKey:aPlaylistName];
	
	if (!playlistTracks) {
		NSLog(@"WARNING - SoundManager: No play list exists with the name '%@'", aPlaylistName);
		return;
	}

	currentPlaylistName = aPlaylistName;
	currentPlaylistTracks = playlistTracks;
	usePlaylist = YES;
	
	[self playMusicWithKey:[playlistTracks objectAtIndex:0] timesToRepeat:0];
}

- (void)removeFromPlaylistNamed:(NSString*)aPlaylistName track:(NSString*)aTrackName {

	NSMutableArray *playlistTracks = [musicPlaylists objectForKey:aPlaylistName];
	if (playlistTracks) {
		int indexToRemove = -1;
		for (NSUInteger index=0; index < [currentPlaylistTracks count]; index++) {
			if ([[currentPlaylistTracks objectAtIndex:index] isEqualToString:aTrackName]) {
				indexToRemove = index;
				break;
			}
		}
        if (indexToRemove >= 0)
            [currentPlaylistTracks removeObjectAtIndex:indexToRemove];
	}
}

- (void)removePlaylistNamed:(NSString*)aPlaylistName {
	[musicPlaylists removeObjectForKey:aPlaylistName];
}

- (void)clearPlaylistNamed:(NSString*)aPlaylistName {
	NSMutableArray *playlistTracks = [musicPlaylists objectForKey:aPlaylistName];
	
	if (playlistTracks) {
		[playlistTracks removeAllObjects];
	}
}

#pragma mark -
#pragma mark Sound control

- (NSUInteger)playSoundWithKey:(NSString*)aSoundKey gain:(float)aGain pitch:(float)aPitch location:(CGPoint)aLocation shouldLoop:(BOOL)aLoop sourceID:(NSInteger)aSourceID {
	
	// Find the buffer linked to the key which has been passed in
	NSNumber *numVal = [soundLibrary objectForKey:aSoundKey];
	if(numVal == nil) return 0;
	NSUInteger bufferID = [numVal unsignedIntValue];
	
	// Find an available source if -1 has been passed in as the sourceID.  If the sourceID is
    // not -1 i.e. a source ID has been passed in then check to make sure that source is not playing
    // and if not play the identified buffer ID within the provided source
    NSUInteger sourceID;
    if(aSourceID == -1) {
        sourceID = [self nextAvailableSource];
    } else {
        NSInteger sourceState;
        alGetSourcei(aSourceID, AL_SOURCE_STATE, &sourceState);
        if(sourceState == AL_PLAYING)
            return 0;
        sourceID = aSourceID;
    }
	
	// Make sure that the source is clean by resetting the buffer assigned to the source
	// to 0
	alSourcei(sourceID, AL_BUFFER, 0);
    
	// Attach the buffer we have looked up to the source we have just found
	alSourcei(sourceID, AL_BUFFER, bufferID);
	
	// Set the pitch and gain of the source
	alSourcef(sourceID, AL_PITCH, aPitch);
	alSourcef(sourceID, AL_GAIN, aGain * fxVolume);
	
	// Set the looping value
	if(aLoop) {
		alSourcei(sourceID, AL_LOOPING, AL_TRUE);
	} else {
		alSourcei(sourceID, AL_LOOPING, AL_FALSE);
	}
   
	// Set the source location
	alSource3f(sourceID, AL_POSITION, aLocation.x, aLocation.y, 0.0f);
	
	// Now play the sound
	alSourcePlay(sourceID);
    alError = alGetError();

    // Check to see if there were any errors
	[self checkForErrors];
    
	// Return the source ID so that loops can be stopped etc
	return sourceID;
}


- (void)stopSoundWithKey:(NSString*)aSoundKey {

	// Reset errors in OpenAL
	alError = alGetError();
	alError = AL_NO_ERROR;
	
    // Find the buffer which has been linked to the sound key provided
    NSNumber *numVal = [soundLibrary objectForKey:aSoundKey];
    
    // If the key is not found log it and finish
    if(numVal == nil) {
        NSLog(@"WARNING - SoundManager: No sound with key '%@' was found so cannot be stopped", aSoundKey);
        return;
    }
    
    // Get the buffer number from
    NSUInteger bufferID = [numVal unsignedIntValue];
	NSInteger bufferForSource;
	NSInteger sourceState;
	for(NSNumber *sourceID in soundSources) {
		
		NSUInteger currentSourceID = [sourceID unsignedIntValue];
		
		// Grab the current state of the source and also the buffer attached to it
		alGetSourcei(currentSourceID, AL_SOURCE_STATE, &sourceState);
		alGetSourcei(currentSourceID, AL_BUFFER, &bufferForSource);
		
		// If this source is not playing then unbind it.  If it is playing and the buffer it
		// is playing is the one we are removing, then also unbind that source from this buffer
		if(bufferForSource == bufferID) {
			alSourceStop(currentSourceID);
			alSourcei(currentSourceID, AL_BUFFER, 0);
		}
	} 

	// Check for any errors
	[self checkForErrors];
	
	// Remove the soundkey from the soundLibrary
    [soundLibrary removeObjectForKey:aSoundKey];
	
    NSLog(@"INFO - SoundManager: Removed sound with key '%@'", aSoundKey);
}


- (void)playMusicWithKey:(NSString*)aMusicKey timesToRepeat:(NSUInteger)aRepeatCount {
	
	NSError *error;
	
	NSString *path = [musicLibrary objectForKey:aMusicKey];
	
	if(!path) {
		NSLog(@"ERROR - SoundManager: The music key '%@' could not be found", aMusicKey);
		return;
	}
	
	if(musicPlayer)
		[musicPlayer release];
	
	// Initialize the AVAudioPlayer using the path that we have retrieved from the music library dictionary
	musicPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:path] error:&error];
	
	// If the backgroundMusicPlayer object is nil then there was an error
	if(!musicPlayer) {
		NSLog(@"ERROR - SoundManager: Could not play music for key '%d'", error);
		return;
	}
	
	// Set the delegate for this music player to be the sound manager
	musicPlayer.delegate = self;
	
	// Set the number of times this music should repeat.  -1 means never stop until its asked to stop
	[musicPlayer setNumberOfLoops:aRepeatCount];
	
	// Set the volume of the music
	[musicPlayer setVolume:currentMusicVolume];
	
	// Play the music
	[musicPlayer play];
	
	// Set the isMusicPlaying flag
	isMusicPlaying = YES;
}

- (void)playNextTrack {
	if (playlistIndex + 1 == [currentPlaylistTracks count]-1 && loopLastPlaylistTrack) {
		playlistIndex += 1;
		[self playMusicWithKey:[currentPlaylistTracks objectAtIndex:playlistIndex] timesToRepeat:-1];
	} else if (playlistIndex + 1 < [currentPlaylistTracks count]) {
		playlistIndex += 1;
		[self playMusicWithKey:[currentPlaylistTracks objectAtIndex:playlistIndex] timesToRepeat:0];
	} else if (loopPlaylist) {
		playlistIndex = 0;
		[self playMusicWithKey:[currentPlaylistTracks objectAtIndex:playlistIndex] timesToRepeat:0];
	}
}

- (void)stopMusic {
	[musicPlayer stop];
	isMusicPlaying = NO;
	usePlaylist = NO;
}


- (void)pauseMusic {
	if(musicPlayer)
		[musicPlayer pause];
	isMusicPlaying = NO;
}

- (void)resumeMusic {
	if (musicPlayer) {
		[musicPlayer play];
		isMusicPlaying = YES;
	}
}

#pragma mark -
#pragma mark SoundManager settings

- (void)setMusicVolume:(float)aVolume {

	// Set the volume iVar
	if (aVolume > 1)
		aVolume = 1.0f;

	currentMusicVolume = aVolume;
	musicVolume = aVolume;

	// Check to make sure that the audio player exists and if so set its volume
	if(musicPlayer) {
		[musicPlayer setVolume:currentMusicVolume];
	}
}


- (void)setFxVolume:(float)aVolume {
	fxVolume = aVolume;
}


- (void)setListenerPosition:(CGPoint)aPosition {
	listenerPosition = aPosition;
	alListener3f(AL_POSITION, aPosition.x, aPosition.y, 0.0f);
}


- (void)setOrientation:(CGPoint)aPosition {
    float orientation[] = {aPosition.x, aPosition.y, 0.0f, 0.0f, 0.0f, 1.0f};
    alListenerfv(AL_ORIENTATION, orientation);
}

- (void)fadeMusicVolumeFrom:(float)aFromVolume toVolume:(float)aToVolume duration:(float)aSeconds stop:(BOOL)aStop {

	// If there is already a fade timer active, invalidate it so we can start another one
	if (timer) {
		[timer invalidate];
		timer = NULL;
	}
	
	// Work out how much to fade the music by based on the current volume, the requested volume
	// and the duration
	fadeAmount = (aToVolume - aFromVolume) / (aSeconds / kFadeInterval); 
	currentMusicVolume = aFromVolume;

	// Reset the fades duration
	fadeDuration = 0;
	targetFadeDuration = aSeconds;
	isFading = YES;
	stopMusicAfterFade = aStop;
	
	// Set up a timer that fires kFadeInterval times per second calling the fadeVolume method
	timer = [NSTimer scheduledTimerWithTimeInterval:kFadeInterval target:self selector:@selector(fadeVolume:) userInfo:nil repeats:TRUE];
}

- (void)crossFadeTo:(NSString*)aTrack duration:(float)aDuration {
	// TODO: Finish this method
}


#pragma mark -
#pragma mark AVAudioPlayerDelegate

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag {
	isMusicPlaying = NO;

	// If we are using a play list then handle the next track to be played
	if (usePlaylist) {
		[self playNextTrack];
	}
}

#pragma mark -
#pragma mark AVAudioSessionDelegate

- (void)beginInterruption {
	NSLog(@"BEGIN");
	[self setActivated:NO];
}

- (void)endInterruption {
	[self setActivated:YES];
}

@end


#pragma mark -
#pragma mark Private implementation

@implementation SoundManager (Private)

// Define the number of sources which will be created.  iPhone can have a max of 32
#define MAX_OPENAL_SOURCES 16

- (BOOL)initOpenAL {
    NSLog(@"INFO - Sound Manager: Initializing sound manager");
	
    // Get the device we are going to use for sound.  Using NULL gets the default device
	device = alcOpenDevice(NULL);
	
	// If a device has been found we then need to create a context, make it current and then
	// preload the OpenAL Sources
	if(device) {
		// Use the device we have now got to create a context in which to play our sounds
		context = alcCreateContext(device, NULL);
		[self checkForErrors];
        
		// Make the context we have just created into the active context
		alcMakeContextCurrent(context);
		[self checkForErrors];
        
        // Set the distance model to be used
        alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
		[self checkForErrors];
        
		// Pre-create sound sources which can be dynamically allocated to buffers (sounds)
		NSUInteger sourceID;
		for(int index = 0; index < MAX_OPENAL_SOURCES; index++) {
			// Generate an OpenAL source
			alGenSources(1, &sourceID);
			[self checkForErrors];
            
            // Configure the generated source so that sounds fade as the player moves
            // away from them
            alSourcef(sourceID, AL_REFERENCE_DISTANCE, 25.0f);
            alSourcef(sourceID, AL_MAX_DISTANCE, 150.0f);
            alSourcef(sourceID, AL_ROLLOFF_FACTOR, 6.0f);
			[self checkForErrors];
            
			// Add the generated sourceID to our array of sound sources
			[soundSources addObject:[NSNumber numberWithUnsignedInt:sourceID]];
		}
        
        NSLog(@"INFO - Sound Manager: Finished initializing the sound manager");
		// Return YES as we have successfully initialized OpenAL
		return YES;
	}

	// We were unable to obtain a device for playing sound so tell the user and return NO.
    NSLog(@"ERROR - SoundManager: Unable to allocate a device for sound.");
	return NO;
}


- (NSUInteger)nextAvailableSource {
	
	// Holder for the current state of the current source
	NSInteger sourceState;
	
	// Find a source which is not being used at the moment
	for(NSNumber *sourceNumber in soundSources) {
		alGetSourcei([sourceNumber unsignedIntValue], AL_SOURCE_STATE, &sourceState);
		// If this source is not playing then return it
		if(sourceState != AL_PLAYING) return [sourceNumber unsignedIntValue];
	}
	
	// If all the sources are being used we look for the first non looping source
	// and use the source associated with that
	NSInteger looping;
	for(NSNumber *sourceNumber in soundSources) {
		alGetSourcei([sourceNumber unsignedIntValue], AL_LOOPING, &looping);
		if(!looping) {
			// We have found a none looping source so return this source and stop checking
			NSUInteger sourceID = [sourceNumber unsignedIntValue];
			alSourceStop(sourceID);
			return sourceID;
		}
	}
	
	// If there are no looping sources to be found then just use the first source and use that
	NSUInteger sourceID = [[soundSources objectAtIndex:0] unsignedIntegerValue];
	alSourceStop(sourceID);
	
	// Check for any errors that may have been raised
	[self checkForErrors];
	
	// Return the sourceID found
	return sourceID;
}


#pragma mark -
#pragma mark Interruption handling

- (void)setActivated:(BOOL)aState {
    
    OSStatus result;
    
    if(aState) {
        NSLog(@"INFO - SoundManager: OpenAL Active");
        
        // Set the AudioSession AudioCategory to what has been defined in soundCategory
		[audioSession setCategory:soundCategory error:&audioSessionError];
        if(audioSessionError) {
            NSLog(@"ERROR - SoundManager: Unable to set the audio session category");
            return;
        }
        
        // Set the audio session state to true and report any errors
		[audioSession setActive:YES error:&audioSessionError];
		if (audioSessionError) {
            NSLog(@"ERROR - SoundManager: Unable to set the audio session state to YES with error %@.", [audioSessionError localizedDescription]);
            return;
        }
		
		if (musicPlayer) {
			[musicPlayer play];
		}
        
        // As we are finishing the interruption we need to bind back to our context.
        alcMakeContextCurrent(context);
		[self checkForErrors];
    } else {
        NSLog(@"INFO - SoundManager: OpenAL Inactive");
        
        // As we are being interrupted we set the current context to NULL.  If this sound manager is to be
        // compaitble with firmware prior to 3.0 then the context would need to also be destroyed and
        // then re-created when the interruption ended.
        alcMakeContextCurrent(NULL);
		[self checkForErrors];
    }
}


- (BOOL)isAudioPlaying {
	
	UInt32 audioPlaying = 0;
	UInt32 audioPlayingSize = sizeof(audioPlaying);
	
	AudioSessionGetProperty(kAudioSessionProperty_OtherAudioIsPlaying, &audioPlayingSize, &audioPlaying);
	
	return (BOOL)audioPlaying;
}


- (void)fadeVolume:(NSTimer*)aTimer {
	fadeDuration += kFadeInterval;
	if (fadeDuration > targetFadeDuration) {
		if (timer) {
			[timer invalidate];
			timer = NULL;
		}

		isFading = NO;
		if (stopMusicAfterFade) {
			[musicPlayer stop];
			isMusicPlaying = NO;
		}
	} else {
		currentMusicVolume += fadeAmount;
	}
	
	// If music is current playing then set its volume
	if(isMusicPlaying) {
		[musicPlayer setVolume:currentMusicVolume];
	}
}

- (void)checkForErrors {
	alError = alGetError();
	if(alError != AL_NO_ERROR) {
		NSLog(@"ERROR - SoundManager: OpenAL reported error '%d'", alError);
	}
}

@end

