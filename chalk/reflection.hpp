#pragma once
#include <vector>
#include <functional>
#include <any>
#include <memory>

#include "chalk.hpp"

namespace chalk::reflection
{
    class CoreTypeReflection
    {
        std::string userName;
        std::string typeName;
        size_t size;
    };

    template<typename TClass, typename TMem>
    union ptrmem_cast {
        TMem TClass::*ptr;
        uintptr_t utr;
    };

    class SimpleReflection
    {
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

    template<typename TLinee, typename TMirror>
    class ReflectionOutliner
        : public OutlinerBase<ReflectionOutliner<TLinee, TMirror>> 
    {
        public:
            using ReflectionResult = typename TMirror::ReflectionResult;
            using Linee = TLinee;
        private:
            ReflectionResult _result;

        public:
            class EntryBuilder
                : public OutlinerBase<EntryBuilder>
            {
                    bool _final;
                public:
                    ReflectionOutliner& parent;
                    ReflectionResult::Entry entry;

                    EntryBuilder(ReflectionOutliner& parent, ReflectionResult::Entry::Kind kind)
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

                    constexpr auto operator% (tags::name n) {
                        entry.name = n.value;
                        return std::move(*this);
                    };
            };
        public:
            ReflectionOutliner() {
                _result.type_name = typeid(TLinee).name();
            }

            using Result = std::shared_ptr<ReflectionResult>;

            constexpr auto reify() { return std::make_shared<ReflectionResult>(_result); }

            // using "ignore unknown"
            using OutlinerBase<ReflectionOutliner<TLinee, TMirror>>::operator%;

            constexpr auto& operator% (name n) {
                _result.name = n.value;
                return *this;
            };

            template<typename TBase>
            constexpr auto& operator% (base<TBase> b) {
                outline<TBase>(this);
                return *this;
            };

            template<typename TBase, typename FRet, typename... FArgs>
                requires std::is_base_of_v<TBase, TLinee> || std::is_same_v<TBase, TLinee>
            constexpr auto operator% (tags::func<TBase, FRet, FArgs...> f) {
                return EntryBuilder(*this, ReflectionResult::Entry::Function)
                    .template with_type<decltype(f.value)>()
                    .template with_ptr(ptrmem_cast {f.value}.utr);
            };

            template<typename TBase, typename TMem>
                requires std::is_base_of_v<TBase, TLinee> || std::is_same_v<TBase, TLinee>
            constexpr auto operator% (tags::mem<TBase, TMem> m) {
                return EntryBuilder(*this, ReflectionResult::Entry::Member)
                    .template with_type<TMem>()
                    .template with_ptr(ptrmem_cast {m.value}.utr);
            };
    };

    class Repository
    {
        public:
            template<typename T>
            using Outliner = ReflectionOutliner<T, Mirror>;
        private:
            std::shared_ptr<Repository> _meta;
            std::shared_ptr<Repository> _supplemental;

            struct Entry {

            };

            std::vector<Entry> _primary;
            std::vector<Entry> _secondary;
            
        public:
            template<typename T>
            void outline() {
                auto reflection = outline<T, Outliner>();
            };
    };

    class Mirror
    {
        public:
            using ReflectionResult = SimpleReflection;
    };
}
