CFLAGS=-g -Wall
CXXFLAGS=-g -Wall -std=c++17

PROGRAMS=simplest improvviso

MIDIFILE_DIR=./MidiFile
MIDIFILE_OBJS=Binasc.o MidiEventList.o MidiEvent.o MidiFile.o MidiMessage.o Options.o

INCMIDIFILE=-I$(MIDIFILE_DIR)/include

all: $(PROGRAMS)

simplest: simplest.o tokenizer.o msgen.o $(MIDIFILE_OBJS)
	$(CXX) -o $@ $^

improvviso: improvviso.o repars.o
	$(CXX) -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCMIDIFILE) -c $< -o $@

%.o: $(MIDIFILE_DIR)/src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCMIDIFILE) -c $< -o $@

clean:
	rm -f *.o $(PROGRAMS)
