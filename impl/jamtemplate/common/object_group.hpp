﻿#ifndef JAMTEMPLATE_OBJECTGROUP_HPP
#define JAMTEMPLATE_OBJECTGROUP_HPP

#include <game_object.hpp>
#include <algorithm>
#include <memory>
#include <vector>

namespace jt {

/// templated Object group for the purpose of holding specific Objects in a separate list.
/// Can be used as a GameObject.
/// For stored objects to be updated and drawn, you need to also add the object to the State itself
///
/// naming convention will differ here from the rest of the project as this is about to mimic the
/// std::vector interface
template <typename T>
class ObjectGroup : public GameObject {
public:
    using Sptr = std::shared_ptr<ObjectGroup<T>>;

    ObjectGroup() = default;

    auto begin() { return m_data.begin(); }

    auto end() { return m_data.end(); }

    auto cbegin() const { return m_data.cbegin(); }

    auto cend() const { return m_data.cend(); }

    bool empty() const { return m_data.empty(); }

    auto size() const { return m_data.size(); }

    auto at(std::size_t idx) { return m_data.at(idx); }

    void push_back(std::weak_ptr<T> wptr) { m_data.push_back(wptr); }

    void emplace_back(std::weak_ptr<T>&& wptr) { m_data.emplace_back(std::move(wptr)); }

    void pop_back()
    {
        if (!m_data.empty()) {
            m_data.pop_back();
        }
    }

    std::weak_ptr<T> back();

    void clear();

private:
    std::vector<std::weak_ptr<T>> m_data {};

    void doUpdate(const float /*elapsed*/) override
    {
        std::erase_if(m_data, [](auto const& wptr) { return wptr.expired(); });
    }
};

template <typename T>
std::weak_ptr<T> ObjectGroup<T>::back()
{
    return m_data.back();
}

template <typename T>
void ObjectGroup<T>::clear()
{
    m_data.clear();
}

template <typename T>
auto begin(ObjectGroup<T>& obj)
{
    return obj.begin();
}

template <typename T>
auto end(ObjectGroup<T>& obj)
{
    return obj.end();
}

} // namespace jt
#endif
