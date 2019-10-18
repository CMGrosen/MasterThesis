
// Generated from Small.g4 by ANTLR 4.7.2


#include "SmallVisitor.h"

#include "SmallParser.h"


using namespace antlrcpp;
using namespace antlr4;

SmallParser::SmallParser(TokenStream *input) : Parser(input) {
  _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

SmallParser::~SmallParser() {
  delete _interpreter;
}

std::string SmallParser::getGrammarFileName() const {
  return "Small.g4";
}

const std::vector<std::string>& SmallParser::getRuleNames() const {
  return _ruleNames;
}

dfa::Vocabulary& SmallParser::getVocabulary() const {
  return _vocabulary;
}


//----------------- FileContext ------------------------------------------------------------------

SmallParser::FileContext::FileContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SmallParser::DeclsContext* SmallParser::FileContext::decls() {
  return getRuleContext<SmallParser::DeclsContext>(0);
}

tree::TerminalNode* SmallParser::FileContext::EOF() {
  return getToken(SmallParser::EOF, 0);
}


size_t SmallParser::FileContext::getRuleIndex() const {
  return SmallParser::RuleFile;
}

antlrcpp::Any SmallParser::FileContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SmallVisitor*>(visitor))
    return parserVisitor->visitFile(this);
  else
    return visitor->visitChildren(this);
}

SmallParser::FileContext* SmallParser::file() {
  FileContext *_localctx = _tracker.createInstance<FileContext>(_ctx, getState());
  enterRule(_localctx, 0, SmallParser::RuleFile);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(10);
    decls();
    setState(11);
    match(SmallParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclsContext ------------------------------------------------------------------

SmallParser::DeclsContext::DeclsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SmallParser::DeclContext* SmallParser::DeclsContext::decl() {
  return getRuleContext<SmallParser::DeclContext>(0);
}

SmallParser::DeclsContext* SmallParser::DeclsContext::decls() {
  return getRuleContext<SmallParser::DeclsContext>(0);
}

tree::TerminalNode* SmallParser::DeclsContext::NEWLINE() {
  return getToken(SmallParser::NEWLINE, 0);
}


size_t SmallParser::DeclsContext::getRuleIndex() const {
  return SmallParser::RuleDecls;
}

antlrcpp::Any SmallParser::DeclsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SmallVisitor*>(visitor))
    return parserVisitor->visitDecls(this);
  else
    return visitor->visitChildren(this);
}

SmallParser::DeclsContext* SmallParser::decls() {
  DeclsContext *_localctx = _tracker.createInstance<DeclsContext>(_ctx, getState());
  enterRule(_localctx, 2, SmallParser::RuleDecls);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(17);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SmallParser::T__1:
      case SmallParser::SIMPLE_IDENTIFIER: {
        enterOuterAlt(_localctx, 1);
        setState(13);
        decl();
        setState(14);
        decls();
        break;
      }

      case SmallParser::NEWLINE: {
        enterOuterAlt(_localctx, 2);
        setState(16);
        match(SmallParser::NEWLINE);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclContext ------------------------------------------------------------------

SmallParser::DeclContext::DeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SmallParser::DeclContext::SIMPLE_IDENTIFIER() {
  return getToken(SmallParser::SIMPLE_IDENTIFIER, 0);
}

SmallParser::ExprContext* SmallParser::DeclContext::expr() {
  return getRuleContext<SmallParser::ExprContext>(0);
}

SmallParser::ValuesContext* SmallParser::DeclContext::values() {
  return getRuleContext<SmallParser::ValuesContext>(0);
}


size_t SmallParser::DeclContext::getRuleIndex() const {
  return SmallParser::RuleDecl;
}

antlrcpp::Any SmallParser::DeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SmallVisitor*>(visitor))
    return parserVisitor->visitDecl(this);
  else
    return visitor->visitChildren(this);
}

