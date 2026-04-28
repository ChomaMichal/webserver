#include "utils.hpp"
void getline_stripspace(std::ifstream &infile, std::string &line) {
  getline(infile, line);
  line.erase(0, line.find_first_not_of(" \t"));
  line.erase(line.find_last_not_of(" \t") + 1);
}
void getline_stripspace(std::ifstream &infile, std::string &line,
                        std::string delim) {
  line.clear();
  char c;
  bool to_trim = true;
  while (infile.get(c)) {
    if (delim.find(c) != std::string::npos && to_trim == false)
      break;
    if (c != ' ' && c != '\t')
      to_trim = false;
    line += c;
  }
  line.erase(0, line.find_first_not_of(" \t"));
  line.erase(line.find_last_not_of(" \t") + 1);
}
void get_redirection_pair(std::string line, std::pair<int, std::string> pair) {
  size_t space = line.find_first_of(' ');
  if (space == std::string::npos) {
    char *endptr;
    pair.first = std::strtol(line.c_str(), &endptr, 10);
    if (*endptr != '\0')
      throw("Invalid Config File: Invalid Redirection");
  } else {
    std::string sredir = line.substr(0, space);
    char *endptr;
    pair.first = std::strtol(line.c_str(), &endptr, 10);
    if (*endptr != '\0')
      throw("Invalid Config File: Invalid Redirection");
    pair.second = line.substr(space + 1, std::string::npos);
  }
}
