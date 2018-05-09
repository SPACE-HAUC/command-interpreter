#ifndef _INTERP_H_
#define _INTERP_H_

#include "Optional.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/types/list.hpp"
#include "cereal/types/string.hpp"

#include <functional>
#include <list>
#include <map>
#include <string>
#include <iostream>

class Sexp {
public:
    bool isAtom;
    std::string atom;
    std::list<Sexp> elements;

    template<class Archive>
    void serialize(Archive &archive) {
        archive(isAtom, atom, elements);
    }
};

typedef std::map<std::string,
                 std::function<std::string(std::list<std::string>)>> CommandSet;
typedef std::function<Optional<std::string>(Sexp)> Interpreter;

// Parse the given command string
Optional<Sexp> parse(std::string cmd);

// Interpret the given command Sexp using the given set of commands
Optional<std::string> interp_with(Sexp s, CommandSet commands);

// Make an interpreter with the given set of commands "built-in"
// That is, produce a function that can interpret commands without having to
// provide the command set every time
Interpreter make_interpreter(CommandSet commands);

// Serialize the given command
std::string serialize(Sexp s);

// Deserialize the given command
Sexp deserialize(std::string str);

// Stringify a Sexp
std::ostream& operator<<(std::ostream& os, const Sexp &s);

#endif /* _INTERP_H_ */
