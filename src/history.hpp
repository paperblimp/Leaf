
#pragma once

// builtin
#include <vector>
#include <memory>

// local
#include "config.hpp"



struct Action
{
    virtual void apply() const = 0;
    virtual void revert() const = 0;



    virtual ~Action() = default;
};

struct ActionGroup: public Action
{
    private:

        std::vector<std::unique_ptr<Action>> actions;

    public:

        ActionGroup(std::vector<std::unique_ptr<Action>> actions);

        virtual void apply() const override;
        virtual void revert() const override;

        virtual ~ActionGroup();
};


class History
{
    private:

        std::optional<size_t> current_idx = std::nullopt;
        std::vector<std::unique_ptr<Action>> actions;
        const size_t max_length;

    public:

        History(size_t max_length);

        void push_action(std::unique_ptr<Action> action);
        void undo();
        void redo();
};

inline History* history = nullptr;
