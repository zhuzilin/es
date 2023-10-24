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
std::u16string EvalVarDecl(JSValue& e, AST* ast);
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

JSValue EvalExpression(JSValue& e, AST* ast);
JSValue EvalPrimaryExpression(JSValue& e, AST* ast);
JSValue EvalIdentifier(AST* ast);
JSValue EvalNumber(AST* ast);
JSValue EvalString(JSValue& e, AST* ast);
JSValue EvalObject(JSValue& e, AST* ast);
JSValue EvalArray(JSValue& e, AST* ast);
JSValue EvalUnaryOperator(JSValue& e, AST* ast);
JSValue EvalBinaryExpression(JSValue& e, AST* ast);
JSValue EvalBinaryExpression(JSValue& e, Token& op, AST* lval, AST* rval);
JSValue EvalBinaryExpression(JSValue& e, Token& op, JSValue lval, JSValue rval);
JSValue EvalArithmeticOperator(JSValue& e, Token& op, JSValue lval, JSValue rval);
JSValue EvalAddOperator(JSValue& e, JSValue lval, JSValue rval);
JSValue EvalBitwiseShiftOperator(JSValue& e, Token& op, JSValue lval, JSValue rval);
JSValue EvalRelationalOperator(JSValue& e, Token& op, JSValue lval, JSValue rval);
JSValue EvalEqualityOperator(JSValue& e, Token& op, JSValue lval, JSValue rval);
JSValue EvalBitwiseOperator(JSValue& e, Token& op, JSValue lval, JSValue rval);
JSValue EvalLogicalOperator(JSValue& e, Token& op, AST* lhs, AST* rhs);
JSValue EvalSimpleAssignment(JSValue& e, JSValue lref, JSValue rval);
JSValue EvalCompoundAssignment(JSValue& e, Token& op, JSValue lref, JSValue rval);
JSValue EvalTripleConditionExpression(JSValue& e, AST* ast);
JSValue EvalAssignmentExpression(JSValue& e, AST* ast);
JSValue EvalLeftHandSideExpression(JSValue& e, AST* ast);
std::vector<JSValue> EvalArgumentsList(JSValue& e, Arguments* ast);
JSValue EvalCallExpression(JSValue& e, JSValue ref, std::vector<JSValue> arg_list);
JSValue EvalIndexExpression(JSValue& e, JSValue base_ref, JSValue identifier_name, ValueGuard& guard);
JSValue EvalIndexExpression(JSValue& e, JSValue base_ref, AST* expr, ValueGuard& guard);
JSValue EvalExpressionList(JSValue& e, AST* ast);

JSValue IdentifierResolution(std::u16string name);

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
          error::SyntaxError(u"return statement must exist in return statement."),
          u"");
      }
    }
  }

  Completion head_result;
  if (statements.size() == 0)
    return Completion(Completion::NORMAL, JSValue(), u"");
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
  TEST_LOG("EvalStatement ", ast->source().substr(0, 50));
  Completion C(Completion::NORMAL, JSValue(), u"");
  JSValue val;
  {
    // HandleScope scope;
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
      val = C.value();
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

std::u16string EvalVarDecl(JSValue& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_VAR_DECL);
  VarDecl* decl = static_cast<VarDecl*>(ast);
  std::u16string ident = decl->ident().source();
  if (decl->init() == nullptr)
    return ident;
  JSValue lhs = IdentifierResolution(ident);
  JSValue rhs = EvalAssignmentExpression(e, decl->init());
  if (unlikely(!error::IsOk(e))) return ident;
  JSValue value = GetValue(e, rhs);
  if (unlikely(!error::IsOk(e))) return ident;
  PutValue(e, lhs, value);
  if (unlikely(!error::IsOk(e))) return ident;
  return ident;
}

Completion EvalVarStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_VAR);
  JSValue e = error::Ok();
  VarStmt* var_stmt = static_cast<VarStmt*>(ast);
  for (VarDecl* decl : var_stmt->decls()) {
    if (decl->init() == nullptr)
      continue;
    EvalVarDecl(e, decl);
    if (unlikely(!error::IsOk(e))) goto error;
  }
  return Completion(Completion::NORMAL, JSValue(), u"");
error:
  return Completion(Completion::THROW, e, u"");
}

Completion EvalIfStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_IF);
  JSValue e = error::Ok();
  If* if_stmt = static_cast<If*>(ast);
  JSValue expr_ref = EvalExpression(e, if_stmt->cond());
  if (unlikely(!error::IsOk(e)))
    return Completion(Completion::THROW, e, u"");
  JSValue expr = GetValue(e, expr_ref);
  if (unlikely(!error::IsOk(e)))
    return Completion(Completion::THROW, e, u"");
  if (ToBoolean(expr)) {
    return EvalStatement(if_stmt->if_block());
  } else if (if_stmt->else_block() != nullptr){
    return EvalStatement(if_stmt->else_block());
  }
  return Completion(Completion::NORMAL, JSValue(), u"");
}

