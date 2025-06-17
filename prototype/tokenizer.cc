#include  "tokenizer.h"
#include  <cstring>

using  namespace  tokenizer;

std::vector<token>  tokenizer::tokenize( const std::string &data, const std::string &separators )
{
	std::vector<token>  tokens;

	token  t;
	bool   t_started = false;

	for ( size_t i=0; i<data.size(); i++ ) 
	{
		if ( strchr( separators.c_str(), data[i] ))
		{
			if ( t_started )
			{
				t.end = i;
				tokens.push_back( t );

				t.start = t.end = 0;
				t.text.clear();
				t_started = false;
			}
		}
		else
		{
			if ( t_started )
			{
				t.end++;
				t.text += data[i];
			}
			else
			{
				t.text.clear();
				t.text += data[i];
				t.start = i;
				t.end = i+1;
				t_started = true;
			}
		}
	}

	if ( t_started )
		tokens.push_back( t );

	return  tokens;
}
