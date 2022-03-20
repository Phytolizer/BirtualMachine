#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fmt/format.h>
#include <magic_enum.hpp>
#include <stdexcept>

using word = std::int64_t;

enum struct inst_type {
    push,
    plus,
    minus,
    mult,
    div,
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

    static constexpr inst minus() {
        return inst{.type = inst_type::minus};
    }

    static constexpr inst mult() {
        return inst{.type = inst_type::mult};
    }

    static constexpr inst div() {
        return inst{.type = inst_type::div};
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

struct illegal_inst_error : std::runtime_error {
    explicit illegal_inst_error(int type)
        : std::runtime_error(fmt::format("illegal instruction {:#x}", type)) {
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
            case inst_type::minus:
                if (stack_size < 2) {
                    throw stack_underflow_error{};
                }
                stack[stack_size - 2] -= stack[stack_size - 1];
                stack_size -= 1;
                break;
            case inst_type::mult:
                if (stack_size < 2) {
                    throw stack_underflow_error{};
                }
                stack[stack_size - 2] *= stack[stack_size - 1];
                stack_size -= 1;
                break;
            case inst_type::div:
                if (stack_size < 2) {
                    throw stack_underflow_error{};
                }
                stack[stack_size - 2] /= stack[stack_size - 1];
                stack_size -= 1;
                break;
            default:
                throw illegal_inst_error{static_cast<int>(i.type)};
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
    inst::push(69), inst::push(420), inst::plus(), inst::push(42), inst::minus(),
};

int main() {
    bm machine;
    for (const inst& i : program) {
        fmt::print("{}\n", magic_enum::enum_name(i.type));
        try {
            machine.execute_inst(i);
        } catch (const std::runtime_error& e) {
            fmt::print(stderr, "ERROR: {}", e.what());
            return -1;
        }
        machine.dump();
    }
}
