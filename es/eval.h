#ifndef ES_EVALUATOR_H
#define ES_EVALUATOR_H

#include <math.h>

#include <es/parser/character.h>
#include <es/parser/ast.h>
#include <es/types/completion.h>
#include <es/types/reference.h>
#include <es/types/builtin/function_object.h>
#include <es/types/builtin/array_object.h>
#include <es/types/builtin/regexp_object.h>
#include <es/types/builtin/error_object.h>
#include <es/types/compare.h>
#include <es/runtime.h>
#include <es/utils/helper.h>

namespace es {

Completion EvalProgram(AST* ast);

Completion EvalStatement(AST* ast);
Completion EvalStatementList(std::vector<AST*> statements);
Completion EvalBlockStatement(AST* ast);
Handle<String> EvalVarDecl(Handle<Error>& e, AST* ast);
Completion EvalVarStatement(AST* ast);
Completion EvalIfStatement(AST* ast);
Completion EvalForStatement(AST* ast);
Completion EvalForInStatement(AST* ast);
Completion EvalDoWhileStatement(AST* ast);
Completion EvalWhileStatement(AST* ast);
Completion EvalContinueStatement(AST* ast);
Completion EvalBreakStatement(AST* ast);
Completion EvalReturnStatement(AST* ast);
Completion EvalLabelledStatement(AST* ast);
Completion EvalWithStatement(AST* ast);
Completion EvalSwitchStatement(AST* ast);
Completion EvalThrowStatement(AST* ast);
Completion EvalTryStatement(AST* ast);
Completion EvalExpressionStatement(AST* ast);

Handle<JSValue> EvalExpression(Handle<Error>& e, AST* ast);
Handle<JSValue> EvalPrimaryExpression(Handle<Error>& e, AST* ast);
Handle<JSValue> EvalExpressionAndGetValue(Handle<Error>& e, AST* ast);
void EvalExpressionAndPutValue(Handle<Error>& e, AST* ast, Handle<JSValue> val);
Handle<Reference> EvalIdentifier(AST* ast);
Handle<JSValue> EvalIdentifierAndGetValue(Handle<Error>& e, AST* ast);
void EvalIdentifierAndPutValue(Handle<Error>& e, AST* ast, Handle<JSValue> val);
Handle<Number> EvalNumber(AST* ast);
Handle<String> EvalString(Handle<Error>& e, AST* ast);
Handle<Object> EvalObject(Handle<Error>& e, AST* ast);
Handle<ArrayObject> EvalArray(Handle<Error>& e, AST* ast);
Handle<JSValue> EvalUnaryOperator(Handle<Error>& e, AST* ast);
Handle<JSValue> EvalBinaryExpression(Handle<Error>& e, AST* ast);
Handle<JSValue> EvalBinaryExpression(Handle<Error>& e, Token& op, AST* lval, AST* rval);
Handle<JSValue> EvalBinaryExpression(Handle<Error>& e, Token& op, Handle<JSValue> lval, Handle<JSValue> rval);
Handle<JSValue> EvalArithmeticOperator(Handle<Error>& e, Token& op, Handle<JSValue> lval, Handle<JSValue> rval);
Handle<JSValue> EvalAddOperator(Handle<Error>& e, Handle<JSValue> lval, Handle<JSValue> rval);
Handle<JSValue> EvalBitwiseShiftOperator(Handle<Error>& e, Token& op, Handle<JSValue> lval, Handle<JSValue> rval);
Handle<JSValue> EvalRelationalOperator(Handle<Error>& e, Token& op, Handle<JSValue> lval, Handle<JSValue> rval);
Handle<JSValue> EvalEqualityOperator(Handle<Error>& e, Token& op, Handle<JSValue> lval, Handle<JSValue> rval);
Handle<JSValue> EvalBitwiseOperator(Handle<Error>& e, Token& op, Handle<JSValue> lval, Handle<JSValue> rval);
Handle<JSValue> EvalLogicalOperator(Handle<Error>& e, Token& op, AST* lhs, AST* rhs);
Handle<JSValue> EvalSimpleAssignment(Handle<Error>& e, Handle<JSValue> lref, Handle<JSValue> rval);
Handle<JSValue> EvalCompoundAssignment(Handle<Error>& e, Token& op, Handle<JSValue> lref, Handle<JSValue> rval);
Handle<JSValue> EvalTripleConditionExpression(Handle<Error>& e, AST* ast);
Handle<JSValue> EvalAssignmentExpression(Handle<Error>& e, AST* ast);
Handle<JSValue> EvalLeftHandSideExpression(Handle<Error>& e, AST* ast);
std::vector<Handle<JSValue>> EvalArgumentsList(Handle<Error>& e, Arguments* ast);
Handle<JSValue> EvalCallExpression(Handle<Error>& e, Handle<JSValue> ref, std::vector<Handle<JSValue>> arg_list);
Handle<Reference> EvalIndexExpression(Handle<Error>& e, Handle<JSValue> base_ref, Handle<String> identifier_name, ValueGuard& guard);
Handle<JSValue> EvalIndexExpression(Handle<Error>& e, Handle<JSValue> base_ref, AST* expr, ValueGuard& guard);
Handle<JSValue> EvalExpressionList(Handle<Error>& e, AST* ast);

void IdentifierResolutionAndPutValue(Handle<Error>& e, Handle<String> name, Handle<JSValue> value);

Completion EvalProgram(AST* ast) {
  ASSERT(ast->type() == AST::AST_PROGRAM || ast->type() == AST::AST_FUNC_BODY);
  auto prog = static_cast<ProgramOrFunctionBody*>(ast);
  auto statements = prog->statements();
  // 12.9 considered syntactically incorrect if it contains
  //      a return statement that is not within a FunctionBody.
  if (ast->type() != AST::AST_FUNC_BODY) {
    for (auto stmt : statements) {
      if (stmt->type() == AST::AST_STMT_RETURN) {
        return Completion(
          Completion::THROW,
          Error::SyntaxError(u"return statement must exist in return statement."),
          u"");
      }
    }
  }

  Completion head_result;
  if (statements.size() == 0)
    return Completion(Completion::NORMAL, Handle<JSValue>(), u"");
  for (auto stmt : prog->statements()) {
    if (head_result.IsAbruptCompletion())
      break;
    Completion tail_result = EvalStatement(stmt);
    if (tail_result.IsThrow())
      return tail_result;
    head_result = Completion(
      tail_result.type(),
      tail_result.IsEmpty() ? head_result.value() : tail_result.value(),
      tail_result.target()
    );
  }
  return head_result;
}

Completion EvalStatement(AST* ast) {
  TEST_LOG("\033[1;33mEvalStatement\033[0m\n", ast->source(), "\n");
  Completion C(Completion::NORMAL, Handle<JSValue>(), u"");
  JSValue* val = nullptr;
  {
    HandleScope scope;
    switch(ast->type()) {
      case AST::AST_STMT_BLOCK:
        C = EvalBlockStatement(ast);
        break;
      case AST::AST_STMT_VAR:
        C = EvalVarStatement(ast);
        break;
      case AST::AST_STMT_EMPTY:
        break;
      case AST::AST_STMT_IF:
        C = EvalIfStatement(ast);
        break;
      case AST::AST_STMT_DO_WHILE:
        C = EvalDoWhileStatement(ast);
        break;
      case AST::AST_STMT_WHILE:
        C = EvalWhileStatement(ast);
        break;
      case AST::AST_STMT_FOR:
        C = EvalForStatement(ast);
        break;
      case AST::AST_STMT_FOR_IN:
        C = EvalForInStatement(ast);
        break;
      case AST::AST_STMT_CONTINUE:
        C = EvalContinueStatement(ast);
        break;
      case AST::AST_STMT_BREAK:
        C = EvalBreakStatement(ast);
        break;
      case AST::AST_STMT_RETURN:
        C = EvalReturnStatement(ast);
        break;
      case AST::AST_STMT_WITH:
        C = EvalWithStatement(ast);
        break;
      case AST::AST_STMT_LABEL:
        C = EvalLabelledStatement(ast);
        break;
      case AST::AST_STMT_SWITCH:
        C = EvalSwitchStatement(ast);
        break;
      case AST::AST_STMT_THROW:
        C = EvalThrowStatement(ast);
        break;
      case AST::AST_STMT_TRY:
        C = EvalTryStatement(ast);
        break;
      case AST::AST_STMT_DEBUG:
        log::Debugger::Turn();
        break;
      default:
        C = EvalExpressionStatement(ast);
        break;
    }
    // Need to bring the value of C out of the current HandleScope
    if (!C.IsEmpty()) {
      val = C.value().val();
    }
  }  // end of HandleScope
  C.SetValue(val);
  return C;
}

Completion EvalStatementList(std::vector<AST*> statements) {
  Completion sl;
  for (auto stmt : statements) {
    Completion s = EvalStatement(stmt);
    if (s.IsThrow())
      return s;
    sl = Completion(s.type(), s.IsEmpty() ? sl.value() : s.value(), s.target());
    if (sl.IsAbruptCompletion())
      return sl;
  }
  return sl;
}

Completion EvalBlockStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_BLOCK);
  Block* block = static_cast<Block*>(ast);
  return EvalStatementList(block->statements());
}

