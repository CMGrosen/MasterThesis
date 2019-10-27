#include "antlr4-runtime.h"
#include "SmallVisitor.h"
#include <nodes/nodes.hpp>
#include "symbol.hpp"


/**
 * This class provides an empty implementation of SmallVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  DST : public SmallVisitor {
private:
    //node startNode;
    std::vector<node> currentnodes;
    std::unordered_map<std::string, functionDeclarationNode> functionTable;
    int scopelvl = 0;
    int threadnumber = 0;
    int order = 0;
    std::shared_ptr<node> firstStatement;
    std::shared_ptr<node> prevStatement;
public:
/*
    node getStartNode(){
        return startNode;
    }
*/
    std::vector<std::unordered_map<std::string, symbol>> symboltables;

    virtual antlrcpp::Any visitFile(SmallParser::FileContext *ctx) override {
        //antlrcpp::Any result = visitChildren(ctx);
        std::shared_ptr<scopeNode> a = visitMain(ctx->main());
        //std::cout << a.size() << std::endl;
        /*
        if(auto x = dynamic_cast<literalNode*>(dynamic_cast<additionNode*>(a[0]->value)->getLeft())){
            std::cout << "left literal = " << x->value << std::endl;
        }
        if(auto x = dynamic_cast<additionNode*>(a[0]->value)){
            std::cout << "operator = " << x->getOperator() << std::endl;
        }
        if(auto x = dynamic_cast<literalNode*>(dynamic_cast<additionNode*>(a[0]->value)->getRight())){
            std::cout << "right literal = " << x->value << std::endl;
        }
         */

        std::cout << "\n\n\nwriting out what we have:\n";
        for(std::shared_ptr<node> x : a->dcls){
            WriteType(x);
            std::cout << "\nwriting next node:\n";
        }
        //std::cout << dynamic_cast<literalNode*>(dynamic_cast<additionNode*>(a[0]->value)->getLeft())->value << std::endl;
        a->setNextStatement(firstStatement);
        std::cout << "File " << order++ << std::endl;
        return 0;
    }

    virtual antlrcpp::Any visitMain(SmallParser::MainContext *ctx) override {
        //std::cout << "main" << ctx->depth() << "\n";
        std::shared_ptr<scopeNode> node = (visitScope(ctx->scope()));
        // pointer til gamle startNode
        std::cout << "Main " << order++ << std::endl;
        return node;
    }

    virtual antlrcpp::Any visitFuncs(SmallParser::FuncsContext *ctx) override {
        //std::cout << "functions" << ctx->depth() << "\n";
        auto result = visitChildren(ctx);
        std::cout << "Funcdcls " << order++ << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitFunc(SmallParser::FuncContext *ctx) override {
        //std::cout << "func" << ctx->depth() << "\n";
        auto result = visitChildren(ctx);
        // gem funcNode i functionTable
        std::cout << "Funcdcl " << order++ << " " << ctx->getText() << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitDcls(SmallParser::DclsContext *ctx) override {
        std::vector<std::shared_ptr<declarationNode>> nodes;
        if(ctx->dcls()) {
            std::shared_ptr<declarationNode> intermediate = visitDcl(ctx->dcl());
            nodes.push_back( intermediate);
            if (prevStatement) {
                prevStatement->setNextStatement(intermediate);
            }
            prevStatement = intermediate;
            std::vector<std::shared_ptr<declarationNode>> res = visitDcls(ctx->dcls());
            for (auto n : res)
                nodes.emplace_back(n);
            return nodes;
        }

        //auto result = visitChildren(ctx);
        std::cout << "Dcls " << order++ << std::endl;
        return nodes;
        //return result;
    }

    virtual antlrcpp::Any visitDcl(SmallParser::DclContext *ctx) override {
        //auto result = visitChildren(ctx);
        std::string name = ctx->NAME()->getText();
        if (ctx->assign()) {
            std::shared_ptr<expressionNode> expr =  std::move(visit(ctx->assign()));
            if (prevStatement) {
                prevStatement->setNextStatement(expr);
            }
            prevStatement = expr;
            return std::make_shared<declarationNode>(declarationNode(expr->getType(), name, expr));
            //return declarationNode(name, visit(ctx->assign()));
        } else {
            // gem symbol i symbol table
            std::cout << "Dcl " << order++ << " " << ctx->getText() << std::endl;
            //return declarationNode(result.);
            //return node();
            //return result;
        }
    }

    virtual antlrcpp::Any visitStmts(SmallParser::StmtsContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "Stmts " << order++ << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitStmt(SmallParser::StmtContext *ctx) override {
        auto result = visitChildren(ctx);

        std::cout << "stmt " << order++ << " " << ctx->getText() << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitAssign(SmallParser::AssignContext *ctx) override {
        //std::cout << "assignment" << ctx->depth() << "\n";
        //auto result = visitChildren(ctx);
        std::cout << "Assign " << order++  << std::endl;
        return visitExpr(ctx->expr());
        //return result;
    }

    virtual antlrcpp::Any visitIter(SmallParser::IterContext *ctx) override {
        //std::cout << "while" << ctx->depth() << "\n";
        auto result = visitChildren(ctx);
        std::cout << "While " << order++ << " " << ctx->getText() << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitIfs(SmallParser::IfsContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "if " << order++  << " " << ctx->getText() << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitReturnStmt(SmallParser::ReturnStmtContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "return " << order++  << " " << ctx->getText() << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitThread(SmallParser::ThreadContext *ctx) override {
        threadnumber++;
        auto result = visitChildren(ctx);
        threadnumber--;
        return result;
    }

    virtual antlrcpp::Any visitEvent(SmallParser::EventContext *ctx) override {
        auto result = visitChildren(ctx);
        
        return result;
    }

    virtual antlrcpp::Any visitScope(SmallParser::ScopeContext *ctx) override {
        std::vector<std::shared_ptr<declarationNode>> dcls = visitDcls(ctx->dcls());
        std::cout << "scope" << std::endl;
        std::shared_ptr<scopeNode> result = std::make_shared<scopeNode>(scopeNode(Type::okType));
        result->dcls = dcls;
        if (prevStatement) {
            prevStatement->setNextStatement(result);
        }
        prevStatement = result;
        return result;
    }

    virtual antlrcpp::Any visitRead(SmallParser::ReadContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "read " << order++ << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitWrite(SmallParser::WriteContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "write " << order++ << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitOutput(SmallParser::OutputContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "output " << order++ << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitExpr(SmallParser::ExprContext *ctx) override {
        std::cout << ctx->OP_ADD() << " " << ctx->OP_SUB() << " " << ctx->OP_MUL()
        << " " << ctx->OP_DIV() << " " << ctx->OP_MOD() << " " << ctx->literal() << "\n";
        if(ctx->OP_ADD()) {
            std::cout << "add " << order++ << " " << ctx->getText() << std::endl;
            return binary_expression(ctx, '+');
        } else if (ctx->OP_SUB()){
            std::cout << "sub" << order++ << " " << ctx->getText() << std::endl;
            return binary_expression(ctx, '-');
        } else if (ctx->OP_MUL()) {
            std::cout << "mul" << order++ << " " << ctx->getText() << std::endl;
            return binary_expression(ctx, '*');
        } else if (ctx->OP_DIV()) {
            std::cout << "div" << order++ << " " << ctx->getText() << std::endl;
            return binary_expression(ctx, '/');
        } else if (ctx->OP_MOD()) {
            std::cout << "mod" << order++ << " " << ctx->getText() << std::endl;
            return binary_expression(ctx, '%');
        } else if (ctx->literal()){
            std::cout << "literal " << order++ << " " << ctx->getText() << std::endl;
            std::shared_ptr<expressionNode> p = (std::shared_ptr<expressionNode>)std::make_shared<literalNode>(literalNode(std::stoi(ctx->literal()->getText())));
            if (prevStatement) {
                prevStatement->setNextStatement(p);
            }
            prevStatement = p;
            if (!firstStatement) firstStatement = p;
            return p;
        }
        //auto result = visitChildren(ctx);
        std::cout << "Expression " << order++ << " " << ctx->getText() << std::endl;
        //return result;
    }

    virtual antlrcpp::Any visitArrayAccess(SmallParser::ArrayAccessContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "array " << order++ << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitFunctionCall(SmallParser::FunctionCallContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "funcCall " << order++ << " " << ctx->getText() << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitParams(SmallParser::ParamsContext *ctx) override {
        auto result = visitChildren(ctx);
        
        return result;
    }

    virtual antlrcpp::Any visitParam(SmallParser::ParamContext *ctx) override {
        auto result = visitChildren(ctx);
        std::cout << "param " << order++ << " " << ctx->getText() << std::endl;
        return result;
    }

    virtual antlrcpp::Any visitLiteral(SmallParser::LiteralContext *ctx) override {
        literalNode* result = visitChildren(ctx);
        std::cout << "lit " << order++ << " " << ctx->getText() << std::endl;
        //return std::stoi(ctx->getText())
        return result;
    }
    void WriteType(const std::shared_ptr<node> input){
        WriteType(input.get());
    }
    void WriteType(node* input){
        if(auto dcl = dynamic_cast<declarationNode*>(input)){
            std::cout << "dcl" << std::endl;
            WriteType(dcl->value);
        } else if (auto add = dynamic_cast<additionNode*>(input)) {
            std::cout << "add" << std::endl;
            WriteType(add->getLeft());
            std::cout << add->getOperator() << std::endl;
            WriteType(add->getRight());
        }  else if (auto sub = dynamic_cast<subtractionNode*>(input)) {
            std::cout << "sub" << std::endl;
            WriteType(sub->getLeft());
            std::cout << sub->getOperator() << std::endl;
            WriteType(sub->getRight());
        }  else if (auto mul = dynamic_cast<multiplicationNode*>(input)) {
            std::cout << "mul" << std::endl;
            WriteType(mul->getLeft());
            std::cout << mul->getOperator() << std::endl;
            WriteType(mul->getRight());
        } else if (auto div = dynamic_cast<divisionNode*>(input)) {
            std::cout << "div" << std::endl;
            WriteType(div->getLeft());
            std::cout << div->getOperator() << std::endl;
            WriteType(div->getRight());
        } else if (auto mod = dynamic_cast<moduloNode*>(input)) {
            std::cout << "mod" << std::endl;
            WriteType(mod->getLeft());
            std::cout << mod->getOperator() << std::endl;
            WriteType(mod->getRight());
        } else if (auto lit = dynamic_cast<literalNode*>(input)) {
            std::cout << "lit" << std::endl;
        } else if (auto expr = dynamic_cast<expressionNode*>(input)) {
            std::cout << "expr" << std::endl;
            WriteType(expr);
        } else {
            std::cout << "failure" << std::endl;
        }
    }
    std::shared_ptr<expressionNode> binary_expression (SmallParser::ExprContext *ctx, char expressionType) {
        std::shared_ptr<expressionNode> l = (visitExpr(ctx->left));
        if (prevStatement) {
            prevStatement->setNextStatement(l);
        }
        prevStatement = l;
        std::shared_ptr<expressionNode> r = (visitExpr(ctx->right));
        if (prevStatement) {
            prevStatement->setNextStatement(r);
        }
        prevStatement = r;
        Type t;
        if (l->getType() == r->getType()) {
            t = l->getType();
        } else {
            t = errorType;
        }
        std::shared_ptr<expressionNode> p;
        switch (expressionType) {
            case '+':
                p =  (std::shared_ptr<expressionNode>)std::make_shared<additionNode>(additionNode(t,l,r));
                break;
            case '-':
                p = (std::shared_ptr<expressionNode>)std::make_shared<subtractionNode>(subtractionNode(t,l,r));
                break;
            case '*':
                p =  (std::shared_ptr<expressionNode>)std::make_shared<multiplicationNode>(multiplicationNode(t,l,r));
                break;
            case '/':
                p =  (std::shared_ptr<expressionNode>)std::make_shared<divisionNode>(divisionNode(t,l,r));
                break;
            case '%':
                p =  (std::shared_ptr<expressionNode>)std::make_shared<moduloNode>(moduloNode(t,l,r));
                break;
            default:
                p =  nullptr;
                break;
        }
        if (prevStatement) {
            prevStatement->setNextStatement(p);
        }
        prevStatement = p;
        //p->setNextStatement(nextStatement);
        //nextStatement = p;
        return p;
    }
};