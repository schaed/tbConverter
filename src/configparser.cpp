#include "configparser.h"

#include <cassert>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include <stdint.h>

using std::string;
using std::cout;
using std::endl;

bool ConfigParser::checkRange(size_t start, size_t end)
{
  if (start == string::npos || end == string::npos || start > end)
    return false;
  else
    return true;
}

int ConfigParser::readNextLine(std::ifstream& intput)
{
  // Search for a non commented line
  while (intput.good())
  {
    string buffer;
    getline(intput, buffer);

    // Find the end of the line (eol or start of a comment)
    size_t end = buffer.find_first_of('#');
    if (end == string::npos) end = buffer.length();

    // If the first non-blank character
    size_t first = buffer.find_first_not_of(" \t\r\n");
    if (first == string::npos) first = buffer.length();

    // If the first valid character is the end, this line is empty
    if (first >= end) continue;

    // The line in buffer is good, make a stream from it
    _lineBuffer = buffer.substr(first, end - first);

    return 0;
  }

  return -1;
}

int ConfigParser::parseForHeader()
{
  size_t start = _lineBuffer.find_first_of('[');
  size_t end   = _lineBuffer.find_first_of(']');

  if (!checkRange(start, end)) return -1;

  _currentHeader = _lineBuffer.substr(start + 1, end - start - 1);

  return 0;
}

int ConfigParser::parseForKey()
{
  size_t separator = _lineBuffer.find_first_of(':');

  if (separator == string::npos) return -1;

  string keyString = _lineBuffer.substr(0, separator);

  size_t keyStart = keyString.find_first_not_of(" \t\r\n");
  size_t keyEnd   = keyString.find_last_not_of(" \t\r\n");

  if (!checkRange(keyStart, keyEnd))
    _currentKey = "";
  else
    _currentKey   = keyString.substr(keyStart, keyEnd + 1 - keyStart);

  string valueString = _lineBuffer.substr(separator + 1);

  size_t valueStart = valueString.find_first_not_of(" \t\r\n");
  size_t valueEnd   = valueString.find_last_not_of(" \t\r\n");

  if (!checkRange(valueStart, valueEnd))
    _currentValue = "";
  else
    _currentValue = valueString.substr(valueStart, valueEnd - valueStart + 1);

  return 0;
}

int ConfigParser::parseForLink()
{
  if (parseForKey()) return -1;
  if (_currentKey.compare("LINK")) return -1;
  return 0;
}

void ConfigParser::parseContents(std::ifstream& input)
{
  while (!readNextLine(input))
  {
    if (!parseForHeader())
    {
      Row row;
      row.header = _currentHeader;
      row.key = "";
      row.value = "";
      row.isHeader = true;
      _parsedContents.push_back(row);
      _numRows++;
    }
    // Look for a link to another configuraiton file
    else if (!parseForLink())
    {
      std::ifstream linked;
      linked.open(_currentValue.c_str());
      if (!linked.is_open())
        throw "ConfigParser: unable to open linked configuration";
      parseContents(linked);
    }
    else if (!parseForKey())
    {
      Row row;
      row.header = _currentHeader;
      row.key    = _currentKey;
      row.value  = _currentValue;
      row.isHeader = false;
      _parsedContents.push_back(row);
      _numRows++;
    }
  }
}



unsigned int ConfigParser::getNumRows() const { return _numRows; }

const ConfigParser::Row* ConfigParser::getRow(unsigned int n) const
{
  assert(n < _numRows &&
         "ConfigParser: row index outside range");
  return &(_parsedContents.at(n));
}

std::vector<ConfigParser::Row> ConfigParser::getParsedConents() const
{
  return _parsedContents;
}

const char* ConfigParser::getFilePath() const { return _filePath; }

double ConfigParser::valueToNumerical(const string& value)
{
  std::stringstream ss;
  ss.str(value);
  double num = 0;
  ss >> num;
  return num;
}

bool ConfigParser::valueToLogical(const string& value)
{
  if (!value.compare("true") || !value.compare("on") || !value.compare("yes"))
    return true;
  else
    return false;
}

