#ifndef _INTERP_H_
#define _INTERP_H_

#include "Optional.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/types/list.hpp"
#include "cereal/types/string.hpp"

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

Optional<Sexp> parse(std::string cmd);
Optional<std::string> interp(Sexp s);
std::string serialize(Sexp s);
Sexp deserialize(std::string str);
std::ostream& operator<<(std::ostream& os, const Sexp &s);

#endif /* _INTERP_H_ */