// 12.6.1 The do-while Statement
Completion EvalDoWhileStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_DO_WHILE);
  JSValue e = error::Ok();
  Runtime::TopContext().EnterIteration();
  DoWhile* loop_stmt = static_cast<DoWhile*>(ast);
  // JSValue V;  // V is substitued by stmt.value()
  JSValue expr_ref;
  JSValue val;
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
        expr_ref = EvalExpression(e, loop_stmt->expr());
        if (unlikely(!error::IsOk(e))) goto error;
        val = GetValue(e, expr_ref);
        if (unlikely(!error::IsOk(e))) goto error;
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
  JSValue e = error::Ok();
  Runtime::TopContext().EnterIteration();
  WhileOrWith* loop_stmt = static_cast<WhileOrWith*>(ast);
  // JSValue V;  // V is substitued by stmt.value()
  JSValue expr_ref;
  JSValue val;
  Completion stmt;
  while (true) {
    expr_ref = EvalExpression(e, loop_stmt->expr());
    if (unlikely(!error::IsOk(e))) goto error;
    val = GetValue(e, expr_ref);
    if (unlikely(!error::IsOk(e))) goto error;
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
  JSValue e = error::Ok();
  Runtime::TopContext().EnterIteration();
  For* for_stmt = static_cast<For*>(ast);
  // JSValue V;  // V is substitued by stmt.value()
  Completion stmt;
  for (auto expr : for_stmt->expr0s()) {
    if (expr->type() == AST::AST_STMT_VAR_DECL) {
      EvalVarDecl(e, expr);
      if (unlikely(!error::IsOk(e))) goto error;
    } else {
      JSValue expr_ref = EvalExpression(e, expr);
      if (unlikely(!error::IsOk(e))) goto error;
      GetValue(e, expr_ref);
      if (unlikely(!error::IsOk(e))) goto error;
    }
  }
  while (true) {
    if (for_stmt->expr1() != nullptr) {
      JSValue test_expr_ref = EvalExpression(e, for_stmt->expr1());
      if (unlikely(!error::IsOk(e))) goto error;
      JSValue test_value = GetValue(e, test_expr_ref);
      if (unlikely(!error::IsOk(e))) goto error;
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
      JSValue inc_expr_ref = EvalExpression(e, for_stmt->expr2());
      if (unlikely(!error::IsOk(e))) goto error;
      GetValue(e, inc_expr_ref);
      if (unlikely(!error::IsOk(e))) goto error;
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
  JSValue e = error::Ok();
  Runtime::TopContext().EnterIteration();
  ForIn* for_in_stmt = static_cast<ForIn*>(ast);
  JSValue obj;
  JSValue expr_ref;
  JSValue expr_val;
  Completion stmt;
  bool has_label;
  JSValue V;
  if (for_in_stmt->expr0()->type() == AST::AST_STMT_VAR_DECL) {
    VarDecl* decl = static_cast<VarDecl*>(for_in_stmt->expr0());
    std::u16string var_name = EvalVarDecl(e, decl);
    if (unlikely(!error::IsOk(e))) goto error;
    expr_ref = EvalExpression(e, for_in_stmt->expr1());
    if (unlikely(!error::IsOk(e))) goto error;
    expr_val = GetValue(e, expr_ref);
    if (unlikely(!error::IsOk(e))) goto error;
    if (expr_val.IsUndefined() || expr_val.IsNull()) {
      Runtime::TopContext().ExitIteration();
      return Completion(Completion::NORMAL, JSValue(), u"");
    }
    obj = ToObject(e, expr_val);
    if (unlikely(!error::IsOk(e))) goto error;

    for (auto pair : js_object::AllEnumerableProperties(obj)) {
      JSValue P = pair.first;
      JSValue var_ref = IdentifierResolution(var_name);
      PutValue(e, var_ref, P);
      if (unlikely(!error::IsOk(e))) goto error;

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
    expr_ref = EvalExpression(e, for_in_stmt->expr1());
    if (unlikely(!error::IsOk(e))) goto error;
    expr_ref = EvalExpression(e, for_in_stmt->expr1());
    if (unlikely(!error::IsOk(e))) goto error;
    expr_val = GetValue(e, expr_ref);
    if (unlikely(!error::IsOk(e))) goto error;
    if (expr_val.IsUndefined() || expr_val.IsNull()) {
      Runtime::TopContext().ExitIteration();
      return Completion(Completion::NORMAL, JSValue(), u"");
    }
    obj = ToObject(e, expr_val);
    for (auto pair : js_object::AllEnumerableProperties(obj)) {
      JSValue P = pair.first;
      JSValue lhs_ref = EvalExpression(e, for_in_stmt->expr0());
      if (unlikely(!error::IsOk(e))) goto error;
      PutValue(e, lhs_ref, P);
      if (unlikely(!error::IsOk(e))) goto error;

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
  JSValue e = error::Ok();
  if (!Runtime::TopContext().InIteration()) {
    e = error::SyntaxError(u"continue not in iteration");
    return Completion(Completion::THROW, e, u"");
  }
  ContinueOrBreak* stmt = static_cast<ContinueOrBreak*>(ast);
  return Completion(Completion::CONTINUE, JSValue(), stmt->ident());
}

Completion EvalBreakStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_BREAK);
  JSValue e = error::Ok();
  if (!Runtime::TopContext().InIteration() && !Runtime::TopContext().InSwitch()) {
    e = error::SyntaxError(u"break not in iteration or switch");
    return Completion(Completion::THROW, e, u"");
  }
  ContinueOrBreak* stmt = static_cast<ContinueOrBreak*>(ast);
  return Completion(Completion::BREAK, JSValue(), stmt->ident());
}

Completion EvalReturnStatement(AST* ast) {
  ASSERT(ast->type() == AST::AST_STMT_RETURN);
  JSValue e = error::Ok();
  Return* return_stmt = static_cast<Return*>(ast);
  if (return_stmt->expr() == nullptr) {
    return Completion(Completion::RETURN, undefined::New(), u"");
  }
  JSValue exp_ref = EvalExpression(e, return_stmt->expr());
  if (unlikely(!error::IsOk(e))) {
    return Completion(Completion::THROW, e, u"");
  }
  return Completion(Completion::RETURN, GetValue(e, exp_ref), u"");
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
      error::SyntaxError(u"cannot have with statement in strict mode"),
      u"");
  }
  JSValue e = error::Ok();
  WhileOrWith* with_stmt = static_cast<WhileOrWith*>(ast);
  JSValue ref = EvalExpression(e, with_stmt->expr());
  if (unlikely(!error::IsOk(e)))
    return Completion(Completion::THROW, e, u"");
  JSValue val = GetValue(e, ref);
  if (unlikely(!error::IsOk(e)))
    return Completion(Completion::THROW, e, u"");
  JSValue obj = ToObject(e, val);
  if (unlikely(!error::IsOk(e)))
    return Completion(Completion::THROW, e, u"");
  // Prevent garbage collect old env.
  JSValue old_env = Runtime::TopLexicalEnv();
  JSValue new_env = NewObjectEnvironment(obj, old_env, true);
  Runtime::TopContext().SetLexicalEnv(new_env);
  Completion C = EvalStatement(with_stmt->stmt());
  Runtime::TopContext().SetLexicalEnv(old_env);
  return C;
}

JSValue EvalCaseClause(JSValue& e, Switch::CaseClause C) {
  JSValue exp_ref = EvalExpression(e, C.expr);
  if (unlikely(!error::IsOk(e)))
    return JSValue();
  return GetValue(e, exp_ref);
}

Completion EvalCaseBlock(Switch* switch_stmt, JSValue input) {
  JSValue e = error::Ok();
  JSValue V;
  bool found = false;
  for (auto C : switch_stmt->before_default_case_clauses()) {
    if (!found) {  // 5.a
      JSValue clause_selector = EvalCaseClause(e, C);
      bool b = StrictEqual(e, input, clause_selector);
      if (unlikely(!error::IsOk(e)))
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
    JSValue clause_selector = EvalCaseClause(e, C);
    bool b = StrictEqual(e, input, clause_selector);
    if (unlikely(!error::IsOk(e)))
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
    EvalCaseClause(e, C);
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
  JSValue e = error::Ok();
  Switch* switch_stmt = static_cast<Switch*>(ast);
  JSValue expr_ref = EvalExpression(e, switch_stmt->expr());
  if (unlikely(!error::IsOk(e)))
    return Completion(Completion::THROW, e, u"");
  JSValue expr_val = GetValue(e, expr_ref);
  if (unlikely(!error::IsOk(e)))
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
  JSValue e = error::Ok();
  Throw* throw_stmt = static_cast<Throw*>(ast);
  JSValue exp_ref = EvalExpression(e, throw_stmt->expr());
  if (unlikely(!error::IsOk(e)))
    return Completion(Completion::THROW, e, u"");
  JSValue val = GetValue(e, exp_ref);
  if (unlikely(!error::IsOk(e)))
    return Completion(Completion::THROW, e, u"");
  return Completion(Completion::THROW, val, u"");
}

Completion EvalCatch(Try* try_stmt, Completion C) {
  JSValue e = error::Ok();
  // Prevent garbage collect old env.
  JSValue old_env = Runtime::TopLexicalEnv();
  JSValue catch_env = NewDeclarativeEnvironment(old_env);
  JSValue ident_str = string::New(try_stmt->catch_ident());
  // NOTE(zhuzilin) The spec say to send C instead of C.value.
  // However, I think it should be send C.value...
  JSValue val;
  if (C.value().IsError()) {
    JSValue error = static_cast<JSValue>(C.value());
    if (error::IsNativeError(error)) {
      val = error::value(error);
    } else {
      val = error_object::New(error);
    }
  } else {
    val = static_cast<JSValue>(C.value());
  }
  CreateAndSetMutableBinding(
    e, lexical_env::env_rec(catch_env), ident_str, false, static_cast<JSValue>(val), false);  // 4 & 5
  if (unlikely(!error::IsOk(e))) {
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
    if (try_stmt->catch_ident() == u"eval" || try_stmt->catch_ident() == u"arguments") {
      JSValue e = error::SyntaxError(u"use eval or arguments as identifier of catch in strict mode");
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
  JSValue e = error::Ok();
  JSValue ref = EvalExpression(e, ast);
  if (unlikely(!error::IsOk(e)))
    return Completion(Completion::THROW, e, u"");
  JSValue val = GetValue(e, ref);
  if (unlikely(!error::IsOk(e)))
    return Completion(Completion::THROW, e, u"");
  return Completion(Completion::NORMAL, val, u"");
}

JSValue EvalExpression(JSValue& e, AST* ast) {
  ASSERT(ast->type() <= AST::AST_EXPR || ast->type() == AST::AST_FUNC);
  JSValue val;
  switch (ast->type()) {
    case AST::AST_EXPR_STRICT_FUTURE:
      if (Runtime::TopContext().strict()) {
        e = error::SyntaxError(u"future reserved word " + ast->source() + u" used");
        return JSValue();
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
  if (unlikely(!error::IsOk(e))) return JSValue();
  return val;
}

JSValue EvalPrimaryExpression(JSValue& e, AST* ast) {
  JSValue val;
  switch (ast->type()) {
    case AST::AST_EXPR_THIS:
      val = Runtime::TopContext().this_binding();
      break;
    case AST::AST_EXPR_STRICT_FUTURE:
    case AST::AST_EXPR_IDENT:
      val = EvalIdentifier(ast);
      break;
    case AST::AST_EXPR_NULL:
      val = null::New();
      break;
    case AST::AST_EXPR_BOOL:
      val = ast->source() == u"true" ? boolean::True() : boolean::False();
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
      val = regex_object::New(string::New(literal->pattern()), string::New(literal->flag()));
      break;
    }
    default:
      std::cout << "Not primary expression, type " << ast->type() << "\n";
      assert(false);
  }
  return val;
}

JSValue IdentifierResolution(std::u16string name) {
  // 10.3.1 Identifier Resolution
  JSValue env = Runtime::TopLexicalEnv();
  JSValue ref_name = string::New(name);
  bool strict = Runtime::TopContext().strict();
  return GetIdentifierReference(env, ref_name, strict);
}

JSValue EvalIdentifier(AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_IDENT || ast->type() == AST::AST_EXPR_STRICT_FUTURE);
  return IdentifierResolution(ast->source());
}

// This verson of string to number assumes the string is valid.
JSValue EvalNumber(const std::u16string& source) {
  double val = 0;
  double frac = 1;
  size_t pos = 0;
  bool dot = false;
  while (pos < source.size()) {
    char16_t c = source[pos];
    switch (c) {
      case u'.':
        dot = true;
        break;
      case u'e':
      case u'E': {
        double exp = 0;
        bool sign = true;
        pos++;  // skip e/E
        c = source[pos];
        if (c == u'-') {
          sign = false;
          pos++;  // skip -
        } else if (c == u'+') {
          sign = true;
          pos++; // skip +;
        }
        while (pos < source.size()) {
          c = source[pos];
          exp *= 10;
          exp += character::Digit(c);
          pos++;
        }
        if (!sign)
          exp = -exp;
        return number::New(val * pow(10.0, exp));
      }
      case u'x':
      case u'X': {
        ASSERT(val == 0);
        pos++;
        while (pos < source.size()) {
          c = source[pos];
          val *= 16;
          val += character::Digit(c);
          pos++;
        }
        return number::New(val);
      }
      default:
        if (dot) {
          frac /= 10;
          val += character::Digit(c) * frac;
        } else {
          val *= 10;
          val += character::Digit(c);
        }
    }
    pos++;
  }
  return number::New(val);
}

JSValue EvalNumber(AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_NUMBER);
  const std::u16string& source = ast->source();
  return EvalNumber(source);
}

JSValue EvalString(JSValue& e, const std::u16string& source) {
  size_t pos = 1;
  std::vector<std::u16string> vals;
  while (pos < source.size() - 1) {
    char16_t c = source[pos];
    switch (c) {
      case u'\\': {
        pos++;
        c = source[pos];
        switch (c) {
          case u'b':
            pos++;
            vals.emplace_back(u"\b");
            break;
          case u't':
            pos++;
            vals.emplace_back(u"\t");
            break;
          case u'n':
            pos++;
            vals.emplace_back(u"\n");
            break;
          case u'v':
            pos++;
            vals.emplace_back(u"\v");
            break;
          case u'f':
            pos++;
            vals.emplace_back(u"\f");
            break;
          case u'r':
            pos++;
            vals.emplace_back(u"\r");
            break;
          case u'0': {
            pos++;
            if (pos < source.size() && character::IsDecimalDigit(source[pos])) {
              e = error::SyntaxError(u"decimal digit after \\0");
              return JSValue();
            }
            vals.emplace_back(std::u16string(1, 0));
            break;
          }
          case u'x': {
            pos++;  // skip 'x'
            char16_t hex = 0;
            for (size_t i = 0; i < 2; i++) {
              hex *= 16;
              hex += character::Digit(source[pos]);
              pos++;
            }
            vals.emplace_back(std::u16string(1, hex));
            break;
          }
          case u'u': {
            pos++;  // skip 'u'
            char16_t hex = 0;
            for (size_t i = 0; i < 4; i++) {
              hex *= 16;
              hex += character::Digit(source[pos]);
              pos++;
            }
            vals.emplace_back(std::u16string(1, hex));
            break;
          }
          default:
            c = source[pos];
            if (character::IsLineTerminator(c)) {
              pos++;
              continue;
            }
            pos++;
            vals.emplace_back(std::u16string(1, c));
        }
        break;
      }
      default: {
        size_t start = pos;
        while (true) {
          if (pos == source.size() - 1 || source[pos] == u'\\')
            break;
          pos++;
        }
        size_t end = pos;
        if (end == source.size() - 1 && vals.size() == 0)
          return string::New(source.substr(start, end - start));
        vals.emplace_back(source.substr(start, end - start));
      }
    }
  }
  if (vals.size() == 0) {
    return string::Empty();
  } else if (vals.size() == 1) {
    return string::New(vals[0]);
  }
  return string::New(StrCat(vals));
}

JSValue EvalString(JSValue& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_STRING);
  const std::u16string& source = ast->source_ref();
  return EvalString(e, source);
}

std::u16string EvalPropertyName(JSValue& e, Token token) {
  switch (token.type()) {
    case Token::TK_STRICT_FUTURE:
    case Token::TK_IDENT:
    case Token::TK_KEYWORD:
    case Token::TK_FUTURE:
    case Token::TK_NULL:
    case Token::TK_BOOL:
      return token.source();
    case Token::TK_NUMBER:
      return ToU16String(e, EvalNumber(token.source_ref()));
    case Token::TK_STRING: {
      JSValue s = EvalString(e, token.source_ref());
      if (unlikely(!error::IsOk(e))) return u"";
      return ToU16String(e, s);
    }
    default:
      assert(false);
  }
}

JSValue EvalObject(JSValue& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_OBJ);
  ObjectLiteral* obj_ast = static_cast<ObjectLiteral*>(ast);
  bool strict = Runtime::TopContext().strict();
  JSValue obj = object_object::New();
  // PropertyName : AssignmentExpression
  for (auto property : obj_ast->properties()) {
    std::u16string prop_name = EvalPropertyName(e, property.key);
    JSValue desc = property_descriptor::New();
    switch (property.type) {
      case ObjectLiteral::Property::NORMAL: {
        JSValue expr_value = EvalAssignmentExpression(e, property.value);
        if (unlikely(!error::IsOk(e))) return JSValue();
        JSValue prop_value = GetValue(e, expr_value);
        if (unlikely(!error::IsOk(e))) return JSValue();
        property_descriptor::SetDataDescriptor(desc, prop_value, true, true, true);
        break;
      }
      default: {
        ASSERT(property.value->type() == AST::AST_FUNC);
        Function* func_ast = static_cast<Function*>(property.value);
        bool strict_func = static_cast<ProgramOrFunctionBody*>(func_ast->body())->strict();
        if (strict || strict_func) {
          for (auto name : func_ast->params()) {
            if (name == u"eval" || name == u"arguments") {
              e = error::SyntaxError(u"object cannot have getter or setter named eval or arguments");
              return JSValue();
            }
          }
        }
        JSValue closure = function_object::New(
          func_ast->params(), func_ast->body(),
          Runtime::TopLexicalEnv(),
          strict || strict_func
        );
        if (property.type == ObjectLiteral::Property::GET) {
          property_descriptor::SetGet(desc, closure);
        } else {
          property_descriptor::SetSet(desc, closure);
        }
        property_descriptor::SetEnumerable(desc, true);
        property_descriptor::SetConfigurable(desc, true);
        break;
      }
    }
    JSValue prop_name_str = string::New(prop_name);
    auto previous = GetOwnProperty(obj, prop_name_str);  // 3
    if (!previous.IsUndefined()) {  // 4
      ASSERT(previous.IsPropertyDescriptor());
      if (strict &&
          property_descriptor::IsDataDescriptor(previous) && property_descriptor::IsDataDescriptor(desc)) {  // 4.a
        e = error::SyntaxError(u"repeat object property name " + prop_name);
        return JSValue();
      }
      if ((property_descriptor::IsDataDescriptor(previous) &&
           property_descriptor::IsAccessorDescriptor(desc)) ||  // 4.b
          (property_descriptor::IsAccessorDescriptor(previous) &&
           property_descriptor::IsDataDescriptor(desc))) {  // 4.c
        e = error::SyntaxError(u"repeat object property name " + prop_name);
        return JSValue();
      }
      if (property_descriptor::IsAccessorDescriptor(previous) &&
          property_descriptor::IsAccessorDescriptor(desc) &&  // 4.d
          ((property_descriptor::HasGet(previous) &&
            property_descriptor::HasGet(desc)) ||
           (property_descriptor::HasSet(previous) &&
            property_descriptor::HasSet(desc)))) {
        e = error::SyntaxError(u"repeat object property name " + prop_name);
        return JSValue();
      }
    }
    DefineOwnProperty(e, obj, prop_name_str, desc, false);
  }
  return obj;
}

JSValue EvalArray(JSValue& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_ARRAY);
  ArrayLiteral* array_ast = static_cast<ArrayLiteral*>(ast);

  JSValue arr = array_object::New(array_ast->length());
  for (auto pair : array_ast->elements()) {
    JSValue init_result = EvalAssignmentExpression(e, pair.second);
    if (unlikely(!error::IsOk(e))) return JSValue();
    JSValue init_value = GetValue(e, init_result);
    if (unlikely(!error::IsOk(e))) return JSValue();
    AddValueProperty(arr, NumberToString(pair.first), init_value, true, true, true);
  }
  return arr;
}

JSValue EvalAssignmentExpression(JSValue& e, AST* ast) {
  return EvalExpression(e, ast);
}

JSValue EvalUnaryOperator(JSValue& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_UNARY);
  Unary* u = static_cast<Unary*>(ast);

  JSValue expr = EvalExpression(e, u->node());
  if (unlikely(!error::IsOk(e))) return JSValue();
  Token op = u->op();

  switch (op.type()) {
    case Token::TK_INC:    // ++
    case Token::TK_DEC: {  // --
      if (expr.IsReference()) {
        if (reference::IsStrictReference(expr) &&
            reference::GetBase(expr).IsEnvironmentRecord() &&
            (string::data(reference::GetReferencedName(expr)) == u"eval" || string::data(reference::GetReferencedName(expr)) == u"arguments")) {
          e = error::SyntaxError(u"cannot inc or dec on eval or arguments");
          return JSValue();
        }
      }
      JSValue old_val = GetValue(e, expr);
      if (unlikely(!error::IsOk(e))) return JSValue();
      double num = ToNumber(e, old_val);
      if (unlikely(!error::IsOk(e))) return JSValue();
      JSValue new_value;
      if (op.type() == Token::TK_INC) {
        new_value = number::New(num + 1);
      } else {
        new_value = number::New(num - 1);
      }
      PutValue(e, expr, new_value);
      if (unlikely(!error::IsOk(e))) return JSValue();
      if (u->prefix()) {
        return new_value;
      } else {
        // a = true; r = a++; r will be 1 instead of true.
        return number::New(num);
      }
    }
    case Token::TK_ADD: {  // +
      JSValue val = GetValue(e, expr);
      if (unlikely(!error::IsOk(e))) return JSValue();
      double num = ToNumber(e, val);
      if (unlikely(!error::IsOk(e))) return JSValue();
      return number::New(num);
    }
    case Token::TK_SUB: {  // -
      JSValue val = GetValue(e, expr);
      if (unlikely(!error::IsOk(e))) return JSValue();
      double num = ToNumber(e, val);
      if (unlikely(!error::IsOk(e))) return JSValue();
      if (isnan(num))
        return number::NaN();
      return number::New(-num);
    }
    case Token::TK_BIT_NOT: {  // ~
      JSValue val = GetValue(e, expr);
      if (unlikely(!error::IsOk(e))) return JSValue();
      int32_t num = ToInt32(e, val);
      if (unlikely(!error::IsOk(e))) return JSValue();
      return number::New(~num);
    }
    case Token::TK_LOGICAL_NOT: {  // !
      JSValue val = GetValue(e, expr);
      if (unlikely(!error::IsOk(e))) return JSValue();
      bool b = ToBoolean(val);
      return boolean::New(!b);
    }
    case Token::TK_KEYWORD: {
      if (op.source_ref() == u"delete") {  // 11.4.1 The delete Operator
        if (!expr.IsReference())  // 2
          return boolean::True();
        if (reference::IsUnresolvableReference(expr)) {  // 3
          if (reference::IsStrictReference(expr)) {
            e = error::SyntaxError(u"delete not exist variable " + string::data(reference::GetReferencedName(expr)));
            return boolean::False();
          }
          return boolean::True();
        }
        if (reference::IsPropertyReference(expr)) {  // 4
          JSValue obj = ToObject(e, reference::GetBase(expr));
          if (unlikely(!error::IsOk(e))) return JSValue();
          return boolean::New(Delete(e, obj, reference::GetReferencedName(expr), reference::IsStrictReference(expr)));
        } else {
          if (reference::IsStrictReference(expr)) {
            e = error::SyntaxError(u"cannot delete environment record in strict mode");
            return boolean::False();
          }
          JSValue bindings =  reference::GetBase(expr);
          return boolean::New(DeleteBinding(e, bindings, reference::GetReferencedName(expr)));
        }
      } else if (op.source_ref() == u"typeof") {
        if (expr.IsReference()) {
          if (reference::IsUnresolvableReference(expr))
            return string::Undefined();
        }
        JSValue val = GetValue(e, expr);
        if (unlikely(!error::IsOk(e))) return JSValue();
        switch (val.type()) {
          case Type::JS_UNDEFINED:
            return string::Undefined();
          case Type::JS_NULL:
            return string::New(u"object");
          case Type::JS_BOOL:
            return string::New(u"boolean");
          case Type::JS_NUMBER:
            return string::New(u"number");
          case Type::JS_STRING:
            return string::New(u"string");
          default:
            if (val.IsCallable())
              return string::New(u"function");
            return string::New(u"object");
        }
      } else if (op.source_ref() == u"void") {
        GetValue(e, expr);
        if (unlikely(!error::IsOk(e))) return JSValue();
        return undefined::New();
      }
    }
    default:
      assert(false);
  }
}

