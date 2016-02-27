#ifndef SPCC_BUFFER_HH
#define SPCC_BUFFER_HH

#include "string_view.hh"
#include "optional.hh"

#include <utility>
#include <cstddef>
#include <memory>
#include <vector>

class buffer {
public:
    virtual string_view name() const = 0;
    virtual string_view data() const = 0;
    virtual const buffer* parent() const = 0;
    virtual string_view original_data() const = 0;
    virtual std::size_t offset_in_original(std::size_t offset) const = 0;
    virtual ~buffer() { }
};

class location {
public:
    location(const buffer& buf, std::size_t offset) :
    buf_(&buf), offset_(offset) { }

    const buffer& buffer() const { return *buf_; }
    std::size_t offset() const { return offset_; }
    location find_spelling_loc() const;
    location next_loc(std::size_t n = 1) { return { buffer(), offset() + n }; }
private:
    const class buffer* buf_;
    std::size_t offset_;
};

class raw_buffer : public buffer {
public:
    raw_buffer(std::string name, std::string data) :
    name_(std::move(name)), data_(std::move(data)) { }

    string_view name() const override { return name_; }
    string_view data() const override { return data_; }
    const buffer* parent() const override { return nullptr; }
    string_view original_data() const override { return data_; };
    std::size_t offset_in_original(std::size_t offset) const override {
        return offset;
    }
private:
    std::string name_;
    std::string data_;
};

class derived_buffer : public buffer {
public:
    derived_buffer(std::unique_ptr<const buffer> parent) :
    parent_(std::move(parent)) { }

    string_view name() const override { return parent_->name(); }
    string_view data() const override { return data_; }
    const buffer* parent() const override { return parent_.get(); }
    string_view original_data() const override {
        return parent_->original_data();
    };
    std::size_t offset_in_original(std::size_t offset) const override;

    string_view peek() const;
    void propagate(std::size_t len);
    void replace(std::size_t len, string_view data);
    void insert(string_view data);
    void erase(std::size_t len);
    bool done() const;
    std::size_t parent_index() const { return index_; }

    struct fragment {
        std::pair<std::size_t, std::size_t> local_range;
        std::pair<std::size_t, std::size_t> parent_range;
        bool propagate;
    };
private:
    std::unique_ptr<const buffer> parent_;
    std::string data_;
    std::size_t index_ = 0;
    std::vector<fragment> fragments_;
};

#endif