Handle<String> EvalVarDecl(Handle<Error>& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_VAR_DECL);
  VarDecl* decl = static_cast<VarDecl*>(ast);
  Handle<String> ident = decl->ident();
  if (decl->init() == nullptr)
    return ident;
  Handle<JSValue> rhs = EvalAssignmentExpression(e, decl->init());
  if (unlikely(!e.val()->IsOk())) return ident;
  Handle<JSValue> value = GetValue(e, rhs);
  if (unlikely(!e.val()->IsOk())) return ident;
  IdentifierResolutionAndPutValue(e, ident, value);
  if (unlikely(!e.val()->IsOk())) return ident;
  return ident;
}

Completion EvalVarStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_VAR);
  Handle<Error> e = Error::Ok();
  VarStmt* var_stmt = static_cast<VarStmt*>(ast);
  for (VarDecl* decl : var_stmt->decls()) {
    if (decl->init() == nullptr)
      continue;
    EvalVarDecl(e, decl);
    if (unlikely(!e.val()->IsOk())) goto error;
  }
  return Completion(Completion::NORMAL, Handle<JSValue>(), u"");
error:
  return Completion(Completion::THROW, e, u"");
}

Completion EvalIfStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_IF);
  Handle<Error> e = Error::Ok();
  If* if_stmt = static_cast<If*>(ast);
  Handle<JSValue> expr = EvalExpressionAndGetValue(e, if_stmt->cond());
  if (unlikely(!e.val()->IsOk()))
    return Completion(Completion::THROW, e, u"");
  if (ToBoolean(expr)) {
    return EvalStatement(if_stmt->if_block());
  } else if (if_stmt->else_block() != nullptr){
    return EvalStatement(if_stmt->else_block());
  }
  return Completion(Completion::NORMAL, Handle<JSValue>(), u"");
}

// 12.6.1 The do-while Statement
Completion EvalDoWhileStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_DO_WHILE);
  Handle<Error> e = Error::Ok();
  Runtime::TopContext().EnterIteration();
  DoWhile* loop_stmt = static_cast<DoWhile*>(ast);
  // Handle<JSValue> V;  // V is substitued by stmt.value()
  Handle<JSValue> expr_ref;
  Handle<JSValue> val;
  Completion stmt;
  while (true) {
    stmt = EvalStatement(loop_stmt->stmt());
    switch (stmt.type()) {
      case Completion::BREAK: {
        if (stmt.target() == ast->label() || stmt.target() == u"") {
          Runtime::TopContext().ExitIteration();
          return Completion(Completion::NORMAL, stmt.value(), u"");
        }
        [[fallthrough]];
      }
      case Completion::RETURN:
      case Completion::THROW: {
        Runtime::TopContext().ExitIteration();
        return stmt;
      }
      case Completion::CONTINUE: {
        if (stmt.target() != u"" && stmt.target() != ast->label()) {
          Runtime::TopContext().ExitIteration();
          return stmt;
        }
        [[fallthrough]];
      }
      default: {  // normal
        val = EvalExpressionAndGetValue(e, loop_stmt->expr());
        if (unlikely(!e.val()->IsOk())) goto error;
      }
    }
    if (!ToBoolean(val))
      break;
  }
  Runtime::TopContext().ExitIteration();
  return Completion(Completion::NORMAL, stmt.value(), u"");
error:
  Runtime::TopContext().ExitIteration();
  return Completion(Completion::THROW, e, u"");
}

// 12.6.2 The while Statement
Completion EvalWhileStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_WHILE);
  Handle<Error> e = Error::Ok();
  Runtime::TopContext().EnterIteration();
  WhileOrWith* loop_stmt = static_cast<WhileOrWith*>(ast);
  // Handle<JSValue> V;  // V is substitued by stmt.value()
  Handle<JSValue> expr_ref;
  Handle<JSValue> val;
  Completion stmt;
  while (true) {
    val = EvalExpressionAndGetValue(e, loop_stmt->expr());
    if (unlikely(!e.val()->IsOk())) goto error;
    if (!ToBoolean(val))
      break;

    stmt = EvalStatement(loop_stmt->stmt());
    switch (stmt.type()) {
      case Completion::BREAK: {
        if (stmt.target() == ast->label() || stmt.target() == u"") {
          Runtime::TopContext().ExitIteration();
          return Completion(Completion::NORMAL, stmt.value(), u"");
        }
        [[fallthrough]];
      }
      case Completion::RETURN:
      case Completion::THROW: {
        Runtime::TopContext().ExitIteration();
        return stmt;
      }
      case Completion::CONTINUE: {
        if (stmt.target() != u"" && stmt.target() != ast->label()) {
          Runtime::TopContext().ExitIteration();
          return stmt;
        }
        [[fallthrough]];
      }
      default: {  // normal
      }
    }
  }
  Runtime::TopContext().ExitIteration();
  return Completion(Completion::NORMAL, stmt.value(), u"");
error:
  Runtime::TopContext().ExitIteration();
  return Completion(Completion::THROW, e, u"");
}

// 12.6.3 The for Statement
Completion EvalForStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_FOR);
  Handle<Error> e = Error::Ok();
  Runtime::TopContext().EnterIteration();
  For* for_stmt = static_cast<For*>(ast);
  // Handle<JSValue> V;  // V is substitued by stmt.value()
  Completion stmt;
  for (auto expr : for_stmt->expr0s()) {
    if (expr->type() == AST::AST_STMT_VAR_DECL) {
      EvalVarDecl(e, expr);
      if (unlikely(!e.val()->IsOk())) goto error;
    } else {
      EvalExpressionAndGetValue(e, expr);
      if (unlikely(!e.val()->IsOk())) goto error;
    }
  }
  while (true) {
    if (for_stmt->expr1() != nullptr) {
      Handle<JSValue> test_value = EvalExpressionAndGetValue(e, for_stmt->expr1());
      if (unlikely(!e.val()->IsOk())) goto error;
      if (!ToBoolean(test_value))
        break;
    }

    stmt = EvalStatement(for_stmt->statement());
    switch (stmt.type()) {
      case Completion::BREAK: {
        if (stmt.target() == ast->label() || stmt.target() == u"") {
          Runtime::TopContext().ExitIteration();
          return Completion(Completion::NORMAL, stmt.value(), u"");
        }
        [[fallthrough]];
      }
      case Completion::RETURN:
      case Completion::THROW: {
        Runtime::TopContext().ExitIteration();
        return stmt;
      }
      case Completion::CONTINUE: {
        if (stmt.target() != u"" && stmt.target() != ast->label()) {
          Runtime::TopContext().ExitIteration();
          return stmt;
        }
        [[fallthrough]];
      }
      default: {  // normal
      }
    }

    if (for_stmt->expr2() != nullptr) {
      EvalExpressionAndGetValue(e, for_stmt->expr2());
      if (unlikely(!e.val()->IsOk())) goto error;
    }
  }
  Runtime::TopContext().ExitIteration();
  return Completion(Completion::NORMAL, stmt.value(), u"");