JSValue EvalBinaryExpression(JSValue& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_BINARY);
  Binary* b = static_cast<Binary*>(ast);
  return EvalBinaryExpression(e, b->op(), b->lhs(), b->rhs());
}

JSValue EvalBinaryExpression(JSValue& e, Token& op, AST* lhs, AST* rhs) {
  // && and || are different, as there are not &&= and ||=
  if (op.IsBinaryLogical()) {
    return EvalLogicalOperator(e, op, lhs, rhs);
  }
  if (op.type() == Token::TK_ASSIGN || op.IsCompoundAssign()) {
    JSValue lref = EvalLeftHandSideExpression(e, lhs);
    if (unlikely(!error::IsOk(e))) return JSValue();
    // TODO(zhuzilin) The compound assignment should do lval = GetValue(lref)
    // here. Check if changing the order will have any influence.
    JSValue rref = EvalExpression(e, rhs);
    if (unlikely(!error::IsOk(e))) return JSValue();
    JSValue rval = GetValue(e, rref);
    if (unlikely(!error::IsOk(e))) return JSValue();
    if (op.type() == Token::TK_ASSIGN) {  // =
      return EvalSimpleAssignment(e, lref, rval);
    } else {
      return EvalCompoundAssignment(e, op, lref, rval);
    }
  }

  JSValue lref = EvalExpression(e, lhs);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue lval = GetValue(e, lref);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue rref = EvalExpression(e, rhs);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue rval = GetValue(e, rref);
  if (unlikely(!error::IsOk(e))) return JSValue();
  return EvalBinaryExpression(e, op, lval, rval);
}