SmallParser::DeclContext* SmallParser::decl() {
  DeclContext *_localctx = _tracker.createInstance<DeclContext>(_ctx, getState());
  enterRule(_localctx, 4, SmallParser::RuleDecl);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(33);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SmallParser::SIMPLE_IDENTIFIER: {
        enterOuterAlt(_localctx, 1);
        setState(19);
        match(SmallParser::SIMPLE_IDENTIFIER);
        setState(22);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == SmallParser::T__0) {
          setState(20);
          match(SmallParser::T__0);
          setState(21);
          expr();
        }
        break;
      }

      case SmallParser::T__1: {
        enterOuterAlt(_localctx, 2);
        setState(24);
        match(SmallParser::T__1);
        setState(25);
        match(SmallParser::SIMPLE_IDENTIFIER);
        setState(31);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == SmallParser::T__0) {
          setState(26);
          match(SmallParser::T__0);
          setState(27);
          match(SmallParser::T__2);
          setState(28);
          values();
          setState(29);
          match(SmallParser::T__3);
        }
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ValuesContext ------------------------------------------------------------------

SmallParser::ValuesContext::ValuesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> SmallParser::ValuesContext::INT() {
  return getTokens(SmallParser::INT);
}

tree::TerminalNode* SmallParser::ValuesContext::INT(size_t i) {
  return getToken(SmallParser::INT, i);
}


size_t SmallParser::ValuesContext::getRuleIndex() const {
  return SmallParser::RuleValues;
}

antlrcpp::Any SmallParser::ValuesContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SmallVisitor*>(visitor))
    return parserVisitor->visitValues(this);
  else
    return visitor->visitChildren(this);
}

SmallParser::ValuesContext* SmallParser::values() {
  ValuesContext *_localctx = _tracker.createInstance<ValuesContext>(_ctx, getState());
  enterRule(_localctx, 6, SmallParser::RuleValues);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(35);
    match(SmallParser::INT);
    setState(40);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SmallParser::T__4) {
      setState(36);
      match(SmallParser::T__4);
      setState(37);
      match(SmallParser::INT);
      setState(42);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExprContext ------------------------------------------------------------------

SmallParser::ExprContext::ExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SmallParser::ExprContext::INT() {
  return getToken(SmallParser::INT, 0);
}


size_t SmallParser::ExprContext::getRuleIndex() const {
  return SmallParser::RuleExpr;
}

antlrcpp::Any SmallParser::ExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SmallVisitor*>(visitor))
    return parserVisitor->visitExpr(this);
  else
    return visitor->visitChildren(this);
}

