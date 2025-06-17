#ifndef  MSGEN_H
#define  MSGEN_H 1

#include  <map>
#include  <string>

#include  <MidiFile.h>

class  MIDIScoreGenerator
{
	private:
		enum class  EventType
		{
			TEMPO,
			NOTE_ON, 
			NOTE_OFF,
			PROGRAM, 
			PITCH_BEND,
			CC
		};

		struct  event 
		{
			int         time;
			EventType   type;
			int        iargs[ 3 ];
			double     dargs[ 3 ];

			event( int t, EventType type ): time(t), type(type) {};
			event( int t, EventType type, double x ): time(t), type(type), dargs{x} {};
			event( int t, EventType type, int x, int y ): time(t), type(type), iargs{x, y} {};
			event( int t, EventType type, int x, double y ): time(t), type(type), iargs{x}, dargs{y} {};
			event( int t, EventType type, int x, int y, int z ): time(t), type(type), iargs{x, y, z} {};
		};

		std::multimap<int, event>  events;
		int  tpq;
		int  track;
		double  tempo;

		void  addMidiFileEvent( smf::MidiFile&, const event& ) const;
	public:
		MIDIScoreGenerator( int ticks_per_quarter ): tpq(ticks_per_quarter), tempo( 120 ) {};

		void  setTPQ( int ticks_per_quarter ) { tpq = ticks_per_quarter; };
		void  setTempo( double time, double tempo );

		void  noteOn( double time, int chan, int note, int vel );
		void  noteOff( double time, int chan, int note, int vel );
		void  note( double time_on, double time_off, int chan, int note, int vel );
		void  program( double time, int chan, int prog );

		void  pitchBend( double time, int chan, double amount );
		void  cc( double time, int chan, int ctl, int value );
		void  volume( double time, int chan, double vol );

		bool  write( const std::string &filename );
};

#endif
