#include  "repars.h"

repars::repars( const std::string &re ): re( re ) 
{
};

repars::result  repars::operator() ( const std::string &text )
{
	result  r;

	std::regex_match( text, r.match, re );
	return  r;
}

repars::result::operator  bool() const 
{
	return  match.ready() && match.size()>0;
}

size_t  repars::result::count() const
{
	return  match.size() ? match.size()-1: 0;
}

std::string repars::result::operator[]( size_t n ) const
{
	return  match[ n ];
}
