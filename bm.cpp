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

  public:
    machine() = default;

    trap execute(const inst& i) {
        switch (i.ty()) {
            case inst::type::push:
                if (m_stack.full()) {
                    return trap::stack_overflow;
                }
                m_stack.push(i.operand());
                break;
            case inst::type::plus: {
                if (m_stack.size() < 2) {
                    return trap::stack_underflow;
                }
                auto a = std::move(m_stack.top());
                m_stack.pop();
                m_stack.top() += a;
            } break;
            case inst::type::minus: {
                if (m_stack.size() < 2) {
                    return trap::stack_underflow;
                }
                auto a = std::move(m_stack.top());
                m_stack.pop();
                m_stack.top() -= a;
            } break;
            case inst::type::mult: {
                if (m_stack.size() < 2) {
                    return trap::stack_underflow;
                }
                auto a = std::move(m_stack.top());
                m_stack.pop();
                m_stack.top() *= a;
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
            } break;
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
};

constexpr std::array program = {
        inst::push(69),
        inst::push(420),
        inst::plus(),
        inst::push(0),
        inst::div(),
};

int main() {
    machine bm;
    for (auto i : program) {
        fmt::print("{}\n", magic_enum::enum_name(i.ty()));
        auto t = bm.execute(i);
        bm.dump();
        if (t != trap::ok) {
            fmt::print("trap activated: {}\n", magic_enum::enum_name(t));
            return 1;
        }
    }
}
