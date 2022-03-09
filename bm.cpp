#include <array>
#include <cstddef>
#include <cstdint>
#include <fmt/format.h>
#include <stdexcept>

using word = std::int64_t;

enum struct inst_type {
    push,
    plus,
};

struct inst {
    inst_type type;
    word value;

    static constexpr inst push(word x) {
        return inst{
            .type = inst_type::push,
            .value = x,
        };
    }

    static constexpr inst plus() {
        return inst{.type = inst_type::plus};
    }
};

struct stack_overflow_error : std::runtime_error {
    explicit stack_overflow_error(std::size_t capacity)
        : std::runtime_error(fmt::format("stack exceeded capacity ({})", capacity)) {
    }
};

struct stack_underflow_error : std::runtime_error {
    stack_underflow_error() : std::runtime_error("stack hit bottom") {
    }
};

struct bm {
    static constexpr std::size_t stack_capacity = 1024;
    std::array<word, stack_capacity> stack;
    std::size_t stack_size = 0;

    bm() = default;

    void execute_inst(inst i) {
        switch (i.type) {
            case inst_type::push:
                if (stack_size == stack_capacity) {
                    throw stack_overflow_error{stack_capacity};
                }
                stack[stack_size] = i.value;
                stack_size += 1;
                break;
            case inst_type::plus:
                if (stack_size < 2) {
                    throw stack_underflow_error{};
                }
                stack[stack_size - 2] += stack[stack_size - 1];
                stack_size -= 1;
                break;
        }
    }

    void dump() const {
        fmt::print("Stack:\n");
        for (std::size_t i = 0; i < stack_size; i += 1) {
            fmt::print("\t{}\n", stack[i]);
        }
    }
};

constexpr std::array program = {
    inst::push(69),
    inst::push(420),
    inst::plus(),
};

int main() {
    bm machine;
    for (const inst& i : program) {
        machine.execute_inst(i);
        machine.dump();
    }
}
