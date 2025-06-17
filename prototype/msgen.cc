#include  "msgen.h"

void  MIDIScoreGenerator::addMidiFileEvent( smf::MidiFile &midi, const event &e ) const
{
	switch ( e.type )
	{
		case  EventType::TEMPO:
			midi.addTempo( track, e.time, e.dargs[0] );
			break;
		case  EventType::NOTE_ON:
			midi.addNoteOn( track, e.time, e.iargs[0], e.iargs[1], e.iargs[2] );
			break;
		case  EventType::NOTE_OFF:
			midi.addNoteOff( track, e.time, e.iargs[0], e.iargs[1], e.iargs[2] );
			break;
		case  EventType::PROGRAM:
			midi.addPatchChange( track, e.time, e.iargs[0], e.iargs[1] );
			break;
		case  EventType::PITCH_BEND:
			midi.addPitchBend( track, e.time, e.iargs[0], e.dargs[0] );
			break;
		case  EventType::CC:
			midi.addController( track, e.time, e.iargs[0], e.iargs[1], e.iargs[2] );
			break;
	}
}

bool  MIDIScoreGenerator::write( const std::string &filename )
{
	smf::MidiFile f;

	f.setTPQ( tpq );
	track = f.addTrack();

	f.addTempo( track, 0, tempo );

	for ( const auto &e: events )
		addMidiFileEvent( f, e.second );

	f.sortTracks();
	return  f.write( filename );
}

void  MIDIScoreGenerator::setTempo( double time, double tempo )
{
	int  t = tpq * time;
	events.emplace( t, event{t, EventType::TEMPO, tempo} );
}

void  MIDIScoreGenerator::noteOn( double time, int chan, int note, int vel )
{
	int  t = tpq * time;
	events.emplace( t, event{t, EventType::NOTE_ON, chan, note, vel} );
}

void  MIDIScoreGenerator::noteOff( double time, int chan, int note, int vel )
{
	int  t = tpq * time;
	events.emplace( t, event{t, EventType::NOTE_OFF, chan, note, vel} );
}
		
void  MIDIScoreGenerator::note( double time_on, double time_off, int chan, int note, int vel )
{
	noteOn( time_on, chan, note, vel );
	noteOff( time_off, chan, note, 127 );
}

void  MIDIScoreGenerator::program( double time, int chan, int prog )
{
	int  t = tpq * time;
	events.emplace( t, event{t, EventType::PROGRAM, chan, prog} );
}

void  MIDIScoreGenerator::pitchBend( double time, int chan, double amount )
{
	int  t = tpq * time;
	events.emplace( t, event{t, EventType::PITCH_BEND, chan, amount} );
}

void  MIDIScoreGenerator::cc( double time, int chan, int ctl, int value )
{
	int  t = tpq * time;
	events.emplace( t, event{t, EventType::CC, chan, ctl, value} );
}

void  MIDIScoreGenerator::volume( double time, int chan, double amount )
{
	cc( time, chan, 7, amount * 127 );
}
