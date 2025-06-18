WAVFILES=whole.wav imprtest.wav
MP3FILES=$(WAVFILES:.wav=.mp3)

FLUIDSYNTH=fluidsynth
SOUNDFONT=/usr/share/sounds/sf2/FluidR3_GM.sf2
SIMPLEST=../simplest
IMPROVVISO=../improvviso

all: $(MP3FILES)

whole.wav: whole.mid

imprtest.wav: imprtest.mid

%.wav: %.mid
	$(FLUIDSYNTH) -o synth.midi-bank-select=mma $(SOUNDFONT) $< -F $@

%.mid: %.sim
	$(SIMPLEST) $< -o $@

%.sim: %.imp
	$(IMPROVVISO) $< > $@

%.mp3: %.wav
	ffmpeg -i $< -codec:a libmp3lame -qscale:a 2 $@   # High quality, medium size

clean:
	rm -f *.sim *.mid *.wav *.mp3
