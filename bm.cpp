#include "fixed_stack.hpp"

#include <cstdint>
#include <fmt/core.h>
#include <magic_enum.hpp>
#include <stack>

using word = std::int64_t;

enum class trap {
    ok,
    stack_overflow,
    stack_underflow,
    illegal_inst,
    illegal_jump,
    div_by_zero,
};

class inst {
  public:
    enum class type {
        push,
        plus,
        minus,
        mult,
        div,
        jmp,
        halt,
    };

  private:
    type m_ty;
    word m_operand;
    constexpr inst(type ty, word operand) : m_ty(ty), m_operand(operand) {}
    constexpr inst(type ty) : m_ty(ty), m_operand(0) {}

  public:
    static constexpr inst push(word operand) {
        return {type::push, operand};
    }
    static constexpr inst plus() {
        return {type::plus};
    }
    static constexpr inst minus() {
        return {type::minus};
    }
    static constexpr inst mult() {
        return {type::mult};
    }
    static constexpr inst div() {
        return {type::div};
    }
    static constexpr inst jmp(word operand) {
        return {type::jmp, operand};
    }
    static constexpr inst halt() {
        return {type::halt};
    }

    type ty() const {
        return m_ty;
    }
    word operand() const {
        return m_operand;
    }
};

class machine {
    static constexpr size_t stack_capacity = 1024;
    phy::fixed_stack<word, stack_capacity> m_stack;
    word m_ip = 0;
    bool m_halt = false;

  public:
    machine() = default;

    trap execute(const inst& i) {
        switch (i.ty()) {
            case inst::type::push:
                if (m_stack.full()) {
                    return trap::stack_overflow;
                }
                m_stack.push(i.operand());
                m_ip++;
                break;
            case inst::type::plus: {
                if (m_stack.size() < 2) {
                    return trap::stack_underflow;
                }
                auto a = std::move(m_stack.top());
                m_stack.pop();
                m_stack.top() += a;
                m_ip++;
            } break;
            case inst::type::minus: {
                if (m_stack.size() < 2) {
                    return trap::stack_underflow;
                }
                auto a = std::move(m_stack.top());
                m_stack.pop();
                m_stack.top() -= a;
                m_ip++;
            } break;
            case inst::type::mult: {
                if (m_stack.size() < 2) {
                    return trap::stack_underflow;
                }
                auto a = std::move(m_stack.top());
                m_stack.pop();
                m_stack.top() *= a;
                m_ip++;
            } break;
            case inst::type::div: {
                if (m_stack.size() < 2) {
                    return trap::stack_underflow;
                }
                auto a = std::move(m_stack.top());
                m_stack.pop();
                if (a == 0) {
                    return trap::div_by_zero;
                }
                m_stack.top() /= a;
                m_ip++;
            } break;
            case inst::type::jmp: {
                if (i.operand() < 0) {
                    return trap::illegal_jump;
                }
                m_ip = i.operand();
            } break;
            case inst::type::halt:
                m_halt = true;
                break;
            default:
                return trap::illegal_inst;
        }
        return trap::ok;
    }

    void dump() const {
        fmt::print("stack:\n");
        for (size_t i = 0; i < m_stack.size(); ++i) {
            fmt::print("    {}\n", m_stack.get(i));
        }
        fmt::print("\n");
    }

    size_t ip() const {
        return m_ip;
    }

    bool halt() const {
        return m_halt;
    }
};

constexpr std::array program = {
        inst::push(69),
        inst::push(420),
        inst::plus(),
        inst::push(4),
        inst::div(),
        inst::halt(),
};

int main() {
    machine bm;
    while (!bm.halt()) {
        fmt::print("{}\n", magic_enum::enum_name(program[bm.ip()].ty()));
        auto t = bm.execute(program[bm.ip()]);
        bm.dump();
        if (t != trap::ok) {
            fmt::print("trap activated: {}\n", magic_enum::enum_name(t));
            return 1;
        }
    }
}
