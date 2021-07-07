//21 GECS

#include <MozziGuts.h>
#include <Oscil.h>
//#include <tables/smoothsquare8192_int8.h>
#include <tables/cos8192_int8.h>
#include <tables/triangle_warm8192_int8.h>
#include <mozzi_midi.h>
#include <mozzi_rand.h>
#include <AutoMap.h>
#include <IntMap.h>

// Tone oscs
//Oscil<8192, AUDIO_RATE> aSqr(SMOOTHSQUARE8192_DATA);
Oscil<8192, AUDIO_RATE> aCos(COS8192_DATA);
Oscil<8192, AUDIO_RATE> aTri(TRIANGLE_WARM8192_DATA);
Oscil<8192, AUDIO_RATE> aTri2(TRIANGLE_WARM8192_DATA);

Oscil<8192, AUDIO_RATE> *oscs[3]{&aCos, &aTri, &aTri2};

#define POT_VOL A0
#define POT_CHORD A1
#define POT_NOTE A2

const uint8_t majorChrd[3]{0, 4, 7};
const uint8_t minorChrd[3]{0, 3, 7};
const uint8_t dimChrd[3]{0, 3, 5};

const uint8_t *majScaleChrds[8]{
	majorChrd, minorChrd,
	minorChrd, majorChrd,
	majorChrd, minorChrd,
	dimChrd, majorChrd};
const uint8_t majScaleIntervals[8]{0, 2, 4, 5, 7, 9, 11, 12};

IntMap noteMap{0, 1023, 36, 84};
IntMap knobMap{0, 1023, 0, 255}; // Solution is to add one value above and bounds check in update

uint8_t volume = 0;

void setup()
{
	delay(200);
	startMozzi();
}

void updateControl()
{
	// Find chord in scale off POT_CHORD
	uint8_t scaleVal = mozziAnalogRead(POT_CHORD) / 128; // convert to a number between 0 and 7
	scaleVal = scaleVal == 8 ? 7 : scaleVal;			 // bounds checking in case of int math fudgery

	// Find base note off POT_NOTE
	uint8_t mapVal = noteMap(mozziAnalogRead(POT_NOTE));
	// Adjust root note for scale interval
	mapVal += majScaleIntervals[scaleVal];
	// Tune oscs to chord based on scale
	for (uint8_t i = 0; i < 3; ++i)
	{
		oscs[i]->setFreq(mtof(mapVal + majScaleChrds[scaleVal][i]));
	}
	// set volume off POT_VOL
	volume = knobMap(mozziAnalogRead(POT_VOL));
}

int updateAudio()
{
	int32_t audioPool{};
	for (int8_t i = 0; i < 3; ++i)
		audioPool += (int32_t)oscs[i]->next();

	audioPool >>= 2; // bring the gain down to prevent clipping

	return audioPool * volume >> 8; // shift back into range after multiplying by volume, which is an 8 bit number
}

void loop()
{
	audioHook();
}