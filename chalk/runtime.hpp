#pragma once
#include <vector>
#include <functional>
#include <any>
#include <memory>

#include "chalk.hpp"

namespace chalk
{
    template<typename TClass, typename TMem>
    union ptrmem_cast {
        TMem TClass::*ptr;
        uintptr_t utr;
    };

    class SimpleReflection {
        public:
            struct Entry {
                enum Kind {
                    Function,
                    Member,
                    Data,
                };

                Kind kind;
                std::string name;
                std::string type_name;
                uintptr_t ptr;
                std::function<std::any (std::vector<std::any>)> use;

                Entry(Kind kind)
                    : kind(kind), name(), type_name(), use()
                    { }
            };

            std::string name;
            std::string type_name;
            std::vector<Entry> entries;
    };

    template<typename T>
    class RuntimeReflector
            : public ReflectorBase<RuntimeReflector<T>> 
        {
            SimpleReflection result;
        public:
            using Reflectee = T;
        public:
            class EntryBuilder
                : public ReflectorBase<EntryBuilder>
            {
                    bool _final;
                public:
                    RuntimeReflector& parent;
                    SimpleReflection::Entry entry;

                    EntryBuilder(RuntimeReflector& parent, SimpleReflection::Entry::Kind kind)
                        : _final(true), parent(parent), entry(kind)
                        { }
                    EntryBuilder(EntryBuilder const& old_self) = delete;
                    EntryBuilder(EntryBuilder&& old_self)
                        : _final(true), parent(old_self.parent), entry(old_self.entry)
                        {
                            old_self._final = false;
                        }
                    auto operator=(EntryBuilder const& old_self) = delete;
                    auto operator=(EntryBuilder&& old_self) = delete;

                    ~EntryBuilder() {
                        if (_final)
                            parent.result.entries.emplace_back(entry);
                    }

                    template<typename TType>
                    constexpr auto with_type () {
                        entry.type_name = typeid(TType).name();
                        return std::move(*this);
                    };

                    constexpr auto with_ptr (uintptr_t ptr) {
                        entry.ptr = ptr;
                        return std::move(*this);
                    };

                    constexpr auto with_use (std::function<std::any (std::vector<std::any>)> use) {
                        entry.use = use;
                        return std::move(*this);
                    };

                    constexpr auto operator% (name n) {
                        entry.name = n.value;
                        return std::move(*this);
                    };
            };
        public:
            RuntimeReflector() {
                result.type_name = typeid(T).name();
            }

            using Result = std::shared_ptr<SimpleReflection>;

            constexpr auto reify() { return std::make_shared<SimpleReflection>(result); }

            // using "ignore unknown"
            using ReflectorBase<RuntimeReflector<T>>::operator%;

            constexpr auto& operator% (name n) {
                result.name = n.value;
                return *this;
            };

            template<typename TBase>
            constexpr auto& operator% (base<TBase> b) {
                *this = TBase::reflect(*this);
                return *this;
            };

            template<typename TBase, typename FRet, typename... FArgs>
                requires std::is_base_of_v<TBase, T>
            constexpr auto operator% (func<TBase, FRet, FArgs...> f) {
                return EntryBuilder(*this, SimpleReflection::Entry::Function)
                    .template with_type<decltype(f.value)>()
                    .template with_ptr(ptrmem_cast {f.value}.utr);
            };

            template<typename TBase, typename TMem>
                requires std::is_base_of_v<TBase, T>
            constexpr auto operator% (mem<TBase, TMem> m) {
                return EntryBuilder(*this, SimpleReflection::Entry::Member)
                    .template with_type<TMem>()
                    .template with_ptr(ptrmem_cast {m.value}.utr);
            };
    };
}
