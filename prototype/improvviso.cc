#include  <cstring>
#include  <fstream>
#include  <iostream>
#include  <map>
#include  <optional>
#include  <string>
#include  <sstream>
#include  <variant>
#include  <unistd.h>

#include  "repars.h"

/*  Range from which values are randomized */
using  AttrRange = std::pair<double, double>;

/*  Current tempo  */
static  double  tempo = 120;

/*  Pitch value in the top-most row
 *  (in MIDI units)
 */
int  starting_pitch = 56;

//  Decreasing pitch according to diatonic scale steps
int  prev_pitch( int p )
{
	int  normalized = p%12;
	int  delta = 1;

	switch (normalized)
	{
		case  11:  // H (B)
		case  9:   // A
		case  7:   // G
		case  4:   // E
		case  2:   // D
			delta = 2;
	}

	if ( p > delta )
		return  p - delta;
	else
		return  p;
}

/*  Attribute definition  */
struct  AttrValue
{
        // modifier/operator  ("=", "+=", "*=", ...?)
	std::string  operation;

        // expression value
	std::variant<std::string, double, AttrRange>  expression;

	public:
		AttrValue() {};
		AttrValue( const std::string &op, const std::string &val ): operation(op), expression(val) {};
		AttrValue( const std::string &op, double x ): operation(op), expression{x} {};
		AttrValue( const std::string &op, const AttrRange &r ): operation(op), expression{r} {};

		std::string  getString() const 
		{
			if ( std::holds_alternative<std::string>( expression ) )
				return  std::get<std::string>( expression );
			else
				return  "";
		}
};

/*  Instrument definition - a set of attributes and their values  */
using  InstrDef = std::map<std::string, AttrValue>;

const char *omit_white( const char *text )
{
	while ( text && text[0] && isspace(text[0]) )
		text++;

	return  text;
}

/*  Parsing of an attribute value as a number or a range {number - number}.
 */
const char *parseAttrValue( const char *text, const std::string &oper, AttrValue &v )
{
	if ( !( text && text[0] ))
		return  text;

	text = omit_white( text );

	if ( isdigit(text[0]) || text[0]=='-' )
	{
		char *endptr;
		double  x = strtod( text, &endptr );

		v = AttrValue( oper, x );
		return  endptr;
	}
	else if ( text[0] == '{' )
	{
		char *endptr;
		double x, y;

		text++;
		text = omit_white( text );
		if ( !( text && text[0]) )  return  nullptr;

		x = strtod( text, &endptr );

		text = endptr;
		text = omit_white( text );
		if ( !( text && text[0]) )  return  nullptr;

		if ( text[0] != '-' )  return nullptr;

		text++;
		text = omit_white( text );
		if ( !( text && text[0]) )  return  nullptr;

		y = strtod( text, &endptr );
		text = endptr;

		text = omit_white( text );
		if ( !( text && text[0]) )  return  nullptr;

		text = strchr( text, '}' );
		if ( !( text && text[0]) )  return  nullptr;

		text++;

		v = AttrValue( oper, (AttrRange)std::make_pair(x, y) );
		return  text;
	}
	else
	{
		std::string  s;
	        s += text[0];

		while ( *text )
		{
			text++;

			if ( isspace(*text) || ispunct(*text) || !*text )
				break;

			s += *text;
		}

		v = AttrValue( oper, s );
		return  text;
	}

	return  text;
}

/*  Parsing of an attribute as a name + AttrValue pair,
 *  in the format   name ["="|"+="|"*="] value.
 */
const char *parseAttribute( const char *text, std::pair<std::string, AttrValue> &a )
{
	text = omit_white( text );
	if ( !text ) return text;

	std::string  name;
	while ( text && text[0] )
		if ( isalpha(text[0]) )
		{
			name += text[ 0 ];
			text++;
		}
		else
			break;

	text = omit_white( text );
	if ( !text ) return text;

	std::string  op;
	while ( text && strchr("=+*", text[0]) )
	{
		op += text[0];
		text++;
	}

	AttrValue  v;
	text = parseAttrValue( text, op, v );

	a = {name, v};
	return  text;
}

/*  Parsing of the string  CODE ":" instrument definition.
 *  Returns a {name, InstrDef} pair if successful.
 */
std::optional<std::pair<char, InstrDef>>   parseInstrument( const char *text )
{
	std::optional<std::pair<char, InstrDef>>  result;
	char  code;

	if ( !(text && text[0]) )  return  result;
	text = omit_white( text );

	if ( !(text && text[0]) )  return  result;
	code = text[0];

	text = strchr( text, ':' );
	if ( !(text && text[0]) )  return  result;

	text = omit_white( text + 1 );
	if ( !(text && text[0]) )  return  result;

	InstrDef  idef;

	while( text && text[0] )
	{
		text = omit_white( text );
		if ( !(text && text[0]) )  break;

		std::pair<std::string, AttrValue>  attr;

		text = parseAttribute( text, attr );
		if ( !attr.first.empty() )  
			idef[ attr.first ] = attr.second;

		text = strchr( text, ',' );
		if ( text ) text++;
	}

	result = {code, idef};
	return  result;
}

AttrValue  get_attr_value( const InstrDef &attrs, 
			   const std::string &name,
			   const AttrValue &defValue )
{
	for ( size_t len=name.size(); len>0; len-- )
	{
		std::string  used_name = name.substr(0, len);
		if ( attrs.count( used_name ))
			return  attrs.at( used_name );
	}

	return  defValue;
}

double  random_fraction()
{
	return   ((double)rand()) / ((double)RAND_MAX);
}