JSValue EvalBinaryExpression(JSValue& e, Token& op, JSValue lval, JSValue rval) {
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
    case Token::TK_KEYWORD:
      if (op.source_ref() != u"instanceof" && op.source_ref() != u"in")
        assert(false);
      [[fallthrough]];
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
JSValue EvalArithmeticOperator(JSValue& e, Token& op, JSValue lval, JSValue rval) {
  double lnum = ToNumber(e, lval);
  if (unlikely(!error::IsOk(e))) return JSValue();
  double rnum = ToNumber(e, rval);
  if (unlikely(!error::IsOk(e))) return JSValue();
  switch (op.type()) {
    case Token::TK_MUL:
      return number::New(lnum * rnum);
    case Token::TK_DIV:
      return number::New(lnum / rnum);
    case Token::TK_MOD:
      return number::New(fmod(lnum, rnum));
    case Token::TK_SUB:
      return number::New(lnum - rnum);
    default:
      assert(false);
  }
}

// 11.6 Additive Operators
JSValue EvalAddOperator(JSValue& e, JSValue lval, JSValue rval) {
  JSValue lprim = ToPrimitive(e, lval, u"");
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue rprim = ToPrimitive(e, rval, u"");
  if (unlikely(!error::IsOk(e))) return JSValue();

  if (lprim.IsString() || rprim.IsString()) {
    std::u16string lstr = ToU16String(e, lprim);
    if (unlikely(!error::IsOk(e))) return JSValue();
    std::u16string rstr = ToU16String(e, rprim);
    if (unlikely(!error::IsOk(e))) return JSValue();
    return string::New(lstr + rstr);
  }

  double lnum = ToNumber(e, lprim);
  if (unlikely(!error::IsOk(e))) return JSValue();
  double rnum = ToNumber(e, rprim);
  if (unlikely(!error::IsOk(e))) return JSValue();
  return number::New(lnum + rnum);
}

// 11.7 Bitwise Shift Operators
JSValue EvalBitwiseShiftOperator(JSValue& e, Token& op, JSValue lval, JSValue rval) {
  int32_t lnum = ToInt32(e, lval);
  if (unlikely(!error::IsOk(e))) return JSValue();
  uint32_t rnum = ToUint32(e, rval);
  if (unlikely(!error::IsOk(e))) return JSValue();
  uint32_t shift_count = rnum & 0x1F;
  switch (op.type()) {
    case Token::TK_BIT_LSH:  // <<
      return number::New(lnum << shift_count);
    case Token::TK_BIT_RSH:  // >>
      return number::New(lnum >> shift_count);
    case Token::TK_BIT_URSH: {  // >>>
      uint32_t lnum = ToUint32(e, lval);
      return number::New(lnum >> rnum);
    }
    default:
      assert(false);
  }
}

// 11.8 Relational Operators
JSValue EvalRelationalOperator(JSValue& e, Token& op, JSValue lval, JSValue rval) {
  switch (op.type()) {
    case Token::TK_LT: {  // <
      JSValue r = LessThan(e, lval, rval);
      if (unlikely(!error::IsOk(e))) return JSValue();
      if (r.IsUndefined())
        return boolean::False();
      else
        return r;
    }
    case Token::TK_GT: {  // >
      JSValue r = LessThan(e, rval, lval, false);
      if (unlikely(!error::IsOk(e))) return JSValue();
      if (r.IsUndefined())
        return boolean::False();
      else
        return r;
    }
    case Token::TK_LE: {  // <=
      JSValue r = LessThan(e, rval, lval, false);
      if (unlikely(!error::IsOk(e))) return JSValue();
      if (r.IsUndefined())
        return boolean::False();
      return boolean::New(!boolean::data(r));
    }
    case Token::TK_GE: {  // >=
      JSValue r = LessThan(e, lval, rval);
      if (unlikely(!error::IsOk(e))) return JSValue();
      if (r.IsUndefined())
        return boolean::False();
      return boolean::New(!boolean::data(r));
    }
    case Token::TK_KEYWORD: {
      if (op.source_ref() == u"instanceof") {
        if (!rval.IsObject()) {
          e = error::TypeError(u"Right-hand side of 'instanceof' is not an object");
          return JSValue();
        }
        if (!rval.IsCallable()) {
          e = error::TypeError(u"Right-hand side of 'instanceof' is not callable");
          return JSValue();
        }
        return boolean::New(HasInstance(e, rval, lval));
      } else if (op.source_ref() == u"in") {
        if (!rval.IsObject()) {
          e = error::TypeError(u"in called on non-object");
          return JSValue();
        }
        return boolean::New(HasProperty(rval, ToString(e, lval)));
      }
      [[fallthrough]];
    }
    default:
      assert(false);
  }
}

// 11.9 Equality Operators
JSValue EvalEqualityOperator(JSValue& e, Token& op, JSValue lval, JSValue rval) {
  switch (op.type()) {
    case Token::TK_EQ:   // ==
      return boolean::New(Equal(e, lval, rval));
    case Token::TK_NE:
      return boolean::New(!Equal(e, lval, rval));
    case Token::TK_EQ3:  // ===
      return boolean::New(StrictEqual(e, lval, rval));
    case Token::TK_NE3:
      return boolean::New(!StrictEqual(e, lval, rval));
    default:
      assert(false);
  }
}

// 11.10 Binary Bitwise Operators
JSValue EvalBitwiseOperator(JSValue& e, Token& op, JSValue lval, JSValue rval) {
  int32_t lnum = ToInt32(e, lval);
  if (unlikely(!error::IsOk(e))) return JSValue();
  int32_t rnum = ToInt32(e, rval);
  if (unlikely(!error::IsOk(e))) return JSValue();
  switch (op.type()) {
    case Token::TK_BIT_AND:  // &
      return number::New(lnum & rnum);
    case Token::TK_BIT_XOR:  // ^
      return number::New(lnum ^ rnum);
    case Token::TK_BIT_OR:  // |
      return number::New(lnum | rnum);
    default:
      assert(false);
  }
}

// 11.11 Binary Logical Operators
JSValue EvalLogicalOperator(JSValue& e, Token& op, AST* lhs, AST* rhs) {
  JSValue lref = EvalExpression(e, lhs);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue lval = GetValue(e, lref);
  if (unlikely(!error::IsOk(e))) return JSValue();
  bool b = ToBoolean(lval);
  if ((op.type() == Token::TK_LOGICAL_AND && !b) || (op.type() == Token::TK_LOGICAL_OR && b))
    return lval;
  JSValue rref = EvalExpression(e, rhs);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue rval = GetValue(e, rref);
  if (unlikely(!error::IsOk(e))) return JSValue();
  return rval;
}

// 11.13.1 Simple Assignment ( = )
JSValue EvalSimpleAssignment(JSValue& e, JSValue lref, JSValue rval) {
  if (lref.IsReference()) {
    // NOTE in 11.13.1.
    // TODO(zhuzilin) not sure how to implement the type error part of the note.
    if (reference::IsStrictReference(lref)) {
      if (reference::IsUnresolvableReference(lref)) {
        e = error::ReferenceError(string::data(reference::GetReferencedName(lref)) + u" is not defined");
        return JSValue();
      }
      if (reference::GetBase(lref).IsEnvironmentRecord() &&
          (string::data(reference::GetReferencedName(lref)) == u"eval" ||
          string::data(reference::GetReferencedName(lref)) == u"arguments")) {
        e = error::SyntaxError(u"cannot assign on eval or arguments");
        return JSValue();
      }
    }
  }
  PutValue(e, lref, rval);
  if (unlikely(!error::IsOk(e)))
    return JSValue();
  return rval;
}

// 11.13.2 Compound Assignment ( op= )
JSValue EvalCompoundAssignment(JSValue& e, Token& op, JSValue lref, JSValue rval) {
  Token calc_op = op.ToCalc();
  JSValue lval = GetValue(e, lref);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue rref = EvalBinaryExpression(e, calc_op, lval, rval);
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue val = GetValue(e, rref);
  if (unlikely(!error::IsOk(e))) return JSValue();
  return EvalSimpleAssignment(e, lref, val);
}

// 11.12 Conditional Operator ( ? : )
JSValue EvalTripleConditionExpression(JSValue& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_TRIPLE);
  TripleCondition* t = static_cast<TripleCondition*>(ast);
  JSValue lref = EvalExpression(e, t->cond());
  if (unlikely(!error::IsOk(e))) return JSValue();
  JSValue lval = GetValue(e, lref);
  if (unlikely(!error::IsOk(e))) return JSValue();
  if (ToBoolean(lval)) {
    JSValue true_ref = EvalAssignmentExpression(e, t->true_expr());
    if (unlikely(!error::IsOk(e))) return JSValue();
    return GetValue(e, true_ref);
  } else {
    JSValue false_ref = EvalAssignmentExpression(e, t->false_expr());
    if (unlikely(!error::IsOk(e))) return JSValue();
    return GetValue(e, false_ref);
  }
}

