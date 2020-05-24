//
// Created by hu on 23/03/2020.
//

#include "interpreter.hpp"

interpreter::interpreter(symEngine e) : engine{std::move(e)} {

}

bool interpreter::run() {
    if(engine.execute()) {
        for (const auto &pair : engine.possible_raceconditions) {
            if(engine.updateModel(pair.second)) {
                refresh();
                reach_potential_raceConditions(pair.first);
            }
        }
        return true;
    } else {
        return false;
    }
}



void interpreter::update() {

    data.run1.ccfg = std::make_unique<CSSA_CCFG>(CSSA_CCFG(*engine.ccfg));
    data.run2.ccfg = std::make_unique<CSSA_CCFG>(CSSA_CCFG(*engine.ccfg));

    auto res = engine.getModel();

    //values
    for (const auto &val : res.values) {
        if (val.first.find(_run1) != std::string::npos) {
            int32_t len = val.first.size() - std::string(_run1).size();
            data.run1.variableValues.insert({val.first.substr(0, len), val.second});
        } else if (val.first.find(_run2) != std::string::npos) {
            int32_t len = val.first.size() - std::string(_run2).size();
            data.run2.variableValues.insert({val.first.substr(0, len), val.second});
        } else { //-readVal
            data.run1.symbolicInputs.insert({val.first, std::stoi(val.second.value)});
            data.run2.symbolicInputs.insert({val.first, std::stoi(val.second.value)});
        }
    }

    for (const auto &interleaving : res.interleavings) {
        if (interleaving.second.second) { //Taken interleaving
            std::string name = interleaving.first.substr(0,interleaving.first.size() - std::string(_run1).size());
            if (interleaving.second.first == _run1) { //run1
                data.run1.interleavingsTaken.insert(name);
            } else { //run2
                data.run2.interleavingsTaken.insert(name);
            }
        }
    }

    for (const auto &path : res.paths) {
        if (path.second.second) { //Path taken
            std::string name = path.first.substr(0,path.first.size() - std::string(_run1).size());
            if (path.second.first == _run1) {
                data.run1.blocksVisited.insert(name);
            } else {
                data.run2.blocksVisited.insert(name);
            }
        }
    }

    for (const auto &concEnd : data.run1.ccfg->endconcNodes) {
        data.run1.threads_to_finish[concEnd];
        for (const auto &p : concEnd->parents) {
            data.run1.threads_to_finish[concEnd].insert(p.lock().get());
        }
    }

    for (const auto &concEnd : data.run2.ccfg->endconcNodes) {
        data.run2.threads_to_finish[concEnd];
        for (const auto &p : concEnd->parents) {
            data.run2.threads_to_finish[concEnd].insert(p.lock().get());
        }
    }

    for (const auto &blk : data.run1.ccfg->nodes) {
        if (data.run1.blocksVisited.find(blk->get_name()) != data.run1.blocksVisited.end()) {
            data.run1.frontier.insert(blk);
        }
    }
    for (const auto &blk : data.run2.ccfg->nodes) {
        if (data.run2.blocksVisited.find(blk->get_name()) != data.run2.blocksVisited.end()) {
            data.run2.frontier.insert(blk);
        }
    }

    //Ignore picked variableOptions for Now

}

void interpreter::refresh() {
    data.clear();
    update();
}

bool interpreter::reach_potential_raceConditions(const std::string& pifunction) {
    auto *pi = reinterpret_cast<piNode*>(engine.ccfg->defs[pifunction]->defsite[pifunction].get());
    bool validModel = find_race(data.run1, *pi);

    if (pi->getName() == "-T_a_3") {
        std::cout << "here";
    } else if (pi->getName() == "-T_a_5") {
        std::cout << "here";
    }
    if (validModel) { //don't bother checking run2, if run1 is not valid
        validModel = find_race(data.run2, *pi);
    }
    if (validModel) { //if both runs are valid and the data-race is found
        std::cout << report_datarace(*pi) << "\n";
    }

    return validModel;
}