double  generate_random( const AttrRange &r )
{
	return  r.first + random_fraction() * (r.second - r.first);
}

double  get_expression_value( const AttrValue &a )
{
	if ( std::holds_alternative<double>( a.expression ))
		return  std::get<double>( a.expression );
	else if ( std::holds_alternative<AttrRange>( a.expression ))
		return  generate_random( std::get<AttrRange>( a.expression ));
	return  -1;
}

double  apply_attribute( double init, const AttrValue &a )
{
	if ( a.operation == "+=" )
		return  init + get_expression_value( a );
	else if ( a.operation == "*=" )
		return  init * get_expression_value( a );
	else
		return  get_expression_value( a );
}

std::string  generate_events( double tick, int pitch, const InstrDef &attrs )
{
	AttrValue  ins = get_attr_value( attrs, "instrument", {"=",1} );
	AttrValue  start = get_attr_value( attrs, "start", {"+=",0} );
	AttrValue  duration = get_attr_value( attrs, "duration", {"=",-1} );
	AttrValue  velocity = get_attr_value( attrs, "velocity", {"=", 127} );
	AttrValue  channel = get_attr_value( attrs, "channel", {"=", 10} );
	AttrValue  type = get_attr_value( attrs, "type", {"=", "percussion"} );

	auto s = ins.getString();
	if ( s == "tempo" )
	{
		double  t = apply_attribute( tick, start );
		double ve = apply_attribute( tempo, velocity );
		tempo = ve;

		std::ostringstream  s;
		s << t << " tempo " <<  ve;

		return  s.str();
	}
	
	int     i = apply_attribute( 0, ins );

	if ( i > 0 )
	{
		std::ostringstream  s;

		double  t = apply_attribute( tick, start );
		double du = apply_attribute( -1, duration );
		double ve = apply_attribute( 127, velocity );
		double ch = apply_attribute( 0, channel );

		auto  typ = type.getString();
	
		if ( typ == "percussion" )
		{
			s << t << " noteon " << ch << ' ' << i << " " << ve;
			if ( du >= 0 )
				s << '\n' << t+du << " noteoff " << ch << ' ' << i << " 127";
		}
		else
		{
			s << t << " noteon " << ch << ' ' << pitch << " " << ve;
			if ( du >= 0 )
				s << '\n' << t+du << " noteoff " << ch << ' ' << pitch << " 127";
		}

		return  s.str();
	}

	return "";
}

void  interprete_command( const std::string &buf )
{
	repars  rp( "[ \t]*pitch[ \t]*=[ \t]*([0-9]+).*" );

	auto  res = rp( buf );
	if ( res )
		starting_pitch = atoi( res[1].c_str() );
}

int main( int argc, char *argv[] )
{
	int  op = 0;
	std::istream *input = nullptr;

	while ( op >= 0 )
	{
		op = getopt( argc, argv, "h" );
		switch ( op )
		{
			case 'h':
				std::cout << "Usage:  improvviso [inputfile]\n";
				return 0;
			case -1:
				break;
			default:
				return 1;
		}
	}

	if ( optind < argc )
		input = new std::ifstream( argv[optind] );
	else
		input = &std::cin;

	double  y = 0;
	double  staff_start_time = 0;
	int     staff_length = 0;
	int     total_length = 0;

	std::string  buf;
	std::map<char, InstrDef>  definitions;

	std::cout << "# generated by 'improvviso'\n"
		"0 tempo " << tempo << "\n"
		"0 cc       10      0 1\n"
		"0 cc       10     32 0\n"
		"0 program  10   0\n\n";

	bool  score_started = false;
	int   pitch = starting_pitch;  // some default value

	while ( std::getline(*input, buf) )
	{
		if ( buf.empty() ) buf = " ";

		if ( buf[0] == '+' || buf[0] == '-' )
		{
			score_started = true;

			if ( buf.substr(1, 3) == "END" )
			{
				total_length += staff_length;
				staff_start_time = total_length;
				score_started = false;
			}
			else
			{
				repars  staff( "([+-]+).*" );
				auto  res = staff( buf );
				if ( res )
					staff_length = res[1].size();
			}

			pitch = starting_pitch;  // some default value
			continue;
		}

		if ( score_started )
		{
			y = staff_start_time;

			//  accidentals 
			std::map<int, int>  accidental;

			for ( const auto c: buf )
			{
				if ( definitions.count(c) )
				{
					int  a = 0;

					if ( accidental.count(pitch) )
					{
						a = accidental.at( pitch );
						accidental.erase( pitch );
					}

					std::cout << 
						generate_events( y, pitch+a, definitions.at(c) ) 
						<< '\n';
				}
				else if ( c=='#' )
					accidental[pitch] = 1;
				else if ( c=='b' )
					accidental[pitch] = -1;

				y++;
			}

			pitch = prev_pitch( pitch );
		}
		else
		{
			const char *delim = strchr( buf.c_str(), ':' );
			
			if ( delim )
			{
				const auto def = parseInstrument( buf.c_str() );
				if ( def )
				{
					AttrValue  chval = get_attr_value( def->second, "channel", 
							{"=", 10} );			
					AttrValue  ins = get_attr_value( def->second, "instrument", 
									{"=", 1} );

					int ch = apply_attribute( 0, chval );
					int i = apply_attribute( 0, ins );

    					//  for now, treating 10 separately, probably the simplest way
					if ( ch != 10 )
						std::cout << y << " program " << ch << ' ' << i << '\n';

					definitions[ def->first ] = def->second;
				}
			}
			else
			{
				delim = strchr( buf.c_str(), '=' );
				if ( delim )
					interprete_command( buf );
			}
		}
	}

	return 0;
}
