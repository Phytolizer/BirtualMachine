#pragma once

#include <array>
#include <cstddef>
#include <stack>
#include <vector>

namespace phy {

template <typename T, size_t N> class fixed_stack {
    std::array<T, N> m_data;
    std::size_t m_size = 0;

  public:
    T& top() {
        return m_data[m_size - 1];
    }
    const T& top() const {
        return m_data[m_size - 1];
    }
    bool push(const T& t) {
        if (m_size == N) {
            return false;
        }
        m_data[m_size++] = t;
        return true;
    }
    bool push(T&& t) {
        if (m_size == N) {
            return false;
        }
        m_data[m_size++] = std::move(t);
        return true;
    }
    bool pop() {
        if (m_size == 0) {
            return false;
        }
        --m_size;
        return true;
    }
    bool empty() const {
        return m_size == 0;
    }
    bool full() const {
        return m_size == N;
    }
    size_t size() const {
        return m_size;
    }
    T& get(size_t i) {
        return m_data[i];
    }
    const T& get(size_t i) const {
        return m_data[i];
    }
};

} // namespace phy
