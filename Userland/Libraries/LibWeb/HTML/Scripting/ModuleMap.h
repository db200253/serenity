/*
 * Copyright (c) 2022-2023, networkException <networkexception@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/URL.h>
#include <LibJS/Heap/Cell.h>
#include <LibJS/Heap/HeapFunction.h>
#include <LibWeb/HTML/Scripting/ModuleScript.h>

namespace Web::HTML {

class ModuleLocationTuple {
public:
    ModuleLocationTuple(URL url, ByteString type)
        : m_url(move(url))
        , m_type(move(type))
    {
    }

    URL const& url() const { return m_url; }
    ByteString const& type() const { return m_type; }

    bool operator==(ModuleLocationTuple const& other) const
    {
        return other.url() == m_url && other.type() == m_type;
    }

private:
    URL m_url;
    ByteString m_type;
};

// https://html.spec.whatwg.org/multipage/webappapis.html#module-map
class ModuleMap final : public JS::Cell {
    JS_CELL(ModuleMap, Cell);
    JS_DECLARE_ALLOCATOR(ModuleMap);

public:
    ModuleMap() = default;
    ~ModuleMap() = default;

    enum class EntryType {
        Fetching,
        Failed,
        ModuleScript
    };

    struct Entry {
        EntryType type;
        JS::GCPtr<JavaScriptModuleScript> module_script;
    };

    using CallbackFunction = JS::NonnullGCPtr<JS::HeapFunction<void(Entry)>>;

    bool is_fetching(URL const& url, ByteString const& type) const;
    bool is_failed(URL const& url, ByteString const& type) const;

    bool is(URL const& url, ByteString const& type, EntryType) const;

    Optional<Entry> get(URL const& url, ByteString const& type) const;

    AK::HashSetResult set(URL const& url, ByteString const& type, Entry);

    void wait_for_change(JS::Heap&, URL const& url, ByteString const& type, Function<void(Entry)> callback);

private:
    virtual void visit_edges(JS::Cell::Visitor&) override;

    HashMap<ModuleLocationTuple, Entry> m_values;
    HashMap<ModuleLocationTuple, Vector<CallbackFunction>> m_callbacks;

    bool m_firing_callbacks { false };
};

}

namespace AK {

template<>
struct Traits<Web::HTML::ModuleLocationTuple> : public DefaultTraits<Web::HTML::ModuleLocationTuple> {
    static unsigned hash(Web::HTML::ModuleLocationTuple const& tuple)
    {
        return pair_int_hash(tuple.url().to_byte_string().hash(), tuple.type().hash());
    }
};

}
