#include <iostream>
#include <stack>
#include <list>
#include <map>
#include <functional>
#include <string>
#include <sstream>

#include "Optional.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/types/list.hpp"
#include "cereal/types/string.hpp"

#include "interp.hpp"

std::ostream& operator<<(std::ostream& os, const Sexp &s) {
    if(s.isAtom) {
        return os << s.atom;
    } else {
        os << "(";
        for(Sexp el : s.elements) {
            os << el << " ";
        }
        return os << ")";
    }
}


// Parse the given string for a substring representing an s-expression,
// if possible.
// The given string *must* begin with '('.
//
// Examples:
// "(blah (blah) baz) shdfhj djf) fjdsj)" -> Just("(blah (blah) baz)")
// "(blah (blah) baz" -> None
// " (blah)" -> None
Optional<std::string> sexp_str(std::string str) {
    if(str.size() < 2 || str[0] != '(') {
	return None<std::string>();
    }

    std::stack<char> parens;
    bool in_str = false;
    char last_char = ' ';
    for (int i = 0; i < str.size(); ++i) {
	char c = str[i];
	switch(c) {
	case '(':
	    if(!in_str) {
		parens.push('(');
	    }
	    break;

	case ')':
	    if(!in_str) {
		parens.pop();
		if(parens.empty()) {
		    return Just(str.substr(0, i + 1));
		}
	    }
	    break;

	case '"':
	    if(last_char != '\\') {
		in_str = !in_str;
	    }
	    break;

	default:
	    break;
	}
	last_char = c;
    }
    return None<std::string>();
}

Optional<Sexp> parse(std::string cmd) {

    if(cmd.size() < 1 || cmd[0] != '(') {
	return None<Sexp>();
    }
    std::list<Sexp> tokens;
    bool in_str = false;
    std::string token = "";
    bool started_cmd = false;
    char last_char = ' ';
    for (int i = 0; i < cmd.size(); ++i) {
	char c = cmd[i];
	switch(c) {
	case '(':
	    if(!started_cmd) {
		started_cmd = true;
	    } else {
		Optional<std::string> sexp_as_str = sexp_str(cmd.substr(i));
		Optional<Sexp> sexp = sexp_as_str.flatMap(parse);
		if(sexp.isEmpty()) {
		    return None<Sexp>();
		} else {
		    tokens.push_back(sexp.get());
		    i += sexp_as_str.get().size();
		}
	    }
	    break;

	case ')':
	    if(started_cmd) {
		// Reached end of cmd
		if(token != "") {
		    Sexp s;
		    s.isAtom = true;
		    s.atom = token;
		    tokens.push_back(s);
		}
		Sexp s;
		s.isAtom = false;
		s.elements = tokens;
		return Just(s);
	    }
	    break;

	case '"':
	    if(last_char != '\\') {
		Sexp s;
		s.isAtom = true;
		s.atom = token;
		if(in_str) {
		    in_str = false;
		    tokens.push_back(s);
		    token = "";
		} else {
		    if(token != "") {
			// need to finish prev token
			tokens.push_back(s);
			token = "";
		    }
		    in_str = true;
		}
	    }
	    break;

	case ' ':
	    if(!in_str) {
		if(token != "") {
		    Sexp s;
		    s.isAtom = true;
		    s.atom = token;
		    tokens.push_back(s);
		    token = "";
		}
	    } else {
		token += c;
	    }
	    break;

	case '\\':
	    break;

	default:
	    token += c;
	}
	last_char = c;
    }
    // End of string with no closing paren
    return None<Sexp>();
}

Optional<std::string> interp_with(Sexp s, CommandSet commands) {
    if(s.isAtom) {
	return Just(s.atom);
    } else {
	std::list<Sexp> elements = s.elements;
	std::list<std::string> element_strs;
	for(Sexp s : elements) {
	    Optional<std::string> element = interp_with(s, commands);
	    if(element.isEmpty()) {
		std::cout << "Error: element fails interp: "
			  << s << std::endl;
		return None<std::string>();
	    }
	    element_strs.push_back(element.get());
	}
	std::string command = element_strs.front();
	element_strs.pop_front();
	try {
	    return Just(commands[command](element_strs));
	} catch(const std::invalid_argument &e) {
	    return Just("Error: invalid argument: " + std::string(e.what()));
	} catch(const std::bad_function_call &e) {
	    return Just("Error: Command '" + command + "' undefined.");
	}
    }
}

std::function<Optional<std::string>(Sexp)>
make_interpreter(CommandSet commands) {
    return [commands](Sexp s) {
	return interp_with(s, commands);
    };
}

std::string serialize(Sexp s) {
    std::stringstream ss;

    {
	cereal::BinaryOutputArchive oarchive(ss); // Create an output archive

	oarchive(s); // Write the data to the archive
    } // archive goes out of scope, ensuring all contents are flushed

    return ss.str();
}

Sexp deserialize(std::string str) {
    std::stringstream ss(str);
    Sexp sexp;
    {
	cereal::BinaryInputArchive iarchive(ss);
	iarchive(sexp);
    }
    return sexp;
}
