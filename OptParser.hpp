/*
 * OptParser.hpp, part of OptParser
 *
 * Copyright (C) 2022 Antonin Portelli
 *
 * OptParser is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OptParser is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OptParser.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OptParser_hpp_
#define OptParser_hpp_

#ifndef OPT_PARSER_NS
#define OPT_PARSER_NS optp
#endif

#include <iomanip>
#include <iostream>
#include <queue>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace OPT_PARSER_NS
{
// String utilities ////////////////////////////////////////////////////////////
template <typename T>
inline T strTo(const std::string &str)
{
  T buf;
  std::istringstream stream(str);

  stream >> buf;

  return buf;
}

// optimized specializations
template <>
inline float strTo<float>(const std::string &str)
{
  return strtof(str.c_str(), (char **)NULL);
}
template <>
inline double strTo<double>(const std::string &str)
{
  return strtod(str.c_str(), (char **)NULL);
}
template <>
inline int strTo<int>(const std::string &str)
{
  return (int)(strtol(str.c_str(), (char **)NULL, 10));
}
template <>
inline long strTo<long>(const std::string &str)
{
  return strtol(str.c_str(), (char **)NULL, 10);
}
template <>
inline std::string strTo<std::string>(const std::string &str)
{
  return str;
}

template <typename T>
inline std::string strFrom(const T x)
{
  std::ostringstream stream;

  stream << x;

  return stream.str();
}

/******************************************************************************
 *                             main class                                     *
 ******************************************************************************/
class OptParser
{
public:
  enum class OptType
  {
    value,
    trigger
  };

private:
  struct OptPar
  {
    std::string shortName, longName, defaultVal, helpMessage;
    OptType type;
    bool optional;
  };
  struct OptRes
  {
    std::string value;
    bool present;
  };

public:
  // constructor
  OptParser(void) = default;
  // destructor
  virtual ~OptParser(void) = default;
  // access
  void addOption(const std::string shortName, const std::string longName, const OptType type,
                 const bool optional = false, const std::string helpMessage = "",
                 const std::string defaultVal = "");
  bool gotOption(const std::string name) const;
  template <typename T = std::string>
  T optionValue(const std::string name) const;
  const std::vector<std::string> &getArgs(void) const;
  // parse
  bool parse(const int argc, const char *argv[]);
  // print option list
  friend std::ostream &operator<<(std::ostream &out, const OptParser &parser);

private:
  // find option index
  int optIndex(const std::string name) const;
  // option name for messages
  static std::string optName(const OptPar &opt);

private:
  std::vector<OptPar> opt_;
  std::vector<OptRes> result_;
  std::vector<std::string> arg_;
  static const std::regex optRegex_;
};

/******************************************************************************
 *                         OptParser implementation                           *
 ******************************************************************************/
// regular expression //////////////////////////////////////////////////////////
constexpr char optRegex[] = "(-([a-zA-Z])(.+)?)|(--([a-zA-Z_-]+)=?(.+)?)";

const std::regex OptParser::optRegex_(optRegex);

// access //////////////////////////////////////////////////////////////////////
void OptParser::addOption(const std::string shortName, const std::string longName,
                          const OptType type, const bool optional, const std::string helpMessage,
                          const std::string defaultVal)
{
  OptPar par;

  par.shortName = shortName;
  par.longName = longName;
  par.defaultVal = defaultVal;
  par.helpMessage = helpMessage;
  par.type = type;
  par.optional = optional;
  auto it = std::find_if(opt_.begin(), opt_.end(),
                         [&par](const OptPar &p)
                         {
                           bool match = false;

                           match |= (par.shortName == p.shortName) and !par.shortName.empty();
                           match |= (par.longName == p.longName) and !par.longName.empty();

                           return match;
                         });
  if (it != opt_.end())
  {
    std::string opt;

    if (!it->shortName.empty())
    {
      opt += "-" + it->shortName;
    }
    if (!opt.empty())
    {
      opt += "/";
    }
    if (!it->longName.empty())
    {
      opt += "--" + it->longName;
    }
    throw(std::logic_error("duplicate option " + opt));
  }
  opt_.push_back(par);
}

bool OptParser::gotOption(const std::string name) const
{
  int i = optIndex(name);

  if (result_.size() != opt_.size())
  {
    throw(std::runtime_error("options not parsed"));
  }
  if (i >= 0)
  {
    return result_[i].present;
  }
  else
  {
    throw(std::out_of_range("no option with name '" + name + "'"));
  }
}

template <typename T>
T OptParser::optionValue(const std::string name) const
{
  int i = optIndex(name);

  if (result_.size() != opt_.size())
  {
    throw(std::runtime_error("options not parsed"));
  }
  if (i >= 0)
  {
    return strTo<T>(result_[i].value);
  }
  else
  {
    throw(std::out_of_range("no option with name '" + name + "'"));
  }
}

