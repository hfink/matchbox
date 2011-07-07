This project contains sources to efficiently extract 
Mel-Frequency-Cepstral-Coefficients from a given audio stream on Apple iOS / OSX.

It further includes a game prototype called "MatchBox" which applies dynamic time warping
on these MFCC features in order to compare two spoken words for similarity.

For more information, please visit http://hfink.eu/matchbox

Content Description of folders: 

./boost_ext

Contains headers and precompiled libraries of the boost testing framework.

./matlab

Contains a prototype implementation of dynamic-time-warping and comparison
tests for MFCC implementations. In order to run all included matlab test and
prototype files, you will need to install both of these libraries: 

    AuditoryToolbox: This toolbox is included in the MIR toolbox which can
        be downloaded from here: 
        https://www.jyu.fi/hum/laitokset/musiikki/en/research/coe/materials/mirtoolbox

    RastaMAT's mfcc implementation:
        http://www.ee.columbia.edu/~dpwe/resources/matlab/rastamat/

./samples

Contains various (self recorded) speech samples which are used in some unit
tests of the CPP implementations.

./Xcode

This folder contains the actual Xcode project and source files. The following
projects are included within this folder: 

    WordMatch

    This is a simple C/C++ library that is compatible with OS X and
    iOS. It contains an implementation to calculate 
    Mel-Frequency-Cepstral-Coefficients, an implementation of dynamic time
    warping, and some utility classes in order to access audio files.

    This project also contains unit tests that work on both platforms, iOS and
    OS X.

    This library also exposes a clean C-API (WordMatchSession) that 
    demonstrates a session-based MFCC extraction for audio being accessible
    via the CoreMedia frameworks (currently only avaiable on > iOS 4). See
    "iPodLibMFCCTest" for a usecase of this API.

    SimodOne

    An iOS application for recording two audio files and measuring their 
    similarity. This application was mainly built in order to test the real 
    field performance of the WordMatch library.

    MatchBox

    An iOS game prototype similar to "I see with my little eye."

    iPodLibMFCCTest

    A prototype iOS app that demonstrates the following use-cases: 

    	Accessing audio data from songs contained in your iPod library
	efficiently.

	Analyzing chunks of audio data using MFCC calculations (see
	WordMatchSession API).

	Both tasks have been implemented in a highly-efficient manner.
