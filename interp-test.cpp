#include "interp.hpp"

#include <cassert>

// Example command
std::string add(std::list<std::string> nums) {
    int sum = 0;
    for(std::string num: nums) {
        sum += std::stoi(num);
    }
    return std::to_string(sum);
}

// Example command
std::string concat(std::list<std::string> strs) {
    std::string res = "";
    for(std::string s: strs) {
        res += s;
    }
    return res;
}

// Example command
std::string exit_repl(std::list<std::string> strs) {
    exit(0);
}

// Example repl implementation
void repl() {
    CommandSet commands;
    commands["add"] = add;
    commands["concat"] = concat;
    commands["exit"] = exit_repl;
    auto interp = make_interpreter(commands);
    std::string cmd;
    std::cout << "interp > ";
    while(getline(std::cin, cmd)) {
        std::cout << parse(cmd).flatMap(interp).getDefault("Invalid command.")
                  << std::endl;
        std::cout << "interp > ";
    }
}

// Unit tests
void test() {
    // Sexp parsing
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

    // Interp
    CommandSet commands;
    commands["add"] = add;
    commands["concat"] = concat;
    auto interp = make_interpreter(commands);

    Optional<std::string> ores = parse("(add 1 2)").flatMap(interp);
    assert(!ores.isEmpty());
    assert(ores.get() == "3");
    ores = parse("(add 1 2 3 4)").flatMap(interp);
    assert(!ores.isEmpty());
    assert(ores.get() == "10");
    ores = parse("(add 1 2 3 4 -20)").flatMap(interp);
    assert(!ores.isEmpty());
    assert(ores.get() == "-10");
    ores = parse("(concat this -- a test)").flatMap(interp);
    assert(!ores.isEmpty());
    assert(ores.get() == "this--atest");
    ores = parse("(concat \"this\" \"test\")").flatMap(interp);
    assert(!ores.isEmpty());
    assert(ores.get() == "thistest");
    ores = parse("(concat \"this   is \" \"test\")").flatMap(interp);
    assert(!ores.isEmpty());
    assert(ores.get() == "this   is test");
}

int main(int argc, char *argv[]) {
    std::cout << "Running tests..." << std::endl;
    test();
    std::cout << "Done." << std::endl;

    std::cout << "Serialization demo:" << std::endl;
    auto serialized = parse("(hi joe schmoe)").map(serialize);
    if (serialized.isEmpty()) {
        std::cout << "Failed." << std::endl;
    } else {
        auto res = serialized.get();
        std::cout << "Serialized: " << res << std::endl;
        Sexp s = deserialize(res);
        std::cout << "Deserialized: " << s << std::endl;
    }
    std::cout << "Done.\n\n" << std::endl;

    repl();
    return 0;
}
