#ifndef  TOKENIZER_H
#define  TOKENIZER_H

#include  <string>
#include  <vector>

namespace  tokenizer  {
	struct  token {
		std::string  text;
		int  start;
		int  end;
	};

	std::vector<token>  tokenize( const std::string&, const std::string &separators );
};

#endif