JSValue EvalLeftHandSideExpression(JSValue& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR_LHS);
  LHS* lhs = static_cast<LHS*>(ast);

  ValueGuard guard;
  JSValue base = EvalExpression(e, lhs->base());
  if (unlikely(!error::IsOk(e))) return JSValue();

  size_t new_count = lhs->new_count();
  for (auto pair : lhs->order()) {
    switch (pair.second) {
      case LHS::PostfixType::CALL: {
        Arguments* args = lhs->args_list()[pair.first];
        std::vector<JSValue> arg_list = EvalArgumentsList(e, args);
        if (unlikely(!error::IsOk(e))) return JSValue();
        if (new_count > 0) {
          base = GetValue(e, base);
          if (unlikely(!error::IsOk(e))) return JSValue();
          if (!base.IsConstructor()) {
            e = error::TypeError(u"base value is not a constructor");
            return JSValue();
          }
          base = Construct(e, base, arg_list);
          if (unlikely(!error::IsOk(e))) return JSValue();
          new_count--;
        } else {
          base = EvalCallExpression(e, base, arg_list);
          if (unlikely(!error::IsOk(e))) return JSValue();
        }
        break;
      }
      case LHS::PostfixType::INDEX: {
        auto index = lhs->index_list()[pair.first];
        base = EvalIndexExpression(e, base, index, guard);
        if (unlikely(!error::IsOk(e))) return JSValue();
        break;
      }
      case LHS::PostfixType::PROP: {
        auto prop = lhs->prop_name_list()[pair.first];
        base = EvalIndexExpression(e, base, string::New(prop), guard);
        if (unlikely(!error::IsOk(e))) return JSValue();
        break;
      }
      default:
        assert(false);
    }
  }
  while (new_count > 0) {
    base = GetValue(e, base);
    if (unlikely(!error::IsOk(e))) return JSValue();
    if (!base.IsConstructor()) {
      e = error::TypeError(u"base value is not a constructor");
      return JSValue();
    }
    base = Construct(e, base, {});
    if (unlikely(!error::IsOk(e))) return JSValue();
    new_count--;
  }
  return base;
}

