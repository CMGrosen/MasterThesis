//
// Created by hu on 15/11/2019.
//

#ifndef ANTLR_CPP_TUTORIAL_EVENTNODE_HPP
#define ANTLR_CPP_TUTORIAL_EVENTNODE_HPP

class eventNode : public node {
public:
    eventNode(Type t, std::shared_ptr<node> condition) : node(t,Event),_condition{std::move(condition)} {}
    const node *getCondition() const {return _condition.get();}
private:
    std::shared_ptr<node> _condition;
};

#endif //ANTLR_CPP_TUTORIAL_EVENTNODE_HPP
