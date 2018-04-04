#include <iostream>
#include <stack>
#include <list>
#include <map>
#include <functional>
#include <string>
#include <cassert>

#include "../cdh_main/include/Optional.hpp"

struct Sexp {
    bool isAtom;
    std::string atom;
    std::list<Sexp> elements;
};

std::map<std::string,
	 std::function<std::string(std::list<std::string>)>> commands;

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
		Optional<Sexp> sexp = sexp_as_str.map(parse);
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

Optional<std::string> interp(Sexp s) {
    if(s.isAtom) {
	return Just(s.atom);
    } else {
	std::list<Sexp> elements = s.elements;
	std::list<std::string> element_strs;
	for(Sexp s : elements) {
	    Optional<std::string> element = interp(s);
	    if(element.isEmpty()) {
		std::cout << "Err: element fails interp" << std::endl;
		return None<std::string>();
	    }
	    element_strs.push_back(element.get());
	}
	std::string command = element_strs.front();
	element_strs.pop_front();
	return Just(commands[command](element_strs));
    }
}

std::string add(std::list<std::string> nums) {
    int sum = 0;
    for(std::string num: nums) {
	sum += std::stoi(num);
    }
    return std::to_string(sum);
}

std::string concat(std::list<std::string> strs) {
    std::string res = "";
    for(std::string s: strs) {
	res += s;
    }
    return res;
}

void test() {
    std::cout << "Sexp string parsing:" << std::endl;
    std::cout << sexp_str("(blah (blah) baz) djjgf fg) f") << std::endl;
    std::cout << sexp_str("blah (blah) baz) djjgf fg) f") << std::endl;
    std::cout << sexp_str("(blah (blah) baz)") << std::endl;
    std::cout << sexp_str("(blah ((blah) baz))") << std::endl;
    std::cout << sexp_str("(blah (\"(blah) bro\" baz))") << std::endl;
    std::cout << sexp_str("(blah (\"(blah) bro))))\" baz))") << std::endl;
    std::cout << sexp_str("(blah (\"(blah) \\\"bro)))\\\")\" baz))") << std::endl;
    std::cout << sexp_str("(hi (joe schmoe) schmoe)") << std::endl;

    std::cout << "Real parsing:" << std::endl;
    std::cout << parse("(hi joe schmoe)") << std::endl;
    Optional<Sexp> os = parse("(hi (joe schmoe) schmoe)");
    assert(!os.isEmpty());
    Sexp s = os.get();
    assert(!s.isAtom);
    assert(s.elements.size() == 3);
    assert(s.elements.front().isAtom);
    assert(s.elements.back().isAtom);
    s.elements.pop_front();
    assert(!s.elements.front().isAtom);
    assert(s.elements.front().elements.size() == 2);

    std::cout << "Interp:" << std::endl;
    commands["add"] = add;
    commands["concat"] = concat;
    Optional<std::string> ores = parse("(add 1 2)").map(interp);
    assert(!ores.isEmpty());
    assert(ores.get() == "3");
    ores = parse("(add 1 2 3 4)").map(interp);
    assert(!ores.isEmpty());
    assert(ores.get() == "10");
    ores = parse("(add 1 2 3 4 -20)").map(interp);
    assert(!ores.isEmpty());
    assert(ores.get() == "-10");
    ores = parse("(concat this -- a test)").map(interp);
    assert(!ores.isEmpty());
    assert(ores.get() == "this--atest");
    ores = parse("(concat \"this\" \"test\")").map(interp);
    assert(!ores.isEmpty());
    assert(ores.get() == "thistest");
    ores = parse("(concat \"this   is \" \"test\")").map(interp);
    assert(!ores.isEmpty());
    assert(ores.get() == "this   is test");
}

int main(int argc, char *argv[]) {
    test();
    return 0;
}