std::vector<JSValue> EvalArgumentsList(JSValue& e, Arguments* ast) {
  std::vector<JSValue> arg_list;
  for (AST* arg_ast : ast->args()) {
    JSValue ref = EvalExpression(e, arg_ast);
    if (unlikely(!error::IsOk(e)))
      return {};
    JSValue arg = GetValue(e, ref);
    if (unlikely(!error::IsOk(e)))
      return {};
    arg_list.emplace_back(arg);
  }
  return arg_list;
}

// 11.2.3
JSValue EvalCallExpression(JSValue& e, JSValue ref, std::vector<JSValue> arg_list) {
  JSValue val = GetValue(e, ref);
  if (unlikely(!error::IsOk(e)))
    return JSValue();
  if (!val.IsObject()) {  // 4
    e = error::TypeError(u"calling non-object.");
    return JSValue();
  }
  if (!val.IsCallable()) {  // 5
    e = error::TypeError(u"calling non-callable.");
    return JSValue();
  }
  JSValue this_value;
  if (ref.IsReference()) {
    JSValue base = reference::GetBase(ref);
    if (reference::IsPropertyReference(ref)) {
      this_value = base;
    } else {
      ASSERT(base.IsEnvironmentRecord());
      this_value = ImplicitThisValue(base);
    }
  } else {
    this_value = undefined::New();
  }
  // indirect 
  if (ref.IsReference() && string::data_view(reference::GetReferencedName(ref)) == u"eval") {
    DirectEvalGuard guard;
    return Call(e, val, this_value, arg_list);
  } else {
    return Call(e, val, this_value, arg_list);
  }
}