error:
  Runtime::TopContext().ExitIteration();
  return Completion(Completion::THROW, e, u"");
}

// 12.6.4 The for-in Statement
Completion EvalForInStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_FOR_IN);
  Handle<Error> e = Error::Ok();
  Runtime::TopContext().EnterIteration();
  ForIn* for_in_stmt = static_cast<ForIn*>(ast);
  Handle<JSObject> obj;
  Handle<JSValue> expr_ref;
  Handle<JSValue> expr_val;
  Completion stmt;
  bool has_label;
  Handle<JSValue> V;
  if (for_in_stmt->expr0()->type() == AST::AST_STMT_VAR_DECL) {
    VarDecl* decl = static_cast<VarDecl*>(for_in_stmt->expr0());
    Handle<String> var_name = EvalVarDecl(e, decl);
    if (unlikely(!e.val()->IsOk())) goto error;
    expr_val = EvalExpressionAndGetValue(e, for_in_stmt->expr1());
    if (unlikely(!e.val()->IsOk())) goto error;
    if (expr_val.val()->IsUndefined() || expr_val.val()->IsNull()) {
      Runtime::TopContext().ExitIteration();
      return Completion(Completion::NORMAL, Handle<JSValue>(), u"");
    }
    obj = ToObject(e, expr_val);
    if (unlikely(!e.val()->IsOk())) goto error;

    for (auto pair : obj.val()->AllEnumerableProperties()) {
      Handle<String> P = pair.first;
      IdentifierResolutionAndPutValue(e, var_name, P);
      if (unlikely(!e.val()->IsOk())) goto error;

      stmt = EvalStatement(for_in_stmt->statement());
      if (!stmt.IsEmpty())
        V = stmt.value();
      has_label = stmt.target() == ast->label() || stmt.target() == u"";
      if (stmt.type() != Completion::CONTINUE || !has_label) {
        if (stmt.type() == Completion::BREAK && has_label) {
          Runtime::TopContext().ExitIteration();
          return Completion(Completion::NORMAL, V, u"");
        }
        if (stmt.IsAbruptCompletion()) {
          Runtime::TopContext().ExitIteration();
          return stmt;
        }
      }
    }
  } else {
    expr_val = EvalExpressionAndGetValue(e, for_in_stmt->expr1());
    if (unlikely(!e.val()->IsOk())) goto error;
    if (expr_val.val()->IsUndefined() || expr_val.val()->IsNull()) {
      Runtime::TopContext().ExitIteration();
      return Completion(Completion::NORMAL, Handle<JSValue>(), u"");
    }
    obj = ToObject(e, expr_val);
    for (auto pair : obj.val()->AllEnumerableProperties()) {
      Handle<String> P = pair.first;
      EvalExpressionAndPutValue(e, for_in_stmt->expr0(), P);
      if (unlikely(!e.val()->IsOk())) goto error;

      stmt = EvalStatement(for_in_stmt->statement());
      if (!stmt.IsEmpty())
        V = stmt.value();
      has_label = stmt.target() == ast->label() || stmt.target() == u"";
      if (stmt.type() != Completion::CONTINUE || !has_label) {
        if (stmt.type() == Completion::BREAK && has_label) {
          Runtime::TopContext().ExitIteration();
          return Completion(Completion::NORMAL, V, u"");
        }
        if (stmt.IsAbruptCompletion()) {
          Runtime::TopContext().ExitIteration();
          return stmt;
        }
      }
    }
  }
  Runtime::TopContext().ExitIteration();
  return Completion(Completion::NORMAL, V, u"");
error:
  Runtime::TopContext().ExitIteration();
  return Completion(Completion::THROW, e, u"");
}

Completion EvalContinueStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_CONTINUE);
  Handle<Error> e = Error::Ok();
  if (!Runtime::TopContext().InIteration()) {
    e = Error::SyntaxError(u"continue not in iteration");
    return Completion(Completion::THROW, e, u"");
  }
  ContinueOrBreak* stmt = static_cast<ContinueOrBreak*>(ast);
  return Completion(Completion::CONTINUE, Handle<JSValue>(), stmt->ident());
}

Completion EvalBreakStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_BREAK);
  Handle<Error> e = Error::Ok();
  if (!Runtime::TopContext().InIteration() && !Runtime::TopContext().InSwitch()) {
    e = Error::SyntaxError(u"break not in iteration or switch");
    return Completion(Completion::THROW, e, u"");
  }
  ContinueOrBreak* stmt = static_cast<ContinueOrBreak*>(ast);
  return Completion(Completion::BREAK, Handle<JSValue>(), stmt->ident());
}

Completion EvalReturnStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_RETURN);
  Handle<Error> e = Error::Ok();
  Return* return_stmt = static_cast<Return*>(ast);
  if (return_stmt->expr() == nullptr) {
    return Completion(Completion::RETURN, Undefined::Instance(), u"");
  }
  Handle<JSValue> exp = EvalExpressionAndGetValue(e, return_stmt->expr());
  if (unlikely(!e.val()->IsOk())) {
    return Completion(Completion::THROW, e, u"");
  }
  return Completion(Completion::RETURN, exp, u"");
}

Completion EvalLabelledStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_LABEL);
  LabelledStmt* label_stmt = static_cast<LabelledStmt*>(ast);
  label_stmt->statement()->SetLabel(label_stmt->label());
  Completion R = EvalStatement(label_stmt->statement());
  if (R.type() == Completion::BREAK && R.target() == label_stmt->label()) {
    return Completion(Completion::NORMAL, R.value(), u"");
  }
  return R;
}

// 12.10 The with Statement
Completion EvalWithStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_WITH);
  if (Runtime::TopContext().strict()) {
    return Completion(
      Completion::THROW,
      Error::SyntaxError(u"cannot have with statement in strict mode"),
      u"");
  }
  Handle<Error> e = Error::Ok();
  WhileOrWith* with_stmt = static_cast<WhileOrWith*>(ast);
  Handle<JSValue> ref = EvalExpression(e, with_stmt->expr());
  if (unlikely(!e.val()->IsOk()))
    return Completion(Completion::THROW, e, u"");
  Handle<JSValue> val = GetValue(e, ref);
  if (unlikely(!e.val()->IsOk()))
    return Completion(Completion::THROW, e, u"");
  Handle<JSObject> obj = ToObject(e, val);
  if (unlikely(!e.val()->IsOk()))
    return Completion(Completion::THROW, e, u"");
  // Prevent garbage collect old env.
  Handle<EnvironmentRecord> old_env = Runtime::TopLexicalEnv();
  Handle<EnvironmentRecord> new_env = NewObjectEnvironment(obj, old_env, true);
  Runtime::TopContext().SetLexicalEnv(new_env);
  Completion C = EvalStatement(with_stmt->stmt());
  Runtime::TopContext().SetLexicalEnv(old_env);
  return C;
}

