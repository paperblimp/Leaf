
// header
#include "history.hpp"

// local
#include "utils/asserts.hpp"



History::History(size_t _max_length): max_length{_max_length}
{
    leaf_assert(_max_length < SIZE_MAX);
}


void History::push_action(std::unique_ptr<Action> action)
{
    auto new_size = this->current_idx.has_value() ? this->current_idx.value() + 1: 0;
    this->actions.resize(new_size);
    this->actions.push_back(std::move(action));

    if (this->actions.size() > this->max_length)
        this->actions.erase(this->actions.begin());

    this->current_idx = this->actions.size() - 1;
}

void History::undo()
{
    if (this->actions.size() == 0 || this->current_idx.has_value() == false)
        return;

    this->actions[this->current_idx.value()]->revert();

    if (this->current_idx.value() == 0)
        this->current_idx = std::nullopt;
    else
        this->current_idx.value() -= 1;
}

void History::redo()
{
    if (this->actions.size() == 0 || (this->current_idx.has_value() && this->current_idx.value() == this->actions.size() - 1))
        return;
    
    if (this->current_idx.has_value())
    {
        this->actions[this->current_idx.value() + 1]->apply();
        this->current_idx.value() += 1;
    }
    else
    {
        this->actions[0]->apply();
        this->current_idx = 0;
    }
}


ActionGroup::ActionGroup(std::vector<std::unique_ptr<Action>> _actions): actions{std::move(_actions)} {}

void ActionGroup::apply() const
{
    for (auto& action: this->actions)
        action->apply();
}

void ActionGroup::revert() const
{
    for (auto& action: this->actions)
        action->revert();
}

ActionGroup::~ActionGroup() = default;