const std::vector<std::string> &OptParser::getArgs(void) const { return arg_; }

// parse ///////////////////////////////////////////////////////////////////////
bool OptParser::parse(const int argc, const char *argv[])
{
  std::smatch sm;
  std::queue<std::string> arg;
  int expectVal = -1;
  bool isCorrect = true;

  for (int i = 1; i < argc; ++i)
  {
    arg.push(argv[i]);
  }
  result_.clear();
  result_.resize(opt_.size());
  arg_.clear();
  for (unsigned int i = 0; i < opt_.size(); ++i)
  {
    result_[i].value = opt_[i].defaultVal;
  }
  while (!arg.empty())
  {
    // option
    if (regex_match(arg.front(), sm, optRegex_))
    {
      // should it be a value?
      if (expectVal >= 0)
      {
        std::cerr << "warning: expected value for option ";
        std::cerr << optName(opt_[expectVal]);
        std::cerr << ", got option '" << arg.front() << "' instead" << std::endl;
        expectVal = -1;
        isCorrect = false;
      }
      // short option
      if (sm[1].matched)
      {
        std::string optName = sm[2].str();

        // find option
        auto it = find_if(opt_.begin(), opt_.end(),
                          [&optName](const OptPar &p) { return (p.shortName == optName); });

        // parse if found
        if (it != opt_.end())
        {
          unsigned int i = it - opt_.begin();

          result_[i].present = true;
          if (opt_[i].type == OptType::value)
          {
            if (sm[3].matched)
            {
              result_[i].value = sm[3].str();
            }
            else
            {
              expectVal = i;
            }
          }
        }
        // warning if not found
        else
        {
          std::cerr << "warning: unknown option '" << arg.front() << "'";
          std::cerr << std::endl;
        }
      }
      // long option
      else if (sm[4].matched)
      {
        std::string optName = sm[5].str();

        // find option
        auto it = find_if(opt_.begin(), opt_.end(),
                          [&optName](const OptPar &p) { return (p.longName == optName); });

        // parse if found
        if (it != opt_.end())
        {
          unsigned int i = it - opt_.begin();

          result_[i].present = true;
          if (opt_[i].type == OptType::value)
          {
            if (sm[6].matched)
            {
              result_[i].value = sm[6].str();
            }
            else
            {
              expectVal = i;
            }
          }
        }
        // warning if not found
        else
        {
          std::cerr << "warning: unknown option '" << arg.front() << "'";
          std::cerr << std::endl;
        }
      }
    }
    else if (expectVal >= 0)
    {
      result_[expectVal].value = arg.front();
      expectVal = -1;
    }
    else
    {
      arg_.push_back(arg.front());
    }
    arg.pop();
  }
  if (expectVal >= 0)
  {
    std::cerr << "warning: expected value for option ";
    std::cerr << optName(opt_[expectVal]) << std::endl;
    expectVal = -1;
    isCorrect = false;
  }
  for (unsigned int i = 0; i < opt_.size(); ++i)
  {
    if (!opt_[i].optional and !result_[i].present)
    {
      std::cerr << "warning: mandatory option " << optName(opt_[i]);
      std::cerr << " is missing" << std::endl;
      isCorrect = false;
    }
  }

  return isCorrect;
}

// find option index ///////////////////////////////////////////////////////////
int OptParser::optIndex(const std::string name) const
{
  auto it =
      find_if(opt_.begin(), opt_.end(),
              [&name](const OptPar &p) { return (p.shortName == name) or (p.longName == name); });

  if (it != opt_.end())
  {
    return static_cast<int>(it - opt_.begin());
  }
  else
  {
    return -1;
  }
}

// option name for messages ////////////////////////////////////////////////////
std::string OptParser::optName(const OptPar &opt)
{
  std::string res = "";

  if (!opt.shortName.empty())
  {
    res += "-" + opt.shortName;
    if (!opt.longName.empty())
    {
      res += "/";
    }
  }
  if (!opt.longName.empty())
  {
    res += "--" + opt.longName;
    if (opt.type == OptParser::OptType::value)
    {
      res += "=";
    }
  }

  return res;
}

// print option list ///////////////////////////////////////////////////////////
std::ostream &operator<<(std::ostream &out, const OPT_PARSER_NS::OptParser &parser);

std::ostream &operator<<(std::ostream &out, const OPT_PARSER_NS::OptParser &parser)
{
  for (auto &o : parser.opt_)
  {
    out << std::setw(20) << OPT_PARSER_NS::OptParser::optName(o);
    out << ": " << o.helpMessage;
    if (!o.defaultVal.empty())
    {
      out << " (default: " << o.defaultVal << ")";
    }
    out << std::endl;
  }

  return out;
}

} // namespace OPT_PARSER_NS

#endif // OptParser_hpp_