bool interpreter::find_race(runInformation &run, const piNode &piName) {
    std::shared_ptr<basicblock> defBlk = run.ccfg->defs[piName.getName()];
    std::deque<std::shared_ptr<basicblock>> blks = {run.ccfg->startNode};
    size_t invalidsChecked = 0;
    bool checked_deque = false;
    while (!blks.empty() && invalidsChecked < (blks.size()*2)) {
        std::shared_ptr<basicblock> currentblk = blks.front();
        if (defBlk == currentblk) { //This block is where the pi-function is present
            auto found = run.varValuesDuringExecution.find(piName.getVar());
            if (found != run.varValuesDuringExecution.end()) { //If the variable has been assigned a value
                if (run.variableValues.find(piName.getName())->second.value == found->second) { //Found datarace. Values match too
                    return true;
                }
            }
        }

        blks.pop_front();
        run.frontier.erase(currentblk);
        std::pair<bool, std::vector<std::shared_ptr<basicblock>>> applied_and_successors = applied_semantics(currentblk, run, blks, checked_deque);

        if (applied_and_successors.first) { //If applied, add block itself to front, or add successors
            invalidsChecked = 0;
            if (applied_and_successors.second.empty()) {
                if (currentblk == run.ccfg->exitNode) {
                    return false;
                } else {
                    blks.push_front(currentblk);
                    run.frontier.insert(currentblk);
                }
            } else if (!applied_and_successors.second.front()) {
                //We visited an event which cannot continue
                //In the model from SMT, this is then the intention,
                // so don't visit any additional blocks along this thread

            } else {
                if (applied_and_successors.second.front()->type == Coend) {
                    run.threads_to_finish[applied_and_successors.second.front()].erase(currentblk.get());
                }
                int i = applied_and_successors.second.size();
                while (i-->0) { //There are multiple successors in case of a fork statement
                    //Add from back to front because we wish to visit thread 1 before thread n
                    blks.push_front(applied_and_successors.second[i]);

                }
            }
        } else { //cannot apply anything to this block yet, model does not allow it
            if (!checked_deque) ++invalidsChecked;
            checked_deque = false;
            blks.push_back(currentblk);
            run.frontier.insert(currentblk);
        }
    }
    return false;
}

std::pair<bool, std::vector<std::shared_ptr<basicblock>>>
interpreter::applied_semantics(std::shared_ptr<basicblock> &blk, runInformation &run, std::deque<std::shared_ptr<basicblock>> &current_blocks, bool &checked_deque) {

    if (blk->type == Coend) {
        if (!run.threads_to_finish[blk].empty()) { //Still threads to join
            return {false, {}};
        }
    }


    std::shared_ptr<statementNode> stmt = blk->statements.front();

    bool success = true;
    std::vector<std::shared_ptr<basicblock>> nexts;
    switch (stmt->getNodeType()) {
        case Assign: {
            auto ass = reinterpret_cast<assignNode*>(stmt.get());
            if (ass->getExpr()->getNodeType() == Literal) {
                std::string newVal = reinterpret_cast<literalNode*>(ass->getExpr())->value;

                if (blk->concurrentBlock.first) {
                    for (const std::shared_ptr<edge> &interleaving : run.ccfg->conflict_edges_from[blk]) {
                        if (run.interleavingsTaken.find(interleaving->name) != run.interleavingsTaken.end()) {
                            //Found an interleaving, check if the block in question is in deque
                            for (const auto &o : current_blocks) {
                                if (o == interleaving->to()) { //it is in the deque, just do assignmment
                                    run.varValuesDuringExecution[ass->getOriginalName()] = newVal;
                                    run.lastAssignmentToKey[ass->getOriginalName()] = engine.ccfg->defs[ass->getName()]->defsite[ass->getName()];
                                    blk->statements.front() = stmt = nullptr;
                                    return {true, blk->nexts};
                                }
                            }
                            return {false, {}};
                        }
                    }
                }

                if (run.variableValues.find(ass->getName())->second.value != newVal) {
                    return {false, {}};
                } else {
                    run.varValuesDuringExecution[ass->getOriginalName()] = reinterpret_cast<literalNode *>(ass->getExpr())->value;
                    run.lastAssignmentToKey[ass->getOriginalName()] = engine.ccfg->defs[ass->getName()]->defsite[ass->getName()];
                    blk->statements.front() = stmt = nullptr;
                }
            } else {
                auto res = exec_expr(ass->getExpr(), run);
                success = res.first;
                if (res.first) {
                    ass->setExpr(res.second);
                }
            }
            break;
        }

        case If: {
            auto ifStmt = reinterpret_cast<ifElseNode*>(stmt.get());
            if (ifStmt->getCondition()->getNodeType() == Literal) {
                if (reinterpret_cast<literalNode*>(ifStmt->getCondition())->value == "true") {
                    nexts.push_back(blk->nexts.front());
                } else {
                    nexts.push_back(blk->nexts.back());
                }
            } else {
                auto res = exec_expr(ifStmt->getCondition(), run);
                success = res.first;
                if (res.first) {
                    ifStmt->setCondition(res.second);
                }
            }
            break;
        }
        case Write: {
            auto write = reinterpret_cast<writeNode*>(stmt.get());
            if (write->getExpr()->getNodeType() == Literal) {
                blk->statements.front() = stmt = nullptr;
            } else {
                auto res = exec_expr(write->getExpr(), run);
                success = res.first;
                if (res.first) {
                    write->setExpr(res.second);
                }
            }
            break;
        } case Event: {
            auto event = reinterpret_cast<eventNode*>(stmt.get());
            if (event->getCondition()->getNodeType() == Literal) {
                if (reinterpret_cast<literalNode*>(event->getCondition())->value == "true") {
                    nexts.push_back(blk->nexts.front());
                } else {
                    std::cout << "Event is false, future execution here is not possible\n";
                    return {true, {nullptr}};
                }
            } else {
                auto res = exec_expr(event->getCondition(), run);
                success = res.first;
                if (res.first) {
                    event->setCondition(res.second);
                }
            }
            break;
        }


        case Pi: {
            auto pi = reinterpret_cast<piNode*>(stmt.get());
            std::string val = run.variableValues.find(pi->getName())->second.value;
            //If value matches, replace the value in the last statement with the new value
            if (val == run.varValuesDuringExecution.find(pi->getVar())->second) {
                blk->statements.back()->replacePiWithLit(pi->getName(), pi->getType(), val);
                blk->statements.front() = stmt = nullptr;
            } else {
                return {false, {}};
            }
            break;
        }

        //control statements, and single rule fork which expands. These are always last in a block
        case Concurrent:
        case EndConcurrent:
        case EndFi:
            for (const auto &nxt : blk->nexts) nexts.push_back(nxt);
            break;

        //Can always do these. Just control statements
        case Skip:
        case Phi:
        case Assert: //Don't have these
            success = true;
            blk->statements.front() = stmt = nullptr;
            break;

        case AssignArrField: //ignore
        case Sequential: //ignore
        case While: //have no while

        //expressions
        case Read:
        case Literal:
        case ArrayAccess:
        case ArrayLiteral:
        case Variable:
        case BinaryExpression:
        case UnaryExpression:
        case BasicBlock:
            break;
    }

    if (!nexts.empty()) {
        return {true, std::move(nexts)};
    }

    if (!stmt) { //update blocks
        auto oldVec = blk->statements;

        auto newVec = std::vector<std::shared_ptr<statementNode>>{};
        //update vector to remove finished evaluated statement
        for (auto it = oldVec.begin()+1; it != oldVec.end(); ++it) newVec.push_back(*it);

        //If new vector is empty, need to visit future children blocks. Otherwise have to continue with next statement
        if (newVec.empty())
            return {true, blk->nexts};
        else {
            blk->statements = std::move(newVec);
            return {true, {}};
        }
    } else { //
        return {success, {}};
    }
}










