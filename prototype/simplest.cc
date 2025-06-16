const char *usage_string = 
"simplest:  Simplest possible MIDI score compiler\n"
"\n"
"Usage   :  simplest  [options] [source]\n"
"Options :        -h              - this help\n"
"                 -o filename     - output MIDI file name (default: simplest.mid)\n"
"Input syntax:\n"
"\n"
"# Comments ...\n"
"   tpq       960                   #  set ticks per quarter (division) to 960\n"
"   2.5       EVENT                 #  generate EVENT at time = quarter (not bar!) 2 and half\n"
"\n"
"Events (at time 1.0):\n"
"   1.0       cc         2  13  80    #  set controller - channel 2, controller 13,  value 80\n"
"   1.0       noteoff    2  70  127   #  note off - channel 2,  pitch 70,  velocity 127\n"
"   1.0       noteon     2  70  127   #  note on - channel 2,  pitch 70,  velocity 127\n"
"   1.0  note   1.2      2  70  127   #  note on at time 1.0, note off at 1.2\n"
"   1.0       program    2  1         #  program channel 2 to instrument 1\n"
"   1.0       pitchbend  2  0.9       #  set pitchbend - channel 2, pitchbend 0.9\n"
"   1.0       tempo  120              #  set tempo 120\n"
"   1.0       volume     2  0.9       #  set volume - channel 2, volume 0.9\n"
"\n";

#include  <iostream>
#include  <unistd.h>

#include  "msgen.h"
#include  "tokenizer.h"

std::string  out_filename("simplest.mid");
std::string  in_filename;

int  tpq = 960;

int  generate( std::istream &input, const std::string &fname )
{
	std::string  buffer;

	int  line = 0;

	MIDIScoreGenerator  midi( tpq );

	while ( std::getline( input, buffer ) )
	{
		line++;

		auto  toks = tokenizer::tokenize( buffer, " \t\r\n" );

		if ( toks.empty() || toks.front().text.front()=='#' )
			continue;

		const auto &cmd = toks.front().text;
		if ( cmd == "tpq" )  
		{
			tpq = atoi( toks.at(1).text.c_str() );
			midi.setTPQ( tpq );
		}
		else 
		{
		        double t =  atof( cmd.c_str() );

			const auto &ev = toks.at(1).text;

#define   FARG( I )  atof( toks.at(I).text.c_str() )
#define   IARG( I )  atoi( toks.at(I).text.c_str() )
			if ( ev == "cc" )
				midi.cc( t, IARG(2), IARG(3), IARG(4) );
			else if ( ev == "note" )
				midi.note( t, FARG(2), IARG(3), IARG(4), IARG(5) );
			else if ( ev == "noteon" )
				midi.noteOn( t, IARG(2), IARG(3), IARG(4) );
			else if ( ev == "noteoff" )
				midi.noteOff( t, IARG(2), IARG(3), IARG(4) );
			else if ( ev == "tempo" )
				midi.setTempo( t, FARG(2) );
			else if ( ev == "pitchbend" )
				midi.pitchBend( t, IARG(2), FARG(3) );
			else if ( ev == "program" )
				midi.program( t, IARG(2), IARG(3) );
			else if ( ev == "volume" )
				midi.volume( t, IARG(2), FARG(3) );
			else
				std::cout << "Unrecognized event '" << ev << "' in line " << line << '\n';
		}
	}

	return  !midi.write( fname );
#undef    IARG
#undef    FARG	
}

int  main( int argc, char *argv[] )
{
	int op = 0;
	while( op >= 0 )
	{
		op = getopt( argc, argv, "ho:" );
		switch ( op )
		{
			case 'h':
				std::cout << usage_string;
				return 0;
			case 'o':
				out_filename = optarg;
				break;
		}
	}

	if ( optind < argc )
		in_filename = argv[ optind ];

	if ( in_filename.empty() )
		return  generate( std::cin, out_filename );
	else
	{
		std::ifstream  f( in_filename );
		return  generate( f, out_filename );
	}
}