// 11.2.1 Property Accessors
JSValue EvalIndexExpression(JSValue& e, JSValue base_ref, JSValue identifier_name, ValueGuard& guard) {
  JSValue base_value = GetValue(e, base_ref);
  if (unlikely(!error::IsOk(e)))
    return JSValue();
  guard.AddValue(base_value);
  CheckObjectCoercible(e, base_value);
  if (unlikely(!error::IsOk(e)))
    return JSValue();
  bool strict = Runtime::TopContext().strict();
  return reference::New(base_value, identifier_name, strict);
}

JSValue EvalIndexExpression(JSValue& e, JSValue base_ref, AST* expr, ValueGuard& guard) {
  JSValue property_name_ref = EvalExpression(e, expr);
  if (unlikely(!error::IsOk(e)))
    return JSValue();
  JSValue property_name_value = GetValue(e, property_name_ref);
  if (unlikely(!error::IsOk(e)))
    return JSValue();
  JSValue property_name_str = ToString(e, property_name_value);
  if (unlikely(!error::IsOk(e)))
    return JSValue();
  return EvalIndexExpression(e, base_ref, property_name_str, guard);
}

JSValue EvalExpressionList(JSValue& e, AST* ast) {
  ASSERT(ast->type() == AST::AST_EXPR);
  Expression* exprs = static_cast<Expression*>(ast);
  ASSERT(exprs->elements().size() > 0);
  JSValue val;
  for (AST* expr : exprs->elements()) {
    JSValue ref = EvalAssignmentExpression(e, expr);
    if (unlikely(!error::IsOk(e))) return JSValue();
    val = GetValue(e, ref);
    if (unlikely(!error::IsOk(e))) return JSValue();
  }
  return val;
}

}  // namespace es

#endif  // ES_EVALUATOR_H