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

using namespace std;

namespace qi    = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

BOOST_FUSION_ADAPT_STRUCT
	( Script,
	(std::string, meshPath)
	(Resolution,  res)
	(AAMask,      aamask)
	(float,       fps)
	(float,       duration)
	(FrameSet,    frames)
	)

namespace
{
	template<typename Iterator>
	struct ScriptGrammar : qi::grammar<Iterator, Script()>
	{
		ScriptGrammar() : ScriptGrammar::base_type(script)
		{
			using qi::_1;
			using qi::_2;
			using qi::_val;
			using qi::char_;
			using qi::eol;
			using qi::float_;
			using qi::lit;
			using qi::space;
			using qi::uint_;

			resolution.add("4k",    res4k);
			resolution.add("1080p", res4k);
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
				%= lit("mesh-path")  >> sep >> path       >> eol
				>> lit("resolution") >> sep >> resolution >> eol
				>> lit("aamask")     >> sep >> aamask     >> eol
				>> lit("fps")        >> sep >> float_     >> eol
				>> lit("duration")   >> sep >> time       >> eol
				>> lit("frames")     >> sep >> frameSet   >> eol;

			time
				= (float_ >> "s")   [_val = _1]
				| (float_ >> "min") [_val = _1 * 60.0f];

		}

		qi::rule<Iterator> sp;
		qi::rule<Iterator> sep;

		qi::rule<Iterator, FramesAll()>  framesAll;
		qi::rule<Iterator, FrameRange()> frameRange;
		qi::rule<Iterator, FrameIndex()> frameIndex;
		qi::rule<Iterator, FrameSet()>   frameSet;

		qi::rule<Iterator, string()> path;
		qi::rule<Iterator, Script()> script;
		qi::rule<Iterator, float()>  time;

		qi::symbols<char, Resolution> resolution;
		qi::symbols<char, AAMask>     aamask;
	};
}

Script LoadScript(const char * path)
{
	ifstream file(path);
	if (!file)
		throw invalid_argument("File not found.");
	file.unsetf(ios::skipws);

	string text{istream_iterator<char>(file), istream_iterator<char>()};
	string::const_iterator i(text.begin()), end(text.end());

	Script                                script;
	ScriptGrammar<string::const_iterator> grammar;

	bool matches = qi::parse(i, end, grammar, script);
	if (!matches || i != end)
		throw runtime_error("Parsing failed.");

	return script;
}