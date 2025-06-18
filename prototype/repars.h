#ifndef   REPARS_H
#define   REPARS_H   1

#include  <regex>
#include  <string>

//  Simple parser based on regular expressions,
//  very thin wrapper for std::regex
class  repars
{
	public:

		//  Result returned from parsing
		class  result
		{
			public:
				//  Return true if the match was successfull
				operator bool() const;

				//  Numer of submatches
				size_t count() const;

				//  0 is the whole match, 1, 2... are subexpressions in ()
				std::string operator[]( size_t ) const;

				friend  class  repars;
			private:
				std::smatch  match;
		};

		repars() = default;

		//  Create repars from textual from of regular expression
		repars( const std::string &re );

		//  Parse
		result  operator() ( const std::string &text );

	private:
		std::regex  re;
};

#endif