Completion EvalCaseBlock(Switch* switch_stmt, Handle<JSValue> input) {
  Handle<Error> e = Error::Ok();
  Handle<JSValue> V;
  bool found = false;
  for (auto C : switch_stmt->before_default_case_clauses()) {
    if (!found) {  // 5.a
      // EvalCaseClause
      Handle<JSValue> clause_selector = EvalExpressionAndGetValue(e, C.expr);
      bool b = StrictEqual(e, input, clause_selector);
      if (unlikely(!e.val()->IsOk()))
        return Completion(Completion::THROW, e, u"");
      if (b)
        found = true;
    }
    if (found) {  // 5.b
      Completion R = EvalStatementList(C.stmts);
      if (!R.IsEmpty())
        V = R.value();
      if (R.IsAbruptCompletion())
        return Completion(R.type(), V, R.target());
    }
  }
  bool found_in_b = false;
  size_t i;
  for (i = 0; !found_in_b && i < switch_stmt->after_default_case_clauses().size(); i++) {
    auto C = switch_stmt->after_default_case_clauses()[i];
    // EvalCaseClause
    Handle<JSValue> clause_selector = EvalExpressionAndGetValue(e, C.expr);
    bool b = StrictEqual(e, input, clause_selector);
    if (unlikely(!e.val()->IsOk()))
      return Completion(Completion::THROW, e, u"");
    if (b) {
      found_in_b = true;
      Completion R = EvalStatementList(C.stmts);
      if (!R.IsEmpty())
        V = R.value();
      if (R.IsAbruptCompletion())
        return Completion(R.type(), V, R.target());
    }
  }
  if (!found_in_b && switch_stmt->has_default_clause()) {  // 8
    Completion R = EvalStatementList(switch_stmt->default_clause().stmts);
    if (!R.IsEmpty())
      V = R.value();
    if (R.IsAbruptCompletion())
      return Completion(R.type(), V, R.target());
  }
  for (; i < switch_stmt->after_default_case_clauses().size(); i++) {
    auto C = switch_stmt->after_default_case_clauses()[i];
    EvalExpressionAndGetValue(e, C.expr);
    Completion R = EvalStatementList(C.stmts);
    if (!R.IsEmpty())
      V = R.value();
    if (R.IsAbruptCompletion())
      return Completion(R.type(), V, R.target());
  }
  return Completion(Completion::NORMAL, V, u"");
}

// 12.11 The switch Statement
Completion EvalSwitchStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_SWITCH);
  Handle<Error> e = Error::Ok();
  Switch* switch_stmt = static_cast<Switch*>(ast);
  Handle<JSValue> expr_val = EvalExpressionAndGetValue(e, switch_stmt->expr());
  if (unlikely(!e.val()->IsOk()))
    return Completion(Completion::THROW, e, u"");
  Runtime::TopContext().EnterSwitch();
  Completion R = EvalCaseBlock(switch_stmt, expr_val);
  Runtime::TopContext().ExitSwitch();
  if (R.IsThrow())
    return R;
  bool has_label = ast->label() == R.target();
  if (R.type() == Completion::BREAK && has_label)
    return Completion(Completion::NORMAL, R.value(), u"");
  return R;
}

// 12.13 The throw Statement
Completion EvalThrowStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_THROW);
  Handle<Error> e = Error::Ok();
  Throw* throw_stmt = static_cast<Throw*>(ast);
  Handle<JSValue> exp_ref = EvalExpression(e, throw_stmt->expr());
  if (unlikely(!e.val()->IsOk()))
    return Completion(Completion::THROW, e, u"");
  Handle<JSValue> val = GetValue(e, exp_ref);
  if (unlikely(!e.val()->IsOk()))
    return Completion(Completion::THROW, e, u"");
  return Completion(Completion::THROW, val, u"");
}

Completion EvalCatch(Try* try_stmt, Completion C) {
  Handle<Error> e = Error::Ok();
  // Prevent garbage collect old env.
  Handle<EnvironmentRecord> old_env = Runtime::TopLexicalEnv();
  Handle<EnvironmentRecord> catch_env = NewDeclarativeEnvironment(old_env, 0);
  // NOTE(zhuzilin) The spec say to send C instead of C.value.
  // However, I think it should be send C.value...
  Handle<JSValue> val;
  if (C.value().val()->IsError()) {
    Handle<Error> error = C.value();
    if (error.val()->IsNativeError()) {
      val = error.val()->value();
    } else {
      val = ErrorObject::New(error);
    }
  } else {
    val = C.value();
  }
  CreateAndSetMutableBinding(
    e, catch_env, try_stmt->catch_ident(), false, val, false);  // 4 & 5
  if (unlikely(!e.val()->IsOk())) {
    return Completion(Completion::THROW, e, u"");
  }
  Runtime::TopContext().SetLexicalEnv(catch_env);
  Completion B = EvalBlockStatement(try_stmt->catch_block());
  Runtime::TopContext().SetLexicalEnv(old_env);
  return B;
}

Completion EvalTryStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_TRY);
  Try* try_stmt = static_cast<Try*>(ast);
  if (Runtime::TopContext().strict()) {
    if (try_stmt->catch_ident_is_eval_or_arguments()) {
      Handle<Error> e = Error::SyntaxError(u"use eval or arguments as identifier of catch in strict mode");
      return Completion(Completion::THROW, e, u"");
    }
  }
  Completion B = EvalBlockStatement(try_stmt->try_block());
  if (try_stmt->finally_block() == nullptr) {  // try Block Catch
    if (B.type() != Completion::THROW)
      return B;
    return EvalCatch(try_stmt, B);
  } else if (try_stmt->catch_block() == nullptr) {  // try Block Finally
    Completion F = EvalBlockStatement(try_stmt->finally_block());
    if (F.type() == Completion::NORMAL)
      return B;
    return F;
  } else {  // try Block Catch Finally
    Completion C = B;
    if (B.type() == Completion::THROW) {
      C = EvalCatch(try_stmt, B);
    }
    Completion F = EvalBlockStatement(try_stmt->finally_block());
    if (F.type() == Completion::NORMAL)
      return C;
    return F;
  }
}

Completion EvalExpressionStatement(AST* ast) {
  Handle<Error> e = Error::Ok();
  Handle<JSValue> val = EvalExpressionAndGetValue(e, ast);
  if (unlikely(!e.val()->IsOk()))
    return Completion(Completion::THROW, e, u"");
  return Completion(Completion::NORMAL, val, u"");
}

Handle<JSValue> EvalExpressionAndGetValue(Handle<Error>& e, AST* ast) {
  while (ast->type() == AST::AST_EXPR_LHS) {
    LHS* lhs = static_cast<LHS*>(ast);
    if (lhs->total_count() != 0)
      break;
    ast = lhs->base();
  }
  switch (ast->type()) {
    case AST::AST_EXPR_STRICT_FUTURE:
      if (Runtime::TopContext().strict()) {
        e = Error::SyntaxError(u"future reserved word " + ast->source() + u" used");
        return Handle<JSValue>();
      }
      [[fallthrough]];
    case AST::AST_EXPR_IDENT: {
      return EvalIdentifierAndGetValue(e, ast);
    }
    default: {
      Handle<JSValue> ref = EvalExpression(e, ast);
      if (unlikely(!e.val()->IsOk()))
        return Handle<JSValue>();
      Handle<JSValue> val = GetValue(e, ref);
      return val;
    }
  }
}

void EvalExpressionAndPutValue(Handle<Error>& e, AST* ast, Handle<JSValue> val) {
  while (ast->type() == AST::AST_EXPR_LHS) {
    LHS* lhs = static_cast<LHS*>(ast);
    if (lhs->total_count() != 0)
      break;
    ast = lhs->base();
  }
  switch (ast->type()) {
    case AST::AST_EXPR_STRICT_FUTURE:
      if (Runtime::TopContext().strict()) {
        e = Error::SyntaxError(u"future reserved word " + ast->source() + u" used");
        return;
      }
      [[fallthrough]];
    case AST::AST_EXPR_IDENT: {
      return EvalIdentifierAndPutValue(e, ast, val);
    }
    default: {
      Handle<JSValue> ref = EvalExpression(e, ast);
      if (unlikely(!e.val()->IsOk()))
        return;
      PutValue(e, ref, val);
    }
  }
}