static std::string btos(bool val) {
    return val ? "true" : "false";
}

std::string interpreter::compute_operator(const std::string &left, const std::string &right, op _operator) {
    std::string val;
    switch (_operator) {
        case PLUS:
            val = std::to_string(std::stoi(left) + std::stoi(right));
            break;
        case MINUS:
            val = std::to_string(std::stoi(left) - std::stoi(right));
            break;
        case MULT:
            val = std::to_string(std::stoi(left) * std::stoi(right));
            break;
        case DIV:
            val = std::to_string(std::stoi(left) / std::stoi(right));
            break;
        case MOD:
            val = std::to_string(std::stoi(left) % std::stoi(right));
            break;
        case NOT:
            val = btos(left == "false");
            break;
        case AND:
            val = btos(left == "true" && right == "true");
            break;
        case OR:
            val = btos(left == "true" || right == "true");
            break;
        case LE:
            val = btos(std::stoi(left) < std::stoi(right));
            break;
        case LEQ:
            val = btos(std::stoi(left) <= std::stoi(right));
            break;
        case GE:
            val = btos(std::stoi(left) > std::stoi(right));
            break;
        case GEQ:
            val = btos(std::stoi(left) >= std::stoi(right));
            break;
        case EQ:
            val = btos(left == right);
            break;
        case NEQ:
            val = btos(left != right);
            break;
        case NEG:
            val = std::to_string(-std::stoi(left));
            break;
    }
    return val;
}

