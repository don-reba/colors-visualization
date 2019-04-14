#include "Script.h"

#ifdef _MSC_VER
#pragma warning(disable : 4503)
#endif

#pragma warning(push)
#pragma warning(disable : 4459)
#pragma warning(disable : 4100)
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/home/qi.hpp>
#pragma warning(pop)

#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <iterator>

namespace qi = boost::spirit::qi;

using std::string;

BOOST_FUSION_ADAPT_STRUCT
	(Script,
	(string,     meshPath)
	(string,     outputPath)
	(Resolution, res)
	(AAMask,     aamask)
	(float,      fps)
	(float,      duration)
	(FrameSet,   frames)
	(bool,       printFrameInfo)
	)

namespace
{
	template<typename Iterator>
	struct ScriptGrammar : qi::grammar<Iterator, Script()>
	{
		ScriptGrammar() : ScriptGrammar::base_type(script)
		{
			using namespace qi;

			resolution.add("2160p", res2160p);
			resolution.add("1080p", res1080p);
			resolution.add("720p",  res720p);
			resolution.add("576p",  res576p);
			resolution.add("360p",  res360p);

			aamask.add("1x", aa1x);
			aamask.add("4x", aa4x);

			sp = *(char_(' ') | char_('\t'));

			sep = sp >> ':' >> sp;

			framesAll   = lit("all") [_val = FramesAll()];
			frameRange %= uint_ >> sp >> '-' >> sp >> uint_;
			frameIndex %= uint_;
			frameSet   %= framesAll | frameRange | frameIndex;

			path %= '"' >> +(char_ - '"') >> '"';

			script
				%= lit("mesh-path")        >> sep >> path       >> eol
				>> lit("output-path")      >> sep >> path       >> eol
				>> lit("resolution")       >> sep >> resolution >> eol
				>> lit("antialiasing")     >> sep >> aamask     >> eol
				>> lit("fps")              >> sep >> float_     >> eol
				>> lit("duration")         >> sep >> time       >> eol
				>> lit("frames")           >> sep >> frameSet   >> eol
				>> lit("print-frame-info") >> sep >> bool_      >> eol;

			time
				= (float_ >> "s")                          [_val = _1]
				| (float_ >> "min" >> sp >> float_ >> "s") [_val = _1 * 60.0f + _2]
				| (float_ >> "min")                        [_val = _1 * 60.0f];

		}

		qi::symbols<char, Resolution> resolution;
		qi::symbols<char, AAMask>     aamask;

		qi::rule<Iterator> sp;
		qi::rule<Iterator> sep;

		qi::rule<Iterator, FramesAll()>  framesAll;
		qi::rule<Iterator, FrameRange()> frameRange;
		qi::rule<Iterator, FrameIndex()> frameIndex;
		qi::rule<Iterator, FrameSet()>   frameSet;

		qi::rule<Iterator, string()> path;
		qi::rule<Iterator, Script()> script;
		qi::rule<Iterator, float()>  time;
	};
}

Script LoadScript(const char * path)
{
	using FileIterator   = std::istream_iterator<char>;
	using StringIterator = string::const_iterator;

	std::ifstream file(path);
	if (!file)
		throw std::invalid_argument("File not found.");
	file.unsetf(std::ios::skipws);

	string text{FileIterator(file), FileIterator()};
	StringIterator i   (text.begin());
	StringIterator end (text.end());

	Script                        script;
	ScriptGrammar<StringIterator> grammar;

	bool isMatch = qi::parse(i, end, grammar, script);
	if (!isMatch || i != end)
		throw std::runtime_error("Parsing failed.");

	return script;
}