Handle<JSValue> EvalExpression(Handle<Error>& e, AST* ast) {
  ASSERT(ast->type() <= AST::AST_EXPR || ast->type() == AST::AST_FUNC);
  Handle<JSValue> val;
  switch (ast->type()) {
    case AST::AST_EXPR_STRICT_FUTURE:
      if (Runtime::TopContext().strict()) {
        e = Error::SyntaxError(u"future reserved word " + ast->source() + u" used");
        return Handle<JSValue>();
      }
      [[fallthrough]];
    case AST::AST_EXPR_THIS:
    case AST::AST_EXPR_IDENT:
    case AST::AST_EXPR_NULL:
    case AST::AST_EXPR_BOOL:
    case AST::AST_EXPR_NUMBER:
    case AST::AST_EXPR_STRING:
    case AST::AST_EXPR_REGEXP:
    case AST::AST_EXPR_OBJ:
    case AST::AST_EXPR_ARRAY:
    case AST::AST_EXPR_PAREN:
      val = EvalPrimaryExpression(e, ast);
      break;
    case AST::AST_EXPR_UNARY:
      val = EvalUnaryOperator(e, ast);
      break;
    case AST::AST_EXPR_BINARY:
      val = EvalBinaryExpression(e, ast);
      break;
    case AST::AST_EXPR_TRIPLE:
      val = EvalTripleConditionExpression(e, ast);
      break;
    case AST::AST_EXPR_LHS:
      val = EvalLeftHandSideExpression(e, ast);
      break;
    case AST::AST_EXPR:
      val = EvalExpressionList(e, ast);
      break;
    case AST::AST_FUNC:
      val = EvalFunction(e, ast);
      break;
    default:
      if (unlikely(log::Debugger::On()))
        log::PrintSource("ast: ", ast->source(), "type: " + std::to_string(ast->type()));
      assert(false);
  }
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  return val;
}

Handle<JSValue> EvalPrimaryExpression(Handle<Error>& e, AST* ast) {
  Handle<JSValue> val;
  switch (ast->type()) {
    case AST::AST_EXPR_THIS:
      val = Runtime::TopContext().this_binding();
      break;
    case AST::AST_EXPR_STRICT_FUTURE:
    case AST::AST_EXPR_IDENT:
      val = EvalIdentifier(ast);
      break;
    case AST::AST_EXPR_NULL:
      val = Null::Instance();
      break;
    case AST::AST_EXPR_BOOL:
      val = ast->jsval();
      break;
    case AST::AST_EXPR_NUMBER:
      val = EvalNumber(ast);
      break;
    case AST::AST_EXPR_STRING:
      val = EvalString(e, ast);
      break;
    case AST::AST_EXPR_OBJ:
      val = EvalObject(e, ast);
      break;
    case AST::AST_EXPR_ARRAY:
      val = EvalArray(e, ast);
      break;
    case AST::AST_EXPR_PAREN:
      val = EvalExpression(e, static_cast<Paren*>(ast)->expr());
      break;
    case AST::AST_EXPR_REGEXP: {
      RegExpLiteral* literal = static_cast<RegExpLiteral*>(ast);
      val = RegExpObject::New(String::New(literal->pattern()), String::New(literal->flag()));
      break;
    }
    default:
      std::cout << "Not primary expression, type " << ast->type() << "\n";
      assert(false);
  }
  return val;
}

// This will prevent use from creating a new ref
void IdentifierResolutionAndPutValue(Handle<Error>& e, Handle<String> name, Handle<JSValue> value) {
  // 10.3.1 Identifier Resolution
  Handle<EnvironmentRecord> env = Runtime::TopLexicalEnv();
  bool strict = Runtime::TopContext().strict();
  GetIdentifierReferenceAndPutValue(e, env, name, strict, value);
}

Handle<Reference> EvalIdentifier(AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_IDENT || ast->type() == AST::AST_EXPR_STRICT_FUTURE);
  ASSERT(!ast->jsval().IsNullptr());
  // 10.3.1 Identifier Resolution
  Handle<EnvironmentRecord> env = Runtime::TopLexicalEnv();
  Handle<String> ref_name = ast->jsval();
  bool strict = Runtime::TopContext().strict();
  return GetIdentifierReference(env, ref_name, strict);
}

Handle<JSValue> EvalIdentifierAndGetValue(Handle<Error>& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_IDENT || ast->type() == AST::AST_EXPR_STRICT_FUTURE);
  ASSERT(!ast->jsval().IsNullptr());
  // 10.3.1 Identifier Resolution
  Handle<EnvironmentRecord> env = Runtime::TopLexicalEnv();
  Handle<String> ref_name = ast->jsval();
  bool strict = Runtime::TopContext().strict();
  return GetIdentifierReferenceAndGetValue(e, env, ref_name, strict);
}

void EvalIdentifierAndPutValue(Handle<Error>& e, AST* ast, Handle<JSValue> val) {
  ASSERT(ast->type() == AST::AST_EXPR_IDENT || ast->type() == AST::AST_EXPR_STRICT_FUTURE);
  ASSERT(!ast->jsval().IsNullptr());
  // 10.3.1 Identifier Resolution
  Handle<EnvironmentRecord> env = Runtime::TopLexicalEnv();
  Handle<String> ref_name = ast->jsval();
  bool strict = Runtime::TopContext().strict();
  return GetIdentifierReferenceAndPutValue(e, env, ref_name, strict, val);
}

Handle<Number> EvalNumber(AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_NUMBER);
  ASSERT(!ast->jsval().IsNullptr());
  ASSERT(ast->jsval().val()->IsNumber());
  return ast->jsval();
}

Handle<String> EvalString(Handle<Error>& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_STRING);
  ASSERT(!ast->jsval().IsNullptr());
  ASSERT(ast->jsval().val()->IsString() || ast->jsval().val()->IsError());
  if (ast->jsval().val()->IsError()) {
    e = ast->jsval();
    return Handle<String>();
  }
  Handle<String> s = ast->jsval();
  return s;
}

Handle<Object> EvalObject(Handle<Error>& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_OBJ);
  ObjectLiteral* obj_ast = static_cast<ObjectLiteral*>(ast);
  bool strict = Runtime::TopContext().strict();
  Handle<Object> obj = Object::New(obj_ast->properties().size());
  // PropertyName : AssignmentExpression
  for (auto property : obj_ast->properties()) {
    StackPropertyDescriptor desc;
    switch (property.type) {
      case ObjectLiteral::Property::NORMAL: {
        Handle<JSValue> expr_value = EvalAssignmentExpression(e, property.value);
        if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
        Handle<JSValue> prop_value = GetValue(e, expr_value);
        if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
        desc = StackPropertyDescriptor::NewDataDescriptor(prop_value, true, true, true);
        break;
      }
      default: {
        ASSERT(property.value->type() == AST::AST_FUNC);
        Function* func_ast = static_cast<Function*>(property.value);
        bool strict_func = func_ast->body()->strict();
        if (strict || strict_func) {
          if (func_ast->params_have_eval_or_arguments()) {
            e = Error::SyntaxError(u"object cannot have getter or setter named eval or arguments");
            return Handle<JSValue>();
          }
        }
        Handle<FunctionObject> closure = FunctionObject::New(
          func_ast, Runtime::TopLexicalEnv(), strict || strict_func
        );
        if (property.type == ObjectLiteral::Property::GET) {
          desc.SetGet(closure);
        } else {
          desc.SetSet(closure);
        }
        desc.SetEnumerable(true);
        desc.SetConfigurable(true);
        break;
      }
    }
    Handle<JSValue> prop_name = property.key;
    if (prop_name.val()->IsError()) {
      e = prop_name;
      return Handle<JSValue>();
    }
    Handle<String> prop_name_str = prop_name;
    auto previous_desc = GetOwnProperty(obj, prop_name_str);  // 3
    if (!previous_desc.IsUndefined()) {  // 4
      if (strict &&
          previous_desc.IsDataDescriptor() && desc.IsDataDescriptor()) {  // 4.a
        e = Error::SyntaxError(u"repeat object property name " + prop_name_str.val()->data());
        return Handle<JSValue>();
      }
      if ((previous_desc.IsDataDescriptor() && desc.IsAccessorDescriptor()) ||  // 4.b
          (previous_desc.IsAccessorDescriptor() && desc.IsDataDescriptor())) {  // 4.c
        e = Error::SyntaxError(u"repeat object property name " + prop_name_str.val()->data());
        return Handle<JSValue>();
      }
      if (previous_desc.IsAccessorDescriptor() && desc.IsAccessorDescriptor() &&  // 4.d
          ((previous_desc.HasGet() && desc.HasGet()) ||
           (previous_desc.HasSet() && desc.HasSet()))) {
        e = Error::SyntaxError(u"repeat object property name " + prop_name_str.val()->data());
        return Handle<JSValue>();
      }
    }
    DefineOwnProperty(e, obj, prop_name_str, desc, false);
  }
  return obj;
}