std::pair<bool, std::shared_ptr<expressionNode>> interpreter::exec_expr(const expressionNode *expr, runInformation &run) {
    std::pair<bool, std::shared_ptr<expressionNode>> res;

    switch (expr->getNodeType()) {
        case Assign:
        case AssignArrField:
        case Concurrent:
        case EndConcurrent:
        case Sequential:
        case While:
        case If:
        case EndFi:
        case Write:
        case Event:
        case Skip:
        case BasicBlock:
        case Phi:
        case Pi:
        case Assert:
            std::cout << "expression is apparently a statement. error\n";
            assert(false);
            break;

        case Read:
            res =
                {true
                ,std::make_shared<literalNode>(literalNode(intType,
                        std::to_string(run.symbolicInputs[(reinterpret_cast<const readNode*>(expr)->getName())])
                        ))
                };
            break;
        case Literal:
            res = {true, expr->copy_expression()};
            break;
        case ArrayAccess: {
            auto arracc = reinterpret_cast<const arrayAccessNode*>(expr);
            if (arracc->getAccessor()->getNodeType() == Literal) {
                std::string index = reinterpret_cast<literalNode*>(arracc->getAccessor())->value;
                std::string val = run.varValuesDuringExecution[arracc->getVar()->origName + "[" + index + "]"];
                res = {true, std::make_shared<literalNode>(literalNode(arracc->getType(), val))};
            } else {
                auto copy = arracc->copy_expression();
                auto updatedAccessor = exec_expr(copy.get(), run);
                if (updatedAccessor.first) {
                    reinterpret_cast<arrayAccessNode*>(copy.get())->setAccessor(updatedAccessor.second);
                    res = {true, copy};
                } else {
                    res = {false, nullptr};
                }
            }
            break;
        } case ArrayLiteral:
            //don't handle array, this is good enough for now
            break;
        case Variable: {
            auto var = reinterpret_cast<const variableNode*>(expr);
            std::string val = run.varValuesDuringExecution[var->origName];
            auto found = run.variableValues.find(var->name);
            if (found != run.variableValues.end() && found->second.value != val) {
                return {false, nullptr};
            }
            res = {true, std::make_shared<literalNode>(literalNode(var->getType(), val))};
            break;
        } case BinaryExpression: {
            auto binexpr = reinterpret_cast<const binaryExpressionNode*>(expr);
            auto copy = binexpr->copy_expression();
            bool couldUpdate;
            if (binexpr->getLeft()->getNodeType() != Literal) { //Evaluate left if not literal
                auto updated = exec_expr(binexpr->getLeft(), run);
                couldUpdate = updated.first;
                if (updated.first) {
                    reinterpret_cast<binaryExpressionNode*>(copy.get())->setLeft(updated.second);
                }
            } else if (binexpr->getRight()->getNodeType() != Literal) { //Left is now literal, so calculate right if not literal
                auto updated = exec_expr(binexpr->getRight(), run);
                couldUpdate = updated.first;
                if (updated.first) {
                    reinterpret_cast<binaryExpressionNode*>(copy.get())->setRight(updated.second);
                }
            } else { //Both are literal, evaluate the operator
                std::string val = compute_operator( reinterpret_cast<const literalNode*>(binexpr->getLeft())->value
                                                  , reinterpret_cast<const literalNode*>(binexpr->getRight())->value
                                                  , binexpr->getOperator()
                                                  );
                copy = std::make_shared<literalNode>(literalNode(binexpr->getType(), val));
                couldUpdate = true;
            }
            res = {couldUpdate, copy};
            break;
        } case UnaryExpression: {
            auto unexpr = reinterpret_cast<const unaryExpressionNode *>(expr);
            auto copy = unexpr->copy_expression();
            bool couldUpdate;
            if (unexpr->getExpr()->getNodeType() == Literal) {
                std::string val = compute_operator( reinterpret_cast<const literalNode*>(unexpr->getExpr())->value
                                                  , reinterpret_cast<const literalNode*>(unexpr->getExpr())->value
                                                  , unexpr->getOperator()
                                                  );
                copy = std::make_shared<literalNode>(literalNode(unexpr->getType(), val));
                couldUpdate = true;
            } else {
                auto updated = exec_expr(unexpr->getExpr(), run);
                couldUpdate = updated.first;
                if (updated.first) {
                    reinterpret_cast<unaryExpressionNode*>(copy.get())->setExpr(updated.second);
                }
            }
            res = {couldUpdate, copy};
            break;
        }
    }
    return res;
}

std::string interpreter::report_datarace(const piNode &pi) {
    std::shared_ptr<statementNode> stmtUsage = engine.ccfg->defs[pi.getName()]->statements.back();
    std::shared_ptr<statementNode> def1 = data.run1.lastAssignmentToKey[pi.getVar()];
    std::shared_ptr<statementNode> def2 = data.run2.lastAssignmentToKey[pi.getVar()];

    std::string raceconditionStr =
        "Use of variable '" + pi.getVar() + "' in statement: '" + stmtUsage->strOnSourceForm() + "' line " +
        std::to_string(stmtUsage->get_linenum()) + " can have two different values\n"
        + data.run1.varValuesDuringExecution[pi.getVar()] + " defined in statement: '" + def1->strOnSourceForm()
        + "' on line " + std::to_string(def1->get_linenum()) + "\n"
        "and\n"
        + data.run2.varValuesDuringExecution[pi.getVar()] + " defined in statement: '" + def2->strOnSourceForm()
        + "' on line " + std::to_string(def2->get_linenum()) + "\n"
        ;

    return raceconditionStr;
}

interpreter::~interpreter() {
    data.clear();
}