SmallParser::ExprContext* SmallParser::expr() {
  ExprContext *_localctx = _tracker.createInstance<ExprContext>(_ctx, getState());
  enterRule(_localctx, 8, SmallParser::RuleExpr);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(43);
    match(SmallParser::INT);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

// Static vars and initialization.
std::vector<dfa::DFA> SmallParser::_decisionToDFA;
atn::PredictionContextCache SmallParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN SmallParser::_atn;
std::vector<uint16_t> SmallParser::_serializedATN;

std::vector<std::string> SmallParser::_ruleNames = {
  "file", "decls", "decl", "values", "expr"
};

std::vector<std::string> SmallParser::_literalNames = {
  "", "'='", "'arr'", "'['", "']'", "','", "'[\n]'"
};

std::vector<std::string> SmallParser::_symbolicNames = {
  "", "", "", "", "", "", "NEWLINE", "SIMPLE_IDENTIFIER", "INT"
};

dfa::Vocabulary SmallParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> SmallParser::_tokenNames;

SmallParser::Initializer::Initializer() {
	for (size_t i = 0; i < _symbolicNames.size(); ++i) {
		std::string name = _vocabulary.getLiteralName(i);
		if (name.empty()) {
			name = _vocabulary.getSymbolicName(i);
		}

		if (name.empty()) {
			_tokenNames.push_back("<INVALID>");
		} else {
      _tokenNames.push_back(name);
    }
	}

  _serializedATN = {
    0x3, 0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 
    0x3, 0xa, 0x30, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 0x4, 0x9, 
    0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x3, 0x2, 0x3, 0x2, 0x3, 
    0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x5, 0x3, 0x14, 0xa, 0x3, 
    0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x5, 0x4, 0x19, 0xa, 0x4, 0x3, 0x4, 0x3, 
    0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x5, 0x4, 0x22, 
    0xa, 0x4, 0x5, 0x4, 0x24, 0xa, 0x4, 0x3, 0x5, 0x3, 0x5, 0x3, 0x5, 0x7, 
    0x5, 0x29, 0xa, 0x5, 0xc, 0x5, 0xe, 0x5, 0x2c, 0xb, 0x5, 0x3, 0x6, 0x3, 
    0x6, 0x3, 0x6, 0x2, 0x2, 0x7, 0x2, 0x4, 0x6, 0x8, 0xa, 0x2, 0x2, 0x2, 
    0x2f, 0x2, 0xc, 0x3, 0x2, 0x2, 0x2, 0x4, 0x13, 0x3, 0x2, 0x2, 0x2, 0x6, 
    0x23, 0x3, 0x2, 0x2, 0x2, 0x8, 0x25, 0x3, 0x2, 0x2, 0x2, 0xa, 0x2d, 
    0x3, 0x2, 0x2, 0x2, 0xc, 0xd, 0x5, 0x4, 0x3, 0x2, 0xd, 0xe, 0x7, 0x2, 
    0x2, 0x3, 0xe, 0x3, 0x3, 0x2, 0x2, 0x2, 0xf, 0x10, 0x5, 0x6, 0x4, 0x2, 
    0x10, 0x11, 0x5, 0x4, 0x3, 0x2, 0x11, 0x14, 0x3, 0x2, 0x2, 0x2, 0x12, 
    0x14, 0x7, 0x8, 0x2, 0x2, 0x13, 0xf, 0x3, 0x2, 0x2, 0x2, 0x13, 0x12, 
    0x3, 0x2, 0x2, 0x2, 0x14, 0x5, 0x3, 0x2, 0x2, 0x2, 0x15, 0x18, 0x7, 
    0x9, 0x2, 0x2, 0x16, 0x17, 0x7, 0x3, 0x2, 0x2, 0x17, 0x19, 0x5, 0xa, 
    0x6, 0x2, 0x18, 0x16, 0x3, 0x2, 0x2, 0x2, 0x18, 0x19, 0x3, 0x2, 0x2, 
    0x2, 0x19, 0x24, 0x3, 0x2, 0x2, 0x2, 0x1a, 0x1b, 0x7, 0x4, 0x2, 0x2, 
    0x1b, 0x21, 0x7, 0x9, 0x2, 0x2, 0x1c, 0x1d, 0x7, 0x3, 0x2, 0x2, 0x1d, 
    0x1e, 0x7, 0x5, 0x2, 0x2, 0x1e, 0x1f, 0x5, 0x8, 0x5, 0x2, 0x1f, 0x20, 
    0x7, 0x6, 0x2, 0x2, 0x20, 0x22, 0x3, 0x2, 0x2, 0x2, 0x21, 0x1c, 0x3, 
    0x2, 0x2, 0x2, 0x21, 0x22, 0x3, 0x2, 0x2, 0x2, 0x22, 0x24, 0x3, 0x2, 
    0x2, 0x2, 0x23, 0x15, 0x3, 0x2, 0x2, 0x2, 0x23, 0x1a, 0x3, 0x2, 0x2, 
    0x2, 0x24, 0x7, 0x3, 0x2, 0x2, 0x2, 0x25, 0x2a, 0x7, 0xa, 0x2, 0x2, 
    0x26, 0x27, 0x7, 0x7, 0x2, 0x2, 0x27, 0x29, 0x7, 0xa, 0x2, 0x2, 0x28, 
    0x26, 0x3, 0x2, 0x2, 0x2, 0x29, 0x2c, 0x3, 0x2, 0x2, 0x2, 0x2a, 0x28, 
    0x3, 0x2, 0x2, 0x2, 0x2a, 0x2b, 0x3, 0x2, 0x2, 0x2, 0x2b, 0x9, 0x3, 
    0x2, 0x2, 0x2, 0x2c, 0x2a, 0x3, 0x2, 0x2, 0x2, 0x2d, 0x2e, 0x7, 0xa, 
    0x2, 0x2, 0x2e, 0xb, 0x3, 0x2, 0x2, 0x2, 0x7, 0x13, 0x18, 0x21, 0x23, 
    0x2a, 
  };

  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

SmallParser::Initializer SmallParser::_init;