Handle<ArrayObject> EvalArray(Handle<Error>& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_ARRAY);
  ArrayLiteral* array_ast = static_cast<ArrayLiteral*>(ast);

  Handle<ArrayObject> arr = ArrayObject::New(array_ast->length());
  for (auto pair : array_ast->elements()) {
    Handle<JSValue> init_result = EvalAssignmentExpression(e, pair.second);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    Handle<JSValue> init_value = GetValue(e, init_result);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    AddValueProperty(arr, NumberToString(pair.first), init_value, true, true, true);
  }
  return arr;
}

Handle<JSValue> EvalAssignmentExpression(Handle<Error>& e, AST* ast) {
  return EvalExpression(e, ast);
}

Handle<JSValue> EvalUnaryOperator(Handle<Error>& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_UNARY);
  Unary* u = static_cast<Unary*>(ast);

  Handle<JSValue> expr = EvalExpression(e, u->node());
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  Token op = u->op();

  switch (op.type()) {
    case Token::TK_INC:    // ++
    case Token::TK_DEC: {  // --
      if (expr.val()->IsReference()) {
        Handle<Reference> ref = static_cast<Handle<Reference>>(expr);
        auto stack_ref = Runtime::TopContext().GetReference(ref.val()->id());
        Handle<JSValue> base = stack_ref.base;
        Handle<String> ref_name = stack_ref.name;
        bool is_strict_ref = Runtime::TopContext().strict();
        if (is_strict_ref && base.val()->IsEnvironmentRecord() &&
            (StringEqual(ref_name, String::eval()) ||
             StringEqual(ref_name, String::arguments()))) {
          e = Error::SyntaxError(u"cannot inc or dec on eval or arguments");
          return Handle<JSValue>();
        }
      }
      Handle<JSValue> old_val = GetValue(e, expr);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      double num = ToNumber(e, old_val);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      Handle<JSValue> new_value;
      if (op.type() == Token::TK_INC) {
        new_value = Number::New(num + 1);
      } else {
        new_value = Number::New(num - 1);
      }
      PutValue(e, expr, new_value);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      if (u->prefix()) {
        return new_value;
      } else {
        // a = true; r = a++; r will be 1 instead of true.
        return Number::New(num);
      }
    }
    case Token::TK_ADD: {  // +
      Handle<JSValue> val = GetValue(e, expr);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      double num = ToNumber(e, val);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      return Number::New(num);
    }
    case Token::TK_SUB: {  // -
      Handle<JSValue> val = GetValue(e, expr);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      double num = ToNumber(e, val);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      if (isnan(num))
        return Number::NaN();
      return Number::New(-num);
    }
    case Token::TK_BIT_NOT: {  // ~
      Handle<JSValue> val = GetValue(e, expr);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      int32_t num = ToInt32(e, val);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      return Number::New(~num);
    }
    case Token::TK_LOGICAL_NOT: {  // !
      Handle<JSValue> val = GetValue(e, expr);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      bool b = ToBoolean(val);
      return Bool::Wrap(!b);
    }
    case Token::TK_KEYWORD_DELETE: {  // 11.4.1 The delete Operator
      if (!expr.val()->IsReference())  // 2
        return Bool::True();
      Handle<Reference> ref = static_cast<Handle<Reference>>(expr);
      auto stack_ref = Runtime::TopContext().GetReference(ref.val()->id());
      Handle<JSValue> base = stack_ref.base;
      Handle<String> ref_name = stack_ref.name;
      bool is_strict_ref = Runtime::TopContext().strict();
      if (Reference::IsUnresolvableReference(base)) {  // 3
        if (is_strict_ref) {
          e = Error::SyntaxError(u"delete not exist variable " + ref_name.val()->data());
          return Bool::False();
        }
        return Bool::True();
      }
      if (Reference::IsPropertyReference(base)) {  // 4
        Handle<JSObject> obj = ToObject(e, base);
        if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
        return Bool::Wrap(Delete(e, obj, ref_name, is_strict_ref));
      } else {
        if (is_strict_ref) {
          e = Error::SyntaxError(u"cannot delete environment record in strict mode");
          return Bool::False();
        }
        Handle<EnvironmentRecord> bindings = static_cast<Handle<EnvironmentRecord>>(base);
        return Bool::Wrap(DeleteBinding(e, bindings, ref_name));
      }
      assert(false);
    }
    case Token::TK_KEYWORD_TYPEOF: {
      if (expr.val()->IsReference()) {
        Handle<Reference> ref = static_cast<Handle<Reference>>(expr);
        auto stack_ref = Runtime::TopContext().GetReference(ref.val()->id());
        Handle<JSValue> base = stack_ref.base;
        if (Reference::IsUnresolvableReference(base))
          return String::undefined();
      }
      Handle<JSValue> val = GetValue(e, expr);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      switch (val.val()->type()) {
        case Type::JS_UNDEFINED:
          return String::undefined();
        case Type::JS_NULL:
          return String::object();
        case Type::JS_BOOL:
          return String::boolean();
        case Type::JS_NUMBER:
          return String::number();
        case Type::JS_LONG_STRING:
        case Type::JS_STRING:
          return String::string();
        default:
          if (val.val()->IsCallable())
            return String::function();
          return String::object();
      }
      assert(false);
    }
    case Token::TK_KEYWORD_VOID: {
      GetValue(e, expr);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      return Undefined::Instance();
      assert(false);
    }
    default:
      assert(false);
  }
}

Handle<JSValue> EvalBinaryExpression(Handle<Error>& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_BINARY);
  Binary* b = static_cast<Binary*>(ast);
  return EvalBinaryExpression(e, b->op(), b->lhs(), b->rhs());
}

Handle<JSValue> EvalBinaryExpression(Handle<Error>& e, Token& op, AST* lhs, AST* rhs) {
  // && and || are different, as there are not &&= and ||=
  if (op.IsBinaryLogical()) {
    return EvalLogicalOperator(e, op, lhs, rhs);
  }
  if (op.type() == Token::TK_ASSIGN || op.IsCompoundAssign()) {
    Handle<JSValue> lref = EvalLeftHandSideExpression(e, lhs);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    // TODO(zhuzilin) The compound assignment should do lval = GetValue(lref)
    // here. Check if changing the order will have any influence.
    Handle<JSValue> rval = EvalExpressionAndGetValue(e, rhs);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    if (op.type() == Token::TK_ASSIGN) {  // =
      return EvalSimpleAssignment(e, lref, rval);
    } else {
      return EvalCompoundAssignment(e, op, lref, rval);
    }
  }
  Handle<JSValue> lval = EvalExpressionAndGetValue(e, lhs);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  Handle<JSValue> rval = EvalExpressionAndGetValue(e, rhs);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  return EvalBinaryExpression(e, op, lval, rval);
}