void ConfigParser::valueToVec(const string& value, std::vector<double>& vec)
{
  std::stringstream stream;
  stream.str(value);
  string buffer;
  while (stream.good())
  {
    getline(stream, buffer, ',');
    std::stringstream ss;
    ss.str(buffer);
    double num = 0;
    ss >> num;
    vec.push_back(num);
  }
}

ConfigParser::ConfigParser(const char* filePath) :
  _filePath(filePath), _numRows(0)
{
  //std::cout << "File: " << filePath << std::endl;
  _inputFile.open(_filePath);

  if (!_inputFile.is_open()) throw "ConfigParser: input file failed to open";

  _currentHeader = "";
  _currentValue  = "";
  _currentKey    = "";
  _lineBuffer    = "";

  parseContents(_inputFile);

  _inputFile.close();

}


//PSA specific-------------------------------------------

void ConfigParser::print()
{
  if (_numRows < 1)
    cout<<"configParser empty"<<endl;
  else
    for (unsigned int i=0; i<_numRows; i++)
      cout<<i
          <<" "
          <<_parsedContents.at(i).header
          <<" "
          <<_parsedContents.at(i).key
          <<" "
          <<_parsedContents.at(i).value
          <<" "
          <<endl;
}

//--------------------------------------------------------


// Get any numerical value from parsed parameters
int32_t ConfigParser::getParVal (std::string header1,
                                std::string key1)
{
    for (unsigned int i=0; i<_numRows; i++)
    {
        if (!_parsedContents.at(i).isHeader)
            if (!_parsedContents.at(i).header.compare( header1.c_str() ) )
                if (!_parsedContents.at(i).key.compare( key1.c_str() ) )
                    return (int32_t)valueToNumerical(_parsedContents.at(i).value);
    }
    return 0;
}

// Get any numerical value from parsed parameters
float ConfigParser::getParValFloat (std::string header1,
                                 std::string key1)
{
  for (unsigned int i=0; i<_numRows; i++)
  {
    if (!_parsedContents.at(i).isHeader)
      if (!_parsedContents.at(i).header.compare( header1.c_str() ) )
        if (!_parsedContents.at(i).key.compare( key1.c_str() ) )
          return (float)valueToNumerical(_parsedContents.at(i).value);
  }
  return 0;
}

//get float value from parsed parameters
float ConfigParser::getParFlo (std::string header1,
                                std::string key1)
{
    for (unsigned int i=0; i<_numRows; i++)
    {
        if (!_parsedContents.at(i).isHeader)
            if (!_parsedContents.at(i).header.compare( header1.c_str() ) )
                if (!_parsedContents.at(i).key.compare( key1.c_str() ) )
                    return (float)valueToNumerical(_parsedContents.at(i).value);
    }
    return 0;
}


std::string ConfigParser::getParStr (std::string header1,
                                       std::string key1)
{
    for (unsigned int i=0; i<_numRows; i++)
    {
        if (!_parsedContents.at(i).isHeader)
            if (!_parsedContents.at(i).header.compare( header1.c_str() ) )
                if (!_parsedContents.at(i).key.compare( key1.c_str() ) )
                    return _parsedContents.at(i).value;
    }
    return "";
}






//--------obsolete------------------------------------------
int32_t ConfigParser::getInputParam (std::string name)
{
  for (unsigned int i=0; i<_numRows; i++)
  {
    if (!_parsedContents.at(i).isHeader)
      if (!_parsedContents.at(i).header.compare("Pulse parameters") )
        if (!_parsedContents.at(i).key.compare(name) )
          return (int32_t)valueToNumerical(_parsedContents.at(i).value);
  }

  throw " ConfigParser::getPulseListFileName : NO PUlSE LIST FILE IN CFG FILE!";
  return 0;
}

std::string ConfigParser::getPulseListFileName()
{
  for (unsigned int i=0; i<_numRows; i++)
  {
    if (!_parsedContents.at(i).isHeader)
      if (!_parsedContents.at(i).header.compare("Batch pulses") )
        if (!_parsedContents.at(i).key.compare("filename") )
          return _parsedContents.at(i).value;
  }

  throw " ConfigParser::getPulseListFileName : NO PUlSE LIST FILE IN CFG FILE!";
  return "";
}
