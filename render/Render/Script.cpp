#include "Script.h"

#include "Color.h"

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
#include <sstream>

namespace qi = boost::spirit::qi;

using std::string;

BOOST_FUSION_ADAPT_STRUCT
	(Script,
	(string,           meshPath)
	(string,           outputPath)
	(string,           model)
	(Resolution,       res)
	(AAMask,           aamask)
	(float,            fps)
	(float,            duration)
	(FrameSet,         frames)
	(bool,             printFrameInfo)
	(Script::LabColor, background)
	(float,            noise)
	)

namespace
{
	struct LazyRgbToLab
	{
		using result_type = Script::LabColor;

		template<typename T>
		Script::LabColor operator()(T x, T y, T z) const
		{
			return RgbToLab(Eigen::Vector3f(x, y, z));
		}
	};

	template<typename Iterator, typename Skipper>
	struct ScriptGrammar : qi::grammar<Iterator, Script(), Skipper>
	{
		ScriptGrammar() : ScriptGrammar::base_type(script)
		{
			using namespace qi;
			using boost::phoenix::construct;
			using boost::phoenix::function;

			resolution.add
				("2160p", res2160p)
				("1080p", res1080p)
				("720p",  res720p)
				("576p",  res576p)
				("360p",  res360p);

			aamask.add
				("1x", aa1x)
				("4x", aa4x)
				("8x", aa8x);

			modelType.add
				("fgt",   ModelType::Fgt)
				("voxel", ModelType::Voxel);

			framesAll   = lit("all") [_val = FramesAll()];
			frameRange %= uint_ >> '-' >> uint_;
			frameIndex %= uint_;
			frameSet   %= framesAll | frameRange | frameIndex;

			labColor
				= (lit("lab") > '(' > float_ > ',' > float_ > ',' > float_ > ')')
				[_val = construct<Script::LabColor>(_1, _2, _3)];

			rgbColor
				= (lit("rgb") > '(' > float_ > ',' > float_ > ',' > float_ > ')')
				[_val = function<LazyRgbToLab>()(_1, _2, _3)];

			color %= labColor | rgbColor;

			path %= '"' > +(char_ - '"') > '"';

			constexpr char sep = ':';

			script
				%= (lit("mesh-path")        > sep > path       > eol)
				^  (lit("output-path")      > sep > path       > eol)
				^  (lit("model")            > sep > path       > eol)
				^  (lit("resolution")       > sep > resolution > eol)
				^  (lit("antialiasing")     > sep > aamask     > eol)
				^  (lit("fps")              > sep > float_     > eol)
				^  (lit("duration")         > sep > time       > eol)
				^  (lit("frames")           > sep > frameSet   > eol)
				^  (lit("print-frame-info") > sep > bool_      > eol)
				^  (lit("background")       > sep > color      > eol)
				^  (lit("noise")            > sep > float_     > eol)
				;

			time
				= (float_ > "s")                  [_val = _1]
				| (float_ > "min" > float_ > "s") [_val = _1 * 60.0f + _2]
				| (float_ > "min")                [_val = _1 * 60.0f];

		}

		qi::symbols<char, Resolution> resolution;
		qi::symbols<char, AAMask>     aamask;
		qi::symbols<char, ModelType>  modelType;

		qi::rule<Iterator, FramesAll(),  Skipper> framesAll;
		qi::rule<Iterator, FrameRange(), Skipper> frameRange;
		qi::rule<Iterator, FrameIndex(), Skipper> frameIndex;
		qi::rule<Iterator, FrameSet(),   Skipper> frameSet;


		qi::rule<Iterator, Script::LabColor, Skipper> labColor;
		qi::rule<Iterator, Script::LabColor, Skipper> rgbColor;
		qi::rule<Iterator, Script::LabColor, Skipper> color;

		qi::rule<Iterator, string()>               path;
		qi::rule<Iterator, Script(), Skipper>      script;
		qi::rule<Iterator, float(), Skipper>       time;
	};


	template<typename BeginIter, typename EndIter>
	std::tuple<int, int> GetPosition(BeginIter begin, EndIter end)
	{
		int r = 0, c = 0;
		for (BeginIter i = begin; i != end; ++i)
		{
			if (*i == '\n')
			{
				++r;
				c = 0;
			}
			else
				++c;
		}
		return std::make_tuple(r, c);
	}
}

std::string ToString(ModelType modelType)
{
	switch (modelType)
	{
	case ModelType::Fgt:   return "FGT";
	case ModelType::Voxel: return "Voxel";
	}
	return "";
}

Script LoadScript(const char * path)
{
	using FileIterator   = std::istream_iterator<char>;
	using StringIterator = string::const_iterator;
	using Grammar        = ScriptGrammar<StringIterator, qi::blank_type>;

	std::ifstream file(path);
	if (!file)
		throw std::invalid_argument("Script file could not be opened.");
	file.unsetf(std::ios::skipws);

	string text{FileIterator(file), FileIterator()};
	StringIterator       i   (text.begin());
	const StringIterator end (text.end());

	Script script;

	bool isMatch = false;
	try
	{
		isMatch = qi::phrase_parse(i, end, Grammar(), qi::blank, script);
	}
	catch (const qi::expectation_failure<StringIterator> & e)
	{
		i = e.first;
	}
	if (!isMatch || i != end)
	{
		const std::tuple<int, int> pos = GetPosition(text.begin(), i);
		const int row = std::get<0>(pos) + 1;
		const int col = std::get<1>(pos) + 1;

		std::ostringstream msg;
		msg << "Parsing failed at row " << row << " col " << col << ".";
		throw std::runtime_error(msg.str());
	}

	return script;
}