Handle<JSValue> EvalBinaryExpression(Handle<Error>& e, Token& op, Handle<JSValue> lval, Handle<JSValue> rval) {
  switch (op.type()) {
    case Token::TK_ADD:  // +
      return EvalAddOperator(e, lval, rval);
    case Token::TK_SUB:  // -
    case Token::TK_MUL:  // *
    case Token::TK_DIV:  // /
    case Token::TK_MOD:  // %
      return EvalArithmeticOperator(e, op, lval, rval);
    case Token::TK_BIT_LSH:   // <<
    case Token::TK_BIT_RSH:   // >>
    case Token::TK_BIT_URSH:  // >>>, unsigned right shift
      return EvalBitwiseShiftOperator(e, op, lval, rval);
    case Token::TK_EQ:   // ==
    case Token::TK_NE:   // !=
    case Token::TK_EQ3:  // ===
    case Token::TK_NE3:  // !==
      return EvalEqualityOperator(e, op, lval, rval);
    case Token::TK_BIT_AND:  // &
    case Token::TK_BIT_OR:   // |
    case Token::TK_BIT_XOR:  // ^
      return EvalBitwiseOperator(e, op, lval, rval);
    case Token::TK_KEYWORD_INSTANCE_OF:
    case Token::TK_KEYWORD_IN:
    case Token::TK_LT:   // <
    case Token::TK_GT:   // >
    case Token::TK_LE:   // <=
    case Token::TK_GE:   // >=
      return EvalRelationalOperator(e, op, lval, rval);
    default:
      assert(false);
  }
}

// 11.5 Multiplicative Operators
Handle<JSValue> EvalArithmeticOperator(Handle<Error>& e, Token& op, Handle<JSValue> lval, Handle<JSValue> rval) {
  double lnum = ToNumber(e, lval);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  double rnum = ToNumber(e, rval);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  switch (op.type()) {
    case Token::TK_MUL:
      return Number::New(lnum * rnum);
    case Token::TK_DIV:
      return Number::New(lnum / rnum);
    case Token::TK_MOD:
      return Number::New(fmod(lnum, rnum));
    case Token::TK_SUB:
      return Number::New(lnum - rnum);
    default:
      assert(false);
  }
}

// 11.6 Additive Operators
Handle<JSValue> EvalAddOperator(Handle<Error>& e, Handle<JSValue> lval, Handle<JSValue> rval) {
  Handle<JSValue> lprim = ToPrimitive(e, lval);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  Handle<JSValue> rprim = ToPrimitive(e, rval);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();

  if (lprim.val()->IsString() || rprim.val()->IsString()) {
    Handle<String> lstr = ToString(e, lprim);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    Handle<String> rstr = ToString(e, rprim);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    return String::Concat(lstr, rstr);
  }

  double lnum = ToNumber(e, lprim);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  double rnum = ToNumber(e, rprim);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  return Number::New(lnum + rnum);
}

// 11.7 Bitwise Shift Operators
Handle<JSValue> EvalBitwiseShiftOperator(Handle<Error>& e, Token& op, Handle<JSValue> lval, Handle<JSValue> rval) {
  int32_t lnum = ToInt32(e, lval);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  uint32_t rnum = ToUint32(e, rval);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  uint32_t shift_count = rnum & 0x1F;
  switch (op.type()) {
    case Token::TK_BIT_LSH:  // <<
      return Number::New(lnum << shift_count);
    case Token::TK_BIT_RSH:  // >>
      return Number::New(lnum >> shift_count);
    case Token::TK_BIT_URSH: {  // >>>
      uint32_t lnum = ToUint32(e, lval);
      return Number::New(lnum >> rnum);
    }
    default:
      assert(false);
  }
}

// 11.8 Relational Operators
Handle<JSValue> EvalRelationalOperator(Handle<Error>& e, Token& op, Handle<JSValue> lval, Handle<JSValue> rval) {
  switch (op.type()) {
    case Token::TK_LT: {  // <
      Handle<JSValue> r = LessThan(e, lval, rval);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      if (r.val()->IsUndefined())
        return Bool::False();
      else
        return r;
    }
    case Token::TK_GT: {  // >
      Handle<JSValue> r = LessThan(e, rval, lval, false);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      if (r.val()->IsUndefined())
        return Bool::False();
      else
        return r;
    }
    case Token::TK_LE: {  // <=
      Handle<JSValue> r = LessThan(e, rval, lval, false);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      if (r.val()->IsUndefined())
        return Bool::False();
      return Bool::Wrap(!static_cast<Handle<Bool>>(r).val()->data());
    }
    case Token::TK_GE: {  // >=
      Handle<JSValue> r = LessThan(e, lval, rval);
      if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
      if (r.val()->IsUndefined())
        return Bool::False();
      return Bool::Wrap(!static_cast<Handle<Bool>>(r).val()->data());
    }
    case Token::TK_KEYWORD_INSTANCE_OF: {
      if (!rval.val()->IsObject()) {
        e = Error::TypeError(u"Right-hand side of 'instanceof' is not an object");
        return Handle<JSValue>();
      }
      if (!rval.val()->IsCallable()) {
        e = Error::TypeError(u"Right-hand side of 'instanceof' is not callable");
        return Handle<JSValue>();
      }
      Handle<JSObject> obj = static_cast<Handle<JSObject>>(rval);
      return Bool::Wrap(HasInstance(e, obj, lval));
    }
    case Token::TK_KEYWORD_IN: {
      if (!rval.val()->IsObject()) {
        e = Error::TypeError(u"in called on non-object");
        return Handle<JSValue>();
      }
      Handle<JSObject> obj = static_cast<Handle<JSObject>>(rval);
      return Bool::Wrap(HasProperty(obj, ToString(e, lval)));
    }
    default:
      assert(false);
  }
}

// 11.9 Equality Operators
Handle<JSValue> EvalEqualityOperator(Handle<Error>& e, Token& op, Handle<JSValue> lval, Handle<JSValue> rval) {
  switch (op.type()) {
    case Token::TK_EQ:   // ==
      return Bool::Wrap(Equal(e, lval, rval));
    case Token::TK_NE:
      return Bool::Wrap(!Equal(e, lval, rval));
    case Token::TK_EQ3:  // ===
      return Bool::Wrap(StrictEqual(e, lval, rval));
    case Token::TK_NE3:
      return Bool::Wrap(!StrictEqual(e, lval, rval));
    default:
      assert(false);
  }
}

// 11.10 Binary Bitwise Operators
Handle<JSValue> EvalBitwiseOperator(Handle<Error>& e, Token& op, Handle<JSValue> lval, Handle<JSValue> rval) {
  int32_t lnum = ToInt32(e, lval);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  int32_t rnum = ToInt32(e, rval);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  switch (op.type()) {
    case Token::TK_BIT_AND:  // &
      return Number::New(lnum & rnum);
    case Token::TK_BIT_XOR:  // ^
      return Number::New(lnum ^ rnum);
    case Token::TK_BIT_OR:  // |
      return Number::New(lnum | rnum);
    default:
      assert(false);
  }
}

// 11.11 Binary Logical Operators
Handle<JSValue> EvalLogicalOperator(Handle<Error>& e, Token& op, AST* lhs, AST* rhs) {
  Handle<JSValue> lval = EvalExpressionAndGetValue(e, lhs);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  bool b = ToBoolean(lval);
  if ((op.type() == Token::TK_LOGICAL_AND && !b) || (op.type() == Token::TK_LOGICAL_OR && b))
    return lval;
  Handle<JSValue> rval = EvalExpressionAndGetValue(e, rhs);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  return rval;
}

// 11.13.1 Simple Assignment ( = )
Handle<JSValue> EvalSimpleAssignment(Handle<Error>& e, Handle<JSValue> lref, Handle<JSValue> rval) {
  if (lref.val()->IsReference()) {
    Handle<Reference> ref = static_cast<Handle<Reference>>(lref);
    bool is_strict_ref = Runtime::TopContext().strict();
    // NOTE in 11.13.1.
    // TODO(zhuzilin) not sure how to implement the type error part of the note.
    if (is_strict_ref) {
      auto stack_ref = Runtime::TopContext().GetReference(ref.val()->id());
      Handle<JSValue> base = stack_ref.base;
      Handle<String> ref_name = stack_ref.name;
      if (Reference::IsUnresolvableReference(base)) {
        e = Error::ReferenceError(ref_name.val()->data() + u" is not defined");
        return Handle<JSValue>();
      }
      if (base.val()->IsEnvironmentRecord() &&
          (StringEqual(ref_name, String::eval()) ||
           StringEqual(ref_name, String::arguments()))) {
        e = Error::SyntaxError(u"cannot assign on eval or arguments");
        return Handle<JSValue>();
      }
    }
  }
  PutValue(e, lref, rval);
  if (unlikely(!e.val()->IsOk()))
    return Handle<JSValue>();
  return rval;
}

// 11.13.2 Compound Assignment ( op= )
Handle<JSValue> EvalCompoundAssignment(Handle<Error>& e, Token& op, Handle<JSValue> lref, Handle<JSValue> rval) {
  Token calc_op = op.ToCalc();
  Handle<JSValue> lval = GetValue(e, lref);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  Handle<JSValue> rref = EvalBinaryExpression(e, calc_op, lval, rval);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  Handle<JSValue> val = GetValue(e, rref);
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  return EvalSimpleAssignment(e, lref, val);
}

// 11.12 Conditional Operator ( ? : )
Handle<JSValue> EvalTripleConditionExpression(Handle<Error>& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_TRIPLE);
  TripleCondition* t = static_cast<TripleCondition*>(ast);
  Handle<JSValue> lval = EvalExpressionAndGetValue(e, t->cond());
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  if (ToBoolean(lval)) {
    Handle<JSValue> true_ref = EvalAssignmentExpression(e, t->true_expr());
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    return GetValue(e, true_ref);
  } else {
    Handle<JSValue> false_ref = EvalAssignmentExpression(e, t->false_expr());
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    return GetValue(e, false_ref);
  }
}

Handle<JSValue> EvalLeftHandSideExpression(Handle<Error>& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_LHS);
  LHS* lhs = static_cast<LHS*>(ast);

  ValueGuard guard;
  Handle<JSValue> base = EvalExpression(e, lhs->base());
  if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();

  size_t new_count = lhs->new_count();
  for (auto pair : lhs->order()) {
    switch (pair.second) {
      case LHS::PostfixType::CALL: {
        Arguments* args = lhs->args_list()[pair.first];
        std::vector<Handle<JSValue>> arg_list = EvalArgumentsList(e, args);
        if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
        if (new_count > 0) {
          base = GetValue(e, base);
          if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
          if (!base.val()->IsConstructor()) {
            e = Error::TypeError(u"base value is not a constructor");
            return Handle<JSValue>();
          }
          Handle<JSObject> constructor = static_cast<Handle<JSObject>>(base);
          base = Construct(e, constructor, arg_list);
          if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
          new_count--;
        } else {
          base = EvalCallExpression(e, base, arg_list);
          if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
        }
        break;
      }
      case LHS::PostfixType::INDEX: {
        auto index = lhs->index_list()[pair.first];
        base = EvalIndexExpression(e, base, index, guard);
        if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
        break;
      }
      case LHS::PostfixType::PROP: {
        auto prop = lhs->prop_name_list()[pair.first];
        base = EvalIndexExpression(e, base, prop, guard);
        if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
        break;
      }
      default:
        assert(false);
    }
  }
  while (new_count > 0) {
    base = GetValue(e, base);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    if (!base.val()->IsConstructor()) {
      e = Error::TypeError(u"base value is not a constructor");
      return Handle<JSValue>();
    }
    Handle<JSObject> constructor = static_cast<Handle<JSObject>>(base);
    base = Construct(e, constructor, {});
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    new_count--;
  }
  return base;
}

std::vector<Handle<JSValue>> EvalArgumentsList(Handle<Error>& e, Arguments* ast) {
  std::vector<Handle<JSValue>> arg_list;
  for (AST* arg_ast : ast->args()) {
    Handle<JSValue> arg = EvalExpressionAndGetValue(e, arg_ast);
    if (unlikely(!e.val()->IsOk()))
      return {};
    arg_list.emplace_back(arg);
  }
  return arg_list;
}

// 11.2.3
Handle<JSValue> EvalCallExpression(Handle<Error>& e, Handle<JSValue> ref, std::vector<Handle<JSValue>> arg_list) {
  Handle<JSValue> val = GetValue(e, ref);
  if (unlikely(!e.val()->IsOk()))
    return Handle<JSValue>();
  if (unlikely(!val.val()->IsObject() && !val.val()->IsCallable())) {  // 4, 5
    if (!val.val()->IsObject()) {
      e = Error::TypeError(u"calling non-object.");
      return Handle<JSValue>();
    }
    e = Error::TypeError(u"calling non-callable.");
    return Handle<JSValue>();
  }
  auto obj = static_cast<Handle<JSObject>>(val);
  Handle<JSValue> this_value;
  if (ref.val()->IsReference()) {
    Handle<Reference> r = static_cast<Handle<Reference>>(ref);
    auto stack_ref = Runtime::TopContext().GetReference(r.val()->id());
    Handle<JSValue> base = stack_ref.base;
    Handle<String> ref_name = stack_ref.name;
    if (Reference::IsPropertyReference(base)) {
      this_value = base;
    } else {
      ASSERT(base.val()->IsEnvironmentRecord());
      auto env_rec = static_cast<Handle<EnvironmentRecord>>(base);
      this_value = ImplicitThisValue(env_rec);
    }
    // indirect
    if (StringEqual(ref_name, String::eval())) {
      DirectEvalGuard guard;
      return Call(e, obj, this_value, arg_list);
    }
  } else {
    this_value = Undefined::Instance();
  }
  return Call(e, obj, this_value, arg_list);
}

// 11.2.1 Property Accessors
Handle<Reference> EvalIndexExpression(Handle<Error>& e, Handle<JSValue> base_ref, Handle<String> identifier_name, ValueGuard& guard) {
  Handle<JSValue> base_value = GetValue(e, base_ref);
  if (unlikely(!e.val()->IsOk()))
    return Handle<JSValue>();
  guard.AddValue(base_value);
  if (unlikely(base_value.val()->IsUndefined() || base_value.val()->IsNull())) {
    if (base_value.val()->IsUndefined()) {
      e = Error::TypeError(u"cannot read property " + identifier_name.val()->data() + u" of undefined");
    } else {
      e = Error::TypeError(u"cannot read property " + identifier_name.val()->data() + u" of null");
    }
    return Handle<Reference>();
  }
  if (unlikely(!e.val()->IsOk()))
    return Handle<JSValue>();
  return Runtime::TopContext().AddReference(base_value, identifier_name);
}

Handle<JSValue> EvalIndexExpression(Handle<Error>& e, Handle<JSValue> base_ref, AST* expr, ValueGuard& guard) {
  Handle<JSValue> property_name_value = EvalExpressionAndGetValue(e, expr);
  if (unlikely(!e.val()->IsOk()))
    return Handle<JSValue>();
  Handle<String> property_name_str = ToString(e, property_name_value);
  if (unlikely(!e.val()->IsOk()))
    return Handle<JSValue>();
  return EvalIndexExpression(e, base_ref, property_name_str, guard);
}

Handle<JSValue> EvalExpressionList(Handle<Error>& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR);
  Expression* exprs = static_cast<Expression*>(ast);
  ASSERT(exprs->elements().size() > 0);
  Handle<JSValue> val;
  for (AST* expr : exprs->elements()) {
    Handle<JSValue> ref = EvalAssignmentExpression(e, expr);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
    val = GetValue(e, ref);
    if (unlikely(!e.val()->IsOk())) return Handle<JSValue>();
  }
  return val;
}

}  // namespace es

#endif  // ES_EVALUATOR